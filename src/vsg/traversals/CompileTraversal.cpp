/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/nodes/Commands.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/PagedLOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <vsg/viewer/ExecuteCommands.h>
#include <vsg/viewer/CommandGraph.h>
#include <vsg/viewer/RenderGraph.h>

using namespace vsg;

/////////////////////////////////////////////////////////////////////
//
// CollectDescriptorStats
//
void CollectDescriptorStats::apply(const Object& object)
{
    object.traverse(*this);
}

bool CollectDescriptorStats::checkForResourceHints(const Object& object)
{
    const Object* rh_object = object.getObject("ResourceHints");
    const ResourceHints* resourceHints = dynamic_cast<const ResourceHints*>(rh_object);
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

void CollectDescriptorStats::apply(const ResourceHints& resourceHints)
{
    if (resourceHints.getMaxSlot() > maxSlot) maxSlot = resourceHints.getMaxSlot();

    if (!resourceHints.getDescriptorPoolSizes().empty() || resourceHints.getNumDescriptorSets() > 9)
    {
        externalNumDescriptorSets += resourceHints.getNumDescriptorSets();

        for (auto& [type, count] : resourceHints.getDescriptorPoolSizes())
        {
            descriptorTypeMap[type] += count;
        }
    }
}

void CollectDescriptorStats::apply(const Node& node)
{
    if (checkForResourceHints(node)) return;

    node.traverse(*this);
}

void CollectDescriptorStats::apply(const StateGroup& stategroup)
{
    if (checkForResourceHints(stategroup)) return;

    for (auto& command : stategroup.getStateCommands())
    {
        command->accept(*this);
    }

    stategroup.traverse(*this);
}

void CollectDescriptorStats::apply(const StateCommand& stateCommand)
{
    if (stateCommand.getSlot() > maxSlot) maxSlot = stateCommand.getSlot();

    stateCommand.traverse(*this);
}

void CollectDescriptorStats::apply(const DescriptorSet& descriptorSet)
{
    if (descriptorSets.count(&descriptorSet) == 0)
    {
        descriptorSets.insert(&descriptorSet);

        descriptorSet.traverse(*this);
    }
}

void CollectDescriptorStats::apply(const Descriptor& descriptor)
{
    if (descriptors.count(&descriptor) == 0)
    {
        descriptors.insert(&descriptor);
    }
    descriptorTypeMap[descriptor._descriptorType] += descriptor.getNumDescriptors();
}
void CollectDescriptorStats::apply(const CommandGraph& object)
{
for(auto sec : object._secondaries)
    sec->accept(*this);

    object.traverse(*this);
}

uint32_t CollectDescriptorStats::computeNumDescriptorSets() const
{
    return externalNumDescriptorSets + static_cast<uint32_t>(descriptorSets.size());
}

DescriptorPoolSizes CollectDescriptorStats::computeDescriptorPoolSizes() const
{
    DescriptorPoolSizes poolSizes;
    for (auto& [type, count] : descriptorTypeMap)
    {
        poolSizes.push_back(VkDescriptorPoolSize{type, count});
    }
    return poolSizes;
}

/////////////////////////////////////////////////////////////////////
//
// CompielTraversal
//
CompileTraversal::CompileTraversal(Device* in_device, BufferPreferences bufferPreferences) :
    context(in_device, bufferPreferences)
{
}
CompileTraversal::CompileTraversal(const CompileTraversal& ct) :
    context(ct.context)
{
}

CompileTraversal::~CompileTraversal()
{
}

void CompileTraversal::apply(Object& object)
{
    object.traverse(*this);
}

void CompileTraversal::apply(Command& command)
{
    command.compile(context);
}

void CompileTraversal::apply(Commands& commands)
{
    commands.compile(context);
}

void CompileTraversal::apply(StateGroup& stateGroup)
{
    stateGroup.compile(context);
    stateGroup.traverse(*this);
}

void CompileTraversal::apply(Geometry& geometry)
{
    geometry.compile(context);
    geometry.traverse(*this);
}

class CollectSecondaryCommandGraph : public ConstVisitor
{
public:
    vsg::CommandGraphs _secondaries;
    void apply(const Group& group) override
    {
        group.traverse(*this);
    }
    void apply(const Command& cmd) override{
        const vsg::ExecuteCommands *exec = dynamic_cast<const vsg::ExecuteCommands*>(&cmd);
        if(exec)
        {
            for( auto g :exec->_cmdgraphs)
            {
                _secondaries.emplace_back(g);
            }
        }
    }
};
void CompileTraversal::apply(CommandGraph& commandGraph)
{
    //TODO prune Secondaty ComamandGraphs if primary
    if(commandGraph._commandbufferslevel == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
    {

        context.renderPass = static_cast<RenderGraph*>(commandGraph.getChild(0))->window->renderPass();
        context.viewport = static_cast<RenderGraph*>(commandGraph.getChild(0))->camera->getViewportState();


        context.viewport->getViewport().width = static_cast<RenderGraph*>(commandGraph.getChild(0))->window->extent2D().width;
        context.viewport->getViewport().height = static_cast<RenderGraph*>(commandGraph.getChild(0))->window->extent2D().height;

        CollectSecondaryCommandGraph col;
      //  commandGraph.accept(col);
        for(auto g: commandGraph._secondaries)
        {
            this->apply(*g.get());//g->accept(*this);
     /*  commandGraph->addS g->

            g->setPrimaryCommandGraph(commandGraph);///set Primary in order t
*/
        }

    }
    commandGraph.traverse(*this);


}
void CompileTraversal::apply(RenderGraph& renderGraph)
{
    context.renderPass = renderGraph.window->renderPass();

    if (renderGraph.camera)
    {
        context.viewport = renderGraph.camera->getViewportState();
    }
    else
    {
        context.viewport = vsg::ViewportState::create(renderGraph.window->extent2D());
    }

    renderGraph.traverse(*this);
}
