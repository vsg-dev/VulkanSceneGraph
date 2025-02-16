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

RecordTraversal::RecordTraversal(uint32_t in_maxSlot, const std::set<Bin*>& in_bins) :
    _state(new State(in_maxSlot))
{
    CPU_INSTRUMENTATION_L1_C(instrumentation, COLOR_RECORD);

    _minimumBinNumber = 0;
    int32_t maximumBinNumber = 0;
    for (const auto& bin : in_bins)
    {
        if (bin->binNumber < _minimumBinNumber) _minimumBinNumber = bin->binNumber;
        if (bin->binNumber > maximumBinNumber) maximumBinNumber = bin->binNumber;
    }

    _bins.resize((maximumBinNumber - _minimumBinNumber) + 1);

    for (auto& bin : in_bins)
    {
        _bins[bin->binNumber - _minimumBinNumber] = bin;
    }
}

RecordTraversal::~RecordTraversal()
{
    CPU_INSTRUMENTATION_L2(instrumentation);
}

CommandBuffer* RecordTraversal::getCommandBuffer()
{
    return _state->_commandBuffer;
}

uint32_t RecordTraversal::deviceID() const
{
    return _state->_commandBuffer->deviceID;
}

void RecordTraversal::setFrameStamp(FrameStamp* fs)
{
    _frameStamp = fs;
}

void RecordTraversal::setDatabasePager(DatabasePager* dp)
{
    if (dp == _databasePager) return;

    _databasePager = dp;
    _culledPagedLODs = dp ? dp->culledPagedLODs.get() : nullptr;
}

void RecordTraversal::clearBins()
{
    CPU_INSTRUMENTATION_L2_NC(instrumentation, "RecordTraversal clearBins", COLOR_RECORD_L2);

    for (auto& bin : _bins)
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
    auto lodDistance = _state->lodDistance(sphere);
    if (lodDistance < 0.0)
    {
        return;
    }

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
    auto frameCount = _frameStamp->frameCount;

    // check if lod bounding sphere is in view frustum.
    auto lodDistance = _state->lodDistance(sphere);
    if (lodDistance < 0.0)
    {
        if ((frameCount - plod.frameHighResLastUsed) > 1 && _culledPagedLODs)
        {
            _culledPagedLODs->highresCulled.emplace_back(&plod);
        }

        return;
    }

    // check the high res child to see if it's visible
    {
        const auto& child = plod.children[0];

        auto cutoff = lodDistance * child.minimumScreenHeightRatio;
        bool child_visible = sphere.r > cutoff;
        if (child_visible)
        {
            auto previousHighResUsed = plod.frameHighResLastUsed.exchange(frameCount);
            if (_culledPagedLODs && ((frameCount - previousHighResUsed) > 1))
            {
                _culledPagedLODs->newHighresRequired.emplace_back(&plod);
            }

            if (child.node)
            {
                // high res visible and available so traverse it
                child.node->accept(*this);
                return;
            }
            else if (_databasePager)
            {
                auto priority = sphere.r / cutoff;
                exchange_if_greater(plod.priority, priority);

                auto previousRequestCount = plod.requestCount.fetch_add(1);
                if (previousRequestCount == 0)
                {
                    // we are the first request so tell the databasePager about it
                    _databasePager->request(ref_ptr<PagedLOD>(const_cast<PagedLOD*>(&plod)));
                }
                else
                {
                    //debug("repeat request ",&plod,", ",plod.filename,", ",plod.requestCount.load(),", plod.requestStatus = ",plod.requestStatus.load());
                }
            }
        }
        else
        {
            if (_culledPagedLODs && ((frameCount - plod.frameHighResLastUsed) <= 1))
            {
                _culledPagedLODs->highresCulled.emplace_back(&plod);
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

    if (_state->intersect(cullGroup.bound))
    {
        // debug("Passed node");
        cullGroup.traverse(*this);
    }
}

void RecordTraversal::apply(const CullNode& cullNode)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "CullNode", COLOR_RECORD_L2, &cullNode);

    if (_state->intersect(cullNode.bound))
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

    regionsOfInterest.emplace_back(_state->modelviewMatrixStack.top(), &roi);
}

void RecordTraversal::apply(const DepthSorted& depthSorted)
{
    CPU_INSTRUMENTATION_L2_NCO(instrumentation, "DepthSorted", COLOR_RECORD_L2, &depthSorted);

    if (_state->intersect(depthSorted.bound))
    {
        const auto& mv = _state->modelviewMatrixStack.top();
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
    _state->record();
    vd.record(*(_state->_commandBuffer));
}

void RecordTraversal::apply(const VertexIndexDraw& vid)
{
    GPU_INSTRUMENTATION_L3_NCO(instrumentation, *getCommandBuffer(), "VertexIndexDraw", COLOR_GPU, &vid);

    //debug("Visiting VertexIndexDraw");
    _state->record();
    vid.record(*(_state->_commandBuffer));
}

void RecordTraversal::apply(const Geometry& geometry)
{
    GPU_INSTRUMENTATION_L3_NCO(instrumentation, *getCommandBuffer(), "Geometry", COLOR_GPU, &geometry);

    //debug("Visiting Geometry");
    _state->record();
    geometry.record(*(_state->_commandBuffer));
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
    if (_viewDependentState) _viewDependentState->ambientLights.emplace_back(_state->modelviewMatrixStack.top(), &light);
}

void RecordTraversal::apply(const DirectionalLight& light)
{
    CPU_INSTRUMENTATION_L2_O(instrumentation, &light);

    //debug("RecordTraversal::apply(DirectionalLight) ", light.className());
    if (_viewDependentState) _viewDependentState->directionalLights.emplace_back(_state->modelviewMatrixStack.top(), &light);
}

void RecordTraversal::apply(const PointLight& light)
{
    CPU_INSTRUMENTATION_L2_O(instrumentation, &light);

    //debug("RecordTraversal::apply(PointLight) ", light.className());
    if (_viewDependentState) _viewDependentState->pointLights.emplace_back(_state->modelviewMatrixStack.top(), &light);
}

void RecordTraversal::apply(const SpotLight& light)
{
    CPU_INSTRUMENTATION_L2_O(instrumentation, &light);

    //debug("RecordTraversal::apply(SpotLight) ", light.className());
    if (_viewDependentState) _viewDependentState->spotLights.emplace_back(_state->modelviewMatrixStack.top(), &light);
}

// transform nodes
void RecordTraversal::apply(const Transform& transform)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "Transform", COLOR_RECORD_L2, &transform);

    _state->modelviewMatrixStack.push(transform);
    _state->dirty = true;

    if (transform.subgraphRequiresLocalFrustum)
    {
        _state->pushFrustum();
        transform.traverse(*this);
        _state->popFrustum();
    }
    else
    {
        transform.traverse(*this);
    }

    _state->modelviewMatrixStack.pop();
    _state->dirty = true;
}

void RecordTraversal::apply(const MatrixTransform& mt)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "MatrixTransform", COLOR_RECORD_L2, &mt);

    _state->modelviewMatrixStack.push(mt);
    _state->dirty = true;

    if (mt.subgraphRequiresLocalFrustum)
    {
        _state->pushFrustum();
        mt.traverse(*this);
        _state->popFrustum();
    }
    else
    {
        mt.traverse(*this);
    }

    _state->modelviewMatrixStack.pop();
    _state->dirty = true;
}

void RecordTraversal::apply(const CoordinateFrame& cf)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "CoordinateFrame", COLOR_RECORD_L2, &cf);

    const View* parentView = _viewDependentState ? _viewDependentState->view : nullptr;
    const Camera* camera = parentView ? parentView->camera : nullptr;
    const ViewMatrix* viewMatrix = camera ? camera->viewMatrix : nullptr;

    if (viewMatrix)
    {
        _state->modelviewMatrixStack.push(viewMatrix->transform(cf.origin) * vsg::rotate(cf.rotation));
    }
    else
    {
        _state->modelviewMatrixStack.push(cf);
    }

    _state->dirty = true;

    if (cf.subgraphRequiresLocalFrustum)
    {
        _state->pushFrustum();
        cf.traverse(*this);
        _state->popFrustum();
    }
    else
    {
        cf.traverse(*this);
    }

    _state->modelviewMatrixStack.pop();
    _state->dirty = true;
}

// Animation nodes
void RecordTraversal::apply(const Joint&)
{
    CPU_INSTRUMENTATION_L2(instrumentation);

    // non op for RiggedJoint as it's designed not to have any renderable children
}

// Vulkan nodes
void RecordTraversal::apply(const StateGroup& stateGroup)
{
    GPU_INSTRUMENTATION_L2_NCO(instrumentation, *getCommandBuffer(), "StateGroup", COLOR_RECORD_L2, &stateGroup);

    //debug("Visiting StateGroup");

    auto begin = stateGroup.stateCommands.begin();
    auto end = stateGroup.stateCommands.end();

    _state->push(begin, end);

    stateGroup.traverse(*this);

    _state->pop(begin, end);
}

void RecordTraversal::apply(const Commands& commands)
{
    GPU_INSTRUMENTATION_L3_NCO(instrumentation, *getCommandBuffer(), "Commands", COLOR_GPU, &commands);

    _state->record();
    for (auto& command : commands.children)
    {
        command->record(*(_state->_commandBuffer));
    }
}

void RecordTraversal::apply(const Command& command)
{
    GPU_INSTRUMENTATION_L3_NCO(instrumentation, *getCommandBuffer(), "Command", COLOR_GPU, &command);

    //debug("Visiting Command");
    _state->record();
    command.record(*(_state->_commandBuffer));
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

    // note, View::accept() updates the RecordTraversal's traversalMask
    auto cached_traversalMask = _state->_commandBuffer->traversalMask;
    _state->_commandBuffer->traversalMask = traversalMask;
    _state->_commandBuffer->viewID = view.viewID;
    _state->_commandBuffer->viewDependentState = view.viewDependentState.get();

    // cache the previous bins
    int32_t cached_minimumBinNumber = _minimumBinNumber;
    decltype(_bins) cached_bins;
    cached_bins.swap(_bins);
    auto cached_viewDependentState = _viewDependentState;

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

    _minimumBinNumber = min_binNumber;
    _bins.resize(max_binNumber - min_binNumber + 1);
    for (auto& bin : view.bins)
    {
        _bins[bin->binNumber - min_binNumber] = bin;
        bin->clear();
    }

    // assign and clear the View's ViewDependentState
    _viewDependentState = view.viewDependentState;
    if (_viewDependentState)
    {
        _viewDependentState->clear();
    }

    if (view.camera)
    {
        _state->inheritViewForLODScaling = (view.features & INHERIT_VIEWPOINT) != 0;
        _state->setProjectionAndViewMatrix(view.camera->projectionMatrix->transform(), view.camera->viewMatrix->transform());

        if (const auto& viewportState = view.camera->viewportState)
        {
            _state->push(viewportState);

            if (_viewDependentState)
            {
                auto& viewportData = _viewDependentState->viewportData;
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

            _state->pop(viewportState);
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

    for (auto& bin : view.bins)
    {
        bin->accept(*this);
    }

    if (_viewDependentState)
    {
        _viewDependentState->traverse(*this);
    }

    // swap back previous bin setup.
    _minimumBinNumber = cached_minimumBinNumber;
    cached_bins.swap(_bins);
    cached_regionsOfInterest.swap(regionsOfInterest);
    _state->_commandBuffer->traversalMask = cached_traversalMask;
    _viewDependentState = cached_viewDependentState;
}

void RecordTraversal::apply(const CommandGraph& commandGraph)
{
    GPU_INSTRUMENTATION_L1_NCO(instrumentation, *getCommandBuffer(), "RecordTraversal CommandGraph", COLOR_RECORD_L1, &commandGraph);

    if (recordedCommandBuffers)
    {
        auto cg = const_cast<CommandGraph*>(&commandGraph);
        if (cg->device)
        {
            if (_viewDependentState && _viewDependentState->view && _viewDependentState->view->camera)
            {
                auto camera = _viewDependentState->view->camera;
                cg->getOrCreateRecordTraversal()->_state->setInhertiedViewProjectionAndViewMatrix(camera->projectionMatrix->transform(), camera->viewMatrix->transform());
            }

            cg->record(recordedCommandBuffers, _frameStamp, _databasePager);
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
    _bins[binNumber - _minimumBinNumber]->add(_state, value, node);
}
