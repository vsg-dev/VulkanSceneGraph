/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/RecordTraversal.h>
#include <vsg/app/View.h>
#include <vsg/commands/Command.h>
#include <vsg/commands/Commands.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/io/stream.h>
#include <vsg/maths/plane.h>
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/DepthSorted.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/Light.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/Switch.h>
#include <vsg/threading/atomics.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

using namespace vsg;

#define INLINE_TRAVERSE 0

RecordTraversal::RecordTraversal(CommandBuffer* in_commandBuffer, uint32_t in_maxSlot, std::set<Bin*> in_bins) :
    _state(new State(in_commandBuffer, in_maxSlot))
{
    if (_frameStamp) _frameStamp->ref();
    if (_state) _state->ref();

    _minimumBinNumber = 0;
    int32_t maximumBinNumber = 0;
    for (auto& bin : in_bins)
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
    if (_culledPagedLODs) _culledPagedLODs->unref();
    if (_databasePager) _databasePager->unref();
    if (_state) _state->unref();
    if (_frameStamp) _frameStamp->unref();
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
    if (fs == _frameStamp) return;

    if (_frameStamp) _frameStamp->unref();

    _frameStamp = fs;

    if (_frameStamp) _frameStamp->ref();
}

void RecordTraversal::setDatabasePager(DatabasePager* dp)
{
    if (dp == _databasePager) return;

    if (_databasePager) _databasePager->unref();
    if (_culledPagedLODs) _culledPagedLODs->unref();

    _databasePager = dp;
    _culledPagedLODs = dp ? dp->culledPagedLODs.get() : nullptr;

    if (_databasePager) _databasePager->ref();
    if (_culledPagedLODs) _culledPagedLODs->ref();
}

void RecordTraversal::setProjectionAndViewMatrix(const dmat4& projMatrix, const dmat4& viewMatrix)
{
    _state->setProjectionAndViewMatrix(projMatrix, viewMatrix);
}

void RecordTraversal::clearBins()
{
    for (auto& bin : _bins)
    {
        if (bin) bin->clear();
    }
}

void RecordTraversal::apply(const Object& object)
{
    //debug("Visiting Object");
    object.traverse(*this);
}

void RecordTraversal::apply(const Group& group)
{
    //debug("Visiting Group");
#if INLINE_TRAVERSE
    vsg::Group::t_traverse(group, *this);
#else
    group.traverse(*this);
#endif
}

void RecordTraversal::apply(const QuadGroup& quadGroup)
{
    //debug("Visiting QuadGroup");
#if INLINE_TRAVERSE
    vsg::QuadGroup::t_traverse(quadGroup, *this);
#else
    quadGroup.traverse(*this);
#endif
}

void RecordTraversal::apply(const LOD& lod)
{
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
                    // we are first request so tell the databasePager about it
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

void RecordTraversal::apply(const CullGroup& cullGroup)
{
    if (_state->intersect(cullGroup.bound))
    {
        // debug("Passed node");
        cullGroup.traverse(*this);
    }
}

void RecordTraversal::apply(const CullNode& cullNode)
{
    if (_state->intersect(cullNode.bound))
    {
        //debug("Passed node");
        cullNode.traverse(*this);
    }
}

void RecordTraversal::apply(const DepthSorted& depthSorted)
{
    if (_state->intersect(depthSorted.bound))
    {
        const auto& mv = _state->modelviewMatrixStack.top();
        auto& center = depthSorted.bound.center;
        auto distance = -(mv[0][2] * center.x + mv[1][2] * center.y + mv[2][2] * center.z + mv[3][2]);

        _bins[depthSorted.binNumber - _minimumBinNumber]->add(_state, distance, depthSorted.child);
    }
}

void RecordTraversal::apply(const Switch& sw)
{
    for (auto& child : sw.children)
    {
        if ((traversalMask & (overrideMask | child.mask)) != MASK_OFF)
        {
            child.node->accept(*this);
        }
    }
}

void RecordTraversal::apply(const Light& /*light*/)
{
    //debug("RecordTraversal::apply(Light) ", light.className());
}

void RecordTraversal::apply(const AmbientLight& light)
{
    //debug("RecordTraversal::apply(AmbientLight) ", light.className());
    if (_viewDependentState) _viewDependentState->ambientLights.emplace_back(_state->modelviewMatrixStack.top(), &light);
}

void RecordTraversal::apply(const DirectionalLight& light)
{
    //debug("RecordTraversal::apply(DirectionalLight) ", light.className());
    if (_viewDependentState) _viewDependentState->directionalLights.emplace_back(_state->modelviewMatrixStack.top(), &light);
}

void RecordTraversal::apply(const PointLight& light)
{
    //debug("RecordTraversal::apply(PointLight) ", light.className());
    if (_viewDependentState) _viewDependentState->pointLights.emplace_back(_state->modelviewMatrixStack.top(), &light);
}

void RecordTraversal::apply(const SpotLight& light)
{
    //debug("RecordTraversal::apply(SpotLight) ", light.className());
    if (_viewDependentState) _viewDependentState->spotLights.emplace_back(_state->modelviewMatrixStack.top(), &light);
}

void RecordTraversal::apply(const StateGroup& stateGroup)
{
    //debug("Visiting StateGroup");

    for (auto& command : stateGroup.stateCommands)
    {
        _state->stateStacks[command->slot].push(command);
    }
    _state->dirty = true;

    stateGroup.traverse(*this);

    for (auto& command : stateGroup.stateCommands)
    {
        _state->stateStacks[command->slot].pop();
    }
    _state->dirty = true;
}

void RecordTraversal::apply(const Transform& transform)
{
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

// Vulkan nodes
void RecordTraversal::apply(const Commands& commands)
{
    _state->record();
    for (auto& command : commands.children)
    {
        command->record(*(_state->_commandBuffer));
    }
}

void RecordTraversal::apply(const Command& command)
{
    //debug("Visiting Command");
    _state->record();
    command.record(*(_state->_commandBuffer));
}

void RecordTraversal::apply(const View& view)
{
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

    // assign and clear the View's bins
    int32_t min_binNumber = 0;
    int32_t max_binNumber = 0;
    for (auto& bin : view.bins)
    {
        if (bin->binNumber < min_binNumber) min_binNumber = bin->binNumber;
        if (bin->binNumber > max_binNumber) max_binNumber = bin->binNumber;
    }

    _minimumBinNumber = min_binNumber;
    _bins.resize(max_binNumber - min_binNumber + 1);
    for (auto& bin : view.bins)
    {
        _bins[bin->binNumber] = bin;
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
        setProjectionAndViewMatrix(view.camera->projectionMatrix->transform(), view.camera->viewMatrix->transform());

        if (_viewDependentState && _viewDependentState->viewportData && view.camera->viewportState)
        {
            auto& viewportData = _viewDependentState->viewportData;
            auto& viewports = view.camera->viewportState->viewports;

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

        view.traverse(*this);
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
        _viewDependentState->pack();
    }

    // swap back previous bin setup.
    _minimumBinNumber = cached_minimumBinNumber;
    cached_bins.swap(_bins);
    _state->_commandBuffer->traversalMask = cached_traversalMask;
    _viewDependentState = cached_viewDependentState;
}
