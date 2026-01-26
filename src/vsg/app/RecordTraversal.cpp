/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/animation/Animation.h>
#include <vsg/app/CommandGraph.h>
#include <vsg/app/RecordTraversal.h>
#include <vsg/app/View.h>
#include <vsg/commands/Command.h>
#include <vsg/commands/Commands.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/io/Logger.h>
#include <vsg/io/stream.h>
#include <vsg/lighting/AmbientLight.h>
#include <vsg/lighting/DirectionalLight.h>
#include <vsg/lighting/Light.h>
#include <vsg/lighting/PointLight.h>
#include <vsg/lighting/SpotLight.h>
#include <vsg/maths/plane.h>
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/CoordinateFrame.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/DepthSorted.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/InstanceDraw.h>
#include <vsg/nodes/InstanceDrawIndexed.h>
#include <vsg/nodes/InstanceNode.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/Layer.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/RegionOfInterest.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/Switch.h>
#include <vsg/nodes/TileDatabase.h>
#include <vsg/nodes/VertexDraw.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/threading/atomics.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <vsg/utils/Instrumentation.h>

using namespace vsg;

#define INLINE_TRAVERSE 0

RecordTraversal::RecordTraversal(const Slots& in_maxSlots, const std::set<Bin*>& in_bins) :
    state(new State(in_maxSlots))
{
    CPU_INSTRUMENTATION_L1_C(instrumentation, COLOR_RECORD);

    minimumBinNumber = 0;
    int32_t maximumBinNumber = 0;
    for (const auto& bin : in_bins)
    {
        if (bin->binNumber < minimumBinNumber) minimumBinNumber = bin->binNumber;
        if (bin->binNumber > maximumBinNumber) maximumBinNumber = bin->binNumber;
    }

    bins.resize((maximumBinNumber - minimumBinNumber) + 1);

    for (auto& bin : in_bins)
    {
        bins[bin->binNumber - minimumBinNumber] = bin;
    }
}

RecordTraversal::~RecordTraversal()
{
    CPU_INSTRUMENTATION_L2(instrumentation);
}

CommandBuffer* RecordTraversal::getCommandBuffer()
{
    return state->_commandBuffer;
}

uint32_t RecordTraversal::deviceID() const
{
    return state->_commandBuffer->deviceID;
}

void RecordTraversal::setFrameStamp(FrameStamp* fs)
{
    frameStamp = fs;
}

void RecordTraversal::setDatabasePager(DatabasePager* dp)
{
    if (dp == databasePager) return;

    databasePager = dp;
    culledPagedLODs = dp ? dp->culledPagedLODs.get() : nullptr;
}

void RecordTraversal::clearBins()
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "RecordTraversal clearBins", COLOR_RECORD_L2);

    for (auto& bin : bins)
    {
        if (bin) bin->clear();
    }
}

void RecordTraversal::apply(const Object& object)
{
    // GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "Object", COLOR_RECORD_L2, &object);

    //debug("Visiting Object");
    object.traverse(*this);
}

void RecordTraversal::apply(const Group& group)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "Group", COLOR_RECORD_L2, &group);

    //debug("Visiting Group");
#if INLINE_TRAVERSE
    vsg::Group::t_traverse(group, *this);
#else
    group.traverse(*this);
#endif
}

void RecordTraversal::apply(const QuadGroup& quadGroup)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "QuadGroup", COLOR_RECORD_L2, &quadGroup);

    //debug("Visiting QuadGroup");
#if INLINE_TRAVERSE
    vsg::QuadGroup::t_traverse(quadGroup, *this);
#else
    quadGroup.traverse(*this);
#endif
}

void RecordTraversal::apply(const LOD& lod)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "LOD", COLOR_RECORD_L2, &lod);

    const auto& sphere = lod.bound;

    // check if lod bounding sphere is in view frustum.
    auto lodDistance = state->lodDistance(sphere);
    if (lodDistance < 0.0)
    {
        return;
    }

    if (viewDependentState) lodDistance *= viewDependentState->LODScale;

    for (auto& child : lod.children)
    {
        auto cutoff = lodDistance * child.minimumScreenHeightRatio;
        bool child_visible = sphere.r > cutoff;
        if (child_visible)
        {
            child.node->accept(*this);
            return;
        }
    }
}

void RecordTraversal::apply(const PagedLOD& plod)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "PagedLOD", COLOR_PAGER, &plod);

    const auto& sphere = plod.bound;
    auto frameCount = frameStamp->frameCount;

    // check if lod bounding sphere is in view frustum.
    auto lodDistance = state->lodDistance(sphere);
    if (lodDistance < 0.0)
    {
        if ((frameCount - plod.frameHighResLastUsed) > 1 && culledPagedLODs)
        {
            culledPagedLODs->highresCulled.emplace_back(&plod);
        }

        return;
    }

    if (viewDependentState) lodDistance *= viewDependentState->LODScale;

    // check the high res child to see if it's visible
    {
        const auto& child = plod.children[0];

        auto cutoff = lodDistance * child.minimumScreenHeightRatio;
        bool child_visible = sphere.r > cutoff;
        if (child_visible)
        {
            auto previousHighResUsed = plod.frameHighResLastUsed.exchange(frameCount);
            if (culledPagedLODs && ((frameCount - previousHighResUsed) > 1))
            {
                culledPagedLODs->newHighresRequired.emplace_back(&plod);
            }

            if (child.node)
            {
                // high res visible and available so traverse it
                child.node->accept(*this);
                return;
            }
            else if (databasePager)
            {
                auto priority = sphere.r / cutoff;
                exchange_if_greater(plod.priority, priority);

                auto previousRequestCount = plod.requestCount.fetch_add(1);
                if (previousRequestCount == 0)
                {
                    // we are the first request so tell the databasePager about it
                    databasePager->request(ref_ptr<PagedLOD>(const_cast<PagedLOD*>(&plod)));
                }
                else
                {
                    //debug("repeat request ",&plod,", ",plod.filename,", ",plod.requestCount.load(),", plod.requestStatus = ",plod.requestStatus.load());
                }
            }
        }
        else
        {
            if (culledPagedLODs && ((frameCount - plod.frameHighResLastUsed) <= 1))
            {
                culledPagedLODs->highresCulled.emplace_back(&plod);
            }
        }
    }

    // check the low res child to see if it's visible
    {
        const auto& child = plod.children[1];
        auto cutoff = lodDistance * child.minimumScreenHeightRatio;
        bool child_visible = sphere.r > cutoff;
        if (child_visible)
        {
            if (child.node)
            {
                child.node->accept(*this);
            }
        }
    }
}

void RecordTraversal::apply(const TileDatabase& tileDatabase)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "TileDatabase", COLOR_RECORD_L2, &tileDatabase);

    tileDatabase.traverse(*this);
}

void RecordTraversal::apply(const CullGroup& cullGroup)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "CullGroup", COLOR_RECORD_L2, &cullGroup);

    if (state->intersect(cullGroup.bound))
    {
        // debug("Passed node");
        cullGroup.traverse(*this);
    }
}

void RecordTraversal::apply(const CullNode& cullNode)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "CullNode", COLOR_RECORD_L2, &cullNode);

    if (state->intersect(cullNode.bound))
    {
        //debug("Passed node");
        cullNode.traverse(*this);
    }
}

void RecordTraversal::apply(const Switch& sw)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "Switch", COLOR_RECORD_L2, &sw);

    for (auto& child : sw.children)
    {
        if ((traversalMask & (overrideMask | child.mask)) != MASK_OFF)
        {
            child.node->accept(*this);
        }
    }
}

void RecordTraversal::apply(const RegionOfInterest& roi)
{
    CPU_INSTRUMENTATION_L2_NCO(instrumentation, "RegionOfInterest", COLOR_RECORD_L2, &roi);

    regionsOfInterest.emplace_back(state->modelviewMatrixStack.top(), &roi);
}

void RecordTraversal::apply(const DepthSorted& depthSorted)
{
    CPU_INSTRUMENTATION_L2_NCO(instrumentation, "DepthSorted", COLOR_RECORD_L2, &depthSorted);

    if (state->intersect(depthSorted.bound))
    {
        const auto& mv = state->modelviewMatrixStack.top();
        const auto& center = depthSorted.bound.center;
        auto distance = -(mv[0][2] * center.x + mv[1][2] * center.y + mv[2][2] * center.z + mv[3][2]);

        addToBin(depthSorted.binNumber, distance, depthSorted.child);
    }
}

void RecordTraversal::apply(const Layer& layer)
{
    CPU_INSTRUMENTATION_L2_NCO(instrumentation, "Layer", COLOR_RECORD_L2, &layer);
    if ((traversalMask & (overrideMask | layer.mask)) != MASK_OFF)
    {
        addToBin(layer.binNumber, layer.value, layer.child);
    }
}

void RecordTraversal::apply(const VertexDraw& vd)
{
    GPU_INSTRUMENTATION_L3_NCO(instrumentation, *getCommandBuffer(), "VertexDraw", COLOR_GPU, &vd);

    //debug("Visiting VertexDraw");
    state->record();
    vd.record(*(state->_commandBuffer));
}

void RecordTraversal::apply(const VertexIndexDraw& vid)
{
    GPU_INSTRUMENTATION_L3_NCO(instrumentation, *getCommandBuffer(), "VertexIndexDraw", COLOR_GPU, &vid);

    //debug("Visiting VertexIndexDraw");
    state->record();
    vid.record(*(state->_commandBuffer));
}

void RecordTraversal::apply(const Geometry& geometry)
{
    GPU_INSTRUMENTATION_L3_NCO(instrumentation, *getCommandBuffer(), "Geometry", COLOR_GPU, &geometry);

    //debug("Visiting Geometry");
    state->record();
    geometry.record(*(state->_commandBuffer));
}

void RecordTraversal::apply(const Light&)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    //debug("RecordTraversal::apply(Light) ", light.className());
}

void RecordTraversal::apply(const AmbientLight& light)
{
    CPU_INSTRUMENTATION_L2_O(instrumentation, &light);

    //debug("RecordTraversal::apply(AmbientLight) ", light.className());
    if (light.intensity >= intensityMinimum && viewDependentState) viewDependentState->ambientLights.emplace_back(state->modelviewMatrixStack.top(), &light);
}

void RecordTraversal::apply(const DirectionalLight& light)
{
    CPU_INSTRUMENTATION_L2_O(instrumentation, &light);

    //debug("RecordTraversal::apply(DirectionalLight) ", light.className());
    if (light.intensity >= intensityMinimum && viewDependentState) viewDependentState->directionalLights.emplace_back(state->modelviewMatrixStack.top(), &light);
}

void RecordTraversal::apply(const PointLight& light)
{
    CPU_INSTRUMENTATION_L2_O(instrumentation, &light);

    //debug("RecordTraversal::apply(PointLight) ", light.className());
    if (light.intensity >= intensityMinimum && viewDependentState) viewDependentState->pointLights.emplace_back(state->modelviewMatrixStack.top(), &light);
}

void RecordTraversal::apply(const SpotLight& light)
{
    CPU_INSTRUMENTATION_L2_O(instrumentation, &light);

    //debug("RecordTraversal::apply(SpotLight) ", light.className());
    if (light.intensity >= intensityMinimum && viewDependentState) viewDependentState->spotLights.emplace_back(state->modelviewMatrixStack.top(), &light);
}

// transform nodes
void RecordTraversal::apply(const Transform& transform)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "Transform", COLOR_RECORD_L2, &transform);

    state->modelviewMatrixStack.push(transform);
    state->dirty = true;

    if (transform.subgraphRequiresLocalFrustum)
    {
        state->pushFrustum();
        transform.traverse(*this);
        state->popFrustum();
    }
    else
    {
        transform.traverse(*this);
    }

    state->modelviewMatrixStack.pop();
    state->dirty = true;
}

void RecordTraversal::apply(const MatrixTransform& mt)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "MatrixTransform", COLOR_RECORD_L2, &mt);

    state->modelviewMatrixStack.push(mt);
    state->dirty = true;

    if (mt.subgraphRequiresLocalFrustum)
    {
        state->pushFrustum();
        mt.traverse(*this);
        state->popFrustum();
    }
    else
    {
        mt.traverse(*this);
    }

    state->modelviewMatrixStack.pop();
    state->dirty = true;
}

void RecordTraversal::apply(const CoordinateFrame& cf)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "CoordinateFrame", COLOR_RECORD_L2, &cf);

    const View* parentView = viewDependentState ? viewDependentState->view : nullptr;
    const Camera* camera = parentView ? parentView->camera : nullptr;
    const ViewMatrix* viewMatrix = camera ? camera->viewMatrix : nullptr;

    if (viewMatrix)
    {
        state->modelviewMatrixStack.push(viewMatrix->transform(cf.origin) * vsg::rotate(cf.rotation));
    }
    else
    {
        state->modelviewMatrixStack.push(cf);
    }

    state->dirty = true;

    if (cf.subgraphRequiresLocalFrustum)
    {
        state->pushFrustum();
        cf.traverse(*this);
        state->popFrustum();
    }
    else
    {
        cf.traverse(*this);
    }

    state->modelviewMatrixStack.pop();
    state->dirty = true;
}

// Animation nodes
void RecordTraversal::apply(const Joint&)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    // non op for RiggedJoint as it's designed not to have any renderable children
}

void RecordTraversal::apply(const InstanceNode& instanceNode)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    if (instanceNode.child)
    {
        state->_commandBuffer->instanceNode = &instanceNode;
        instanceNode.child->accept(*this);
    }
}

void RecordTraversal::apply(const InstanceDraw& instanceDraw)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    state->record();
    instanceDraw.record(*(state->_commandBuffer));
}

void RecordTraversal::apply(const InstanceDrawIndexed& instanceDrawIndexed)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    state->record();
    instanceDrawIndexed.record(*(state->_commandBuffer));
}

// Vulkan nodes
void RecordTraversal::apply(const StateGroup& stateGroup)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "StateGroup", COLOR_RECORD_L2, &stateGroup);

    //debug("Visiting StateGroup");

    auto begin = stateGroup.stateCommands.begin();
    auto end = stateGroup.stateCommands.end();

    state->push(begin, end);

    stateGroup.traverse(*this);

    state->pop(begin, end);
}

void RecordTraversal::apply(const Commands& commands)
{
    GPU_INSTRUMENTATION_L3_NCO(instrumentation, *getCommandBuffer(), "Commands", COLOR_GPU, &commands);

    state->record();
    for (auto& command : commands.children)
    {
        command->record(*(state->_commandBuffer));
    }
}

void RecordTraversal::apply(const Command& command)
{
    GPU_INSTRUMENTATION_L3_NCO(instrumentation, *getCommandBuffer(), "Command", COLOR_GPU, &command);

    //debug("Visiting Command");
    state->record();
    command.record(*(state->_commandBuffer));
}

void RecordTraversal::apply(const Bin& bin)
{
    GPU_INSTRUMENTATION_L1_NCO(instrumentation, *getCommandBuffer(), "Bin", COLOR_RECORD_L1, &bin);

    //debug("Visiting Bin");
    bin.traverse(*this);
}

void RecordTraversal::apply(const View& view)
{
    GPU_INSTRUMENTATION_L1_NCO(instrumentation, *getCommandBuffer(), "View", COLOR_RECORD_L1, &view);

    // dirty the state stacks to ensure state is newly applied for the View.
    state->dirtyStateStacks();

    // note, View::accept() updates the RecordTraversal's traversalMask
    auto cached_traversalMask = state->_commandBuffer->traversalMask;
    state->_commandBuffer->traversalMask = traversalMask;
    state->_commandBuffer->viewID = view.viewID;
    state->_commandBuffer->viewDependentState = view.viewDependentState.get();

    // cache the previous bins
    int32_t cached_minimumBinNumber = minimumBinNumber;

    decltype(bins) cached_bins;
    cached_bins.swap(bins);

    auto cached_viewDependentState = viewDependentState;

    decltype(regionsOfInterest) cached_regionsOfInterest;
    cached_regionsOfInterest.swap(regionsOfInterest);

    // assign and clear the View's bins
    int32_t min_binNumber = 0;
    int32_t max_binNumber = 0;
    for (const auto& bin : view.bins)
    {
        if (bin->binNumber < min_binNumber) min_binNumber = bin->binNumber;
        if (bin->binNumber > max_binNumber) max_binNumber = bin->binNumber;
    }

    minimumBinNumber = min_binNumber;
    bins.resize(max_binNumber - min_binNumber + 1);
    for (auto& bin : view.bins)
    {
        bins[bin->binNumber - min_binNumber] = bin;
        bin->clear();
    }

    // assign and clear the View's ViewDependentState
    viewDependentState = view.viewDependentState;
    if (viewDependentState)
    {
        viewDependentState->clear();
        viewDependentState->LODScale = view.LODScale;
    }

    state->pushView(view);

    if (view.camera)
    {
        state->inheritViewForLODScaling = (view.features & INHERIT_VIEWPOINT) != 0;
        state->setProjectionAndViewMatrix(view.camera->projectionMatrix->transform(), view.camera->viewMatrix->transform());

        if (const auto& viewportState = view.camera->viewportState)
        {
            if (viewDependentState)
            {
                auto& viewportData = viewDependentState->viewportData;
                if (viewportData)
                {
                    auto& viewports = viewportState->viewports;
                    auto dest_itr = viewportData->begin();
                    for (auto src_itr = viewports.begin();
                         dest_itr != viewportData->end() && src_itr != viewports.end();
                         ++dest_itr, ++src_itr)
                    {
                        auto& dest_viewport = *dest_itr;
                        vec4 src_viewport(src_itr->x, src_itr->y, src_itr->width, src_itr->height);
                        if (dest_viewport != src_viewport)
                        {
                            dest_viewport = src_viewport;
                            viewportData->dirty();
                        }
                    }
                }
            }

            view.traverse(*this);
        }
        else
        {
            view.traverse(*this);
        }
    }
    else
    {
        view.traverse(*this);
    }

    state->popView(view);

    for (auto& bin : view.bins)
    {
        bin->accept(*this);
    }

    if (viewDependentState)
    {
        viewDependentState->traverse(*this);
    }

    // swap back previous bin setup.
    minimumBinNumber = cached_minimumBinNumber;
    cached_bins.swap(bins);
    cached_regionsOfInterest.swap(regionsOfInterest);
    state->_commandBuffer->traversalMask = cached_traversalMask;
    viewDependentState = cached_viewDependentState;
}

void RecordTraversal::apply(const CommandGraph& commandGraph)
{
    GPU_INSTRUMENTATION_L1_NCO(instrumentation, *getCommandBuffer(), "RecordTraversal CommandGraph", COLOR_RECORD_L1, &commandGraph);

    if (recordedCommandBuffers)
    {
        auto cg = const_cast<CommandGraph*>(&commandGraph);
        if (cg->device)
        {
            cg->getOrCreateRecordTraversal()->state->inherit(*state);

            if (viewDependentState && viewDependentState->view && (viewDependentState->view->features & INHERIT_VIEWPOINT) != 0)
            {
                auto camera = viewDependentState->view->camera;
                if (camera) cg->getOrCreateRecordTraversal()->state->setInhertiedViewProjectionAndViewMatrix(camera->projectionMatrix->transform(), camera->viewMatrix->transform());
            }

            cg->record(recordedCommandBuffers, frameStamp, databasePager);
        }
        else
        {
            warn("RecordTraversal::apply(const CommandGraph& commandGraph) cannot traverse as commandGraph->device = ", cg->device);
        }
    }
    else
    {
        commandGraph.traverse(*this);
    }
}

void RecordTraversal::addToBin(int32_t binNumber, double value, const Node* node)
{
    bins[binNumber - minimumBinNumber]->add(state, value, node);
}
