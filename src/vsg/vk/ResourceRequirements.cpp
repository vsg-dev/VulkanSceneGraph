/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/commands/Command.h>
#include <vsg/commands/Commands.h>
#include <vsg/nodes/Bin.h>
#include <vsg/nodes/DepthSorted.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/RenderGraph.h>
#include <vsg/viewer/View.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

/////////////////////////////////////////////////////////////////////
//
// ResourceRequirements
//
ResourceRequirements::ResourceRequirements()
{
}

uint32_t ResourceRequirements::computeNumDescriptorSets() const
{
    return externalNumDescriptorSets + static_cast<uint32_t>(descriptorSets.size());
}

DescriptorPoolSizes ResourceRequirements::computeDescriptorPoolSizes() const
{
    DescriptorPoolSizes poolSizes;
    for (auto& [type, count] : descriptorTypeMap)
    {
        poolSizes.push_back(VkDescriptorPoolSize{type, count});
    }
    return poolSizes;
}

//////////////////////////////////////////////////////////////////////
//
// CollectResourceRequirements
//
ref_ptr<ResourceHints> CollectResourceRequirements::createResourceHints(uint32_t tileMultiplier) const
{
    auto resourceHints = vsg::ResourceHints::create();

    resourceHints->maxSlot = requirements.maxSlot;
    resourceHints->numDescriptorSets = static_cast<uint32_t>(requirements.computeNumDescriptorSets() * tileMultiplier);
    resourceHints->descriptorPoolSizes = requirements.computeDescriptorPoolSizes();

    for (auto& poolSize : resourceHints->descriptorPoolSizes)
    {
        poolSize.descriptorCount = poolSize.descriptorCount * tileMultiplier;
    }

    return resourceHints;
}

void CollectResourceRequirements::apply(const Object& object)
{
    object.traverse(*this);
}

bool CollectResourceRequirements::checkForResourceHints(const Object& object)
{
    auto resourceHints = object.getObject<ResourceHints>("ResourceHints");
    if (resourceHints)
    {
        apply(*resourceHints);
        return true;
    }
    else
    {
        return false;
    }
}

void CollectResourceRequirements::apply(const ResourceHints& resourceHints)
{
    if (resourceHints.maxSlot > requirements.maxSlot) requirements.maxSlot = resourceHints.maxSlot;

    if (!resourceHints.descriptorPoolSizes.empty() || resourceHints.numDescriptorSets > 0)
    {
        requirements.externalNumDescriptorSets += resourceHints.numDescriptorSets;

        for (auto& [type, count] : resourceHints.descriptorPoolSizes)
        {
            requirements.descriptorTypeMap[type] += count;
        }
    }
}

void CollectResourceRequirements::apply(const Node& node)
{
    bool hasResourceHints = checkForResourceHints(node);
    if (hasResourceHints) ++_numResourceHintsAbove;

    node.traverse(*this);

    if (hasResourceHints) --_numResourceHintsAbove;
}

void CollectResourceRequirements::apply(const StateGroup& stategroup)
{
    bool hasResourceHints = checkForResourceHints(stategroup);
    if (hasResourceHints) ++_numResourceHintsAbove;

    if (_numResourceHintsAbove == 0)
    {
        for (auto& command : stategroup.stateCommands)
        {
            command->accept(*this);
        }
    }

    stategroup.traverse(*this);

    if (hasResourceHints) --_numResourceHintsAbove;
}

void CollectResourceRequirements::apply(const PagedLOD& plod)
{
    bool hasResourceHints = checkForResourceHints(plod);
    if (hasResourceHints) ++_numResourceHintsAbove;

    requirements.containsPagedLOD = true;
    plod.traverse(*this);

    if (hasResourceHints) --_numResourceHintsAbove;
}

void CollectResourceRequirements::apply(const StateCommand& stateCommand)
{
    if (stateCommand.slot > requirements.maxSlot) requirements.maxSlot = stateCommand.slot;

    stateCommand.traverse(*this);
}

void CollectResourceRequirements::apply(const DescriptorSet& descriptorSet)
{
    if (requirements.descriptorSets.count(&descriptorSet) == 0)
    {
        requirements.descriptorSets.insert(&descriptorSet);

        descriptorSet.traverse(*this);
    }
}

void CollectResourceRequirements::apply(const Descriptor& descriptor)
{
    if (requirements.descriptors.count(&descriptor) == 0)
    {
        requirements.descriptors.insert(&descriptor);
    }
    requirements.descriptorTypeMap[descriptor.descriptorType] += descriptor.getNumDescriptors();
}

void CollectResourceRequirements::apply(const View& view)
{
    if (auto itr = requirements.views.find(&view); itr != requirements.views.end())
    {
        requirements.binStack.push(itr->second);
    }
    else
    {
        requirements.binStack.push(ResourceRequirements::BinDetails{});
    }

    view.traverse(*this);

    for (auto& bin : view.bins)
    {
        requirements.binStack.top().bins.insert(bin);
    }

    requirements.views[&view] = requirements.binStack.top();

    requirements.binStack.pop();
}

void CollectResourceRequirements::apply(const DepthSorted& depthSorted)
{
    requirements.binStack.top().indices.insert(depthSorted.binNumber);

    depthSorted.traverse(*this);
}

void CollectResourceRequirements::apply(const Bin& bin)
{
    requirements.binStack.top().bins.insert(&bin);
}
