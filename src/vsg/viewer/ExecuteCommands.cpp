/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/ui/ApplicationEvent.h>
#include <vsg/io/DatabasePager.h>
#include <vsg/viewer/ExecuteCommands.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;
/*
ExecuteCommands::ExecuteCommands(Data* indices) :
    _bufferData(nullptr, 0, 0, indices),
    _indexType(computeIndexType(indices))
{
}

ExecuteCommands::ExecuteCommands(const BufferData& bufferData) :
    _bufferData(bufferData),
    _indexType(computeIndexType(bufferData._data))
{
}
*/
ExecuteCommands::~ExecuteCommands()
{
   /* if (_bufferData._buffer)
    {
        _bufferData._buffer->release(_bufferData._offset, 0); // TODO, we don't locally have a size allocated
    }*/
}

void ExecuteCommands::read(Input& input)
{
   Command::read(input);
  //   Group::read(input);

    // reset the Vulkan related objects
    /*_bufferData._buffer = 0;
    _bufferData._offset = 0;
    _bufferData._range = 0;

    // read the key indices data
    _bufferData._data = input.readObject<Data>("Indices");*/
}

void ExecuteCommands::write(Output& output) const
{
  Command::write(output);
    //  Group::write(output);

    // write indices data
   // output.writeObject("Indices", _bufferData._data.get());
}
#if 0
struct DeviceResources
{
    vsg::CollectDescriptorStats collectStats;
    vsg::ref_ptr<vsg::DescriptorPool> descriptorPool;
    vsg::ref_ptr<vsg::CompileTraversal> compile;
};

void ExecuteCommands::traverse(Visitor& visitor)  {
    ///assume compile
    visitor.st
   // t_traverse(*this, visitor);
    // find which devices are available
    BufferPreferences buffpref;
    using DeviceResourceMap = std::map<vsg::Device*, DeviceResources>;
    DeviceResourceMap deviceResourceMap;

        for (auto& commandGraph :_cmdgraphs)
        {
            auto& deviceResources = deviceResourceMap[commandGraph->_device];
            commandGraph->accept(deviceResources.collectStats);
        }

    // allocate DescriptorPool for each Device
    for (auto& [device, deviceResource] : deviceResourceMap)
    {
        auto physicalDevice = device->getPhysicalDevice();

        auto& collectStats = deviceResource.collectStats;
        auto maxSets = std::max(1u,collectStats.computeNumDescriptorSets());
        const auto& descriptorPoolSizes =collectStats.computeDescriptorPoolSizes();

        deviceResource.compile = new vsg::CompileTraversal(device, buffpref);
        if(!descriptorPoolSizes.empty())
        deviceResource.compile->context.descriptorPool = vsg::DescriptorPool::create(device, maxSets, descriptorPoolSizes);
        deviceResource.compile->context.commandPool = vsg::CommandPool::create(device, physicalDevice->getGraphicsFamily());
        deviceResource.compile->context.graphicsQueue = device->getQueue(physicalDevice->getGraphicsFamily());
    }

    //CompileTraversal trav(context.device);
 //   trav.context=context;
    for( auto sec : _cmdgraphs)
    {
        auto& deviceResource = deviceResourceMap[sec->_device];
        sec->_maxSlot = deviceResource.collectStats.maxSlot;
        deviceResource.compile->context=context;
        sec->accept(*deviceResource.compile);
    }

    records.clear();
    for( auto sec : _cmdgraphs)
        sec->record(records);// context.frameStamp, context.databasePager)
}
void ExecuteCommands::traverse(ConstVisitor& visitor) const  {
    ///assume CollectdescriptorStat
   // t_traverse(*this, visitor);
}
void ExecuteCommands::traverse(RecordTraversal& visitor) const  {
   // t_traverse(*this, visitor);
}
void ExecuteCommands::traverse(CullTraversal& visitor) const  {
 //   t_traverse(*this, visitor);
}

void ExecuteCommands::accept(RecordTraversal& dispatchTraversal) const
{VkCommandBuffer vk_commandBuffer = *(dispatchTraversal.state->_commandBuffer);

    /*
    std::vector<ref_ptr<CommandBuffer> > records;
;*/
    _commandbuffers.clear();

    for(auto r : records)
        _commandbuffers.emplace_back(*r);

    vkCmdExecuteCommands(vk_commandBuffer, _commandbuffers.size(), _commandbuffers.data());


    // traverse the command buffer to place the commands into the command buffer.
   // traverse(dispatchTraversal);

  //  vkCmdEndRenderPass(vk_commandBuffer);
}

#else


void ExecuteCommands::dispatch(CommandBuffer& commandBuffer) const
{

    //std::vector<ref_ptr<CommandBuffer> > records;

    _commandbuffers.clear();

    for(auto r : _cmdgraphs)
        _commandbuffers.emplace_back(*r->lastrecorded);

    vkCmdExecuteCommands(commandBuffer, _commandbuffers.size(), _commandbuffers.data());
}
class UpdatePipeline : public vsg::Visitor
{
public:
    Context context;

    UpdatePipeline(Device* device) :
        context(device) {}

    void apply(vsg::BindGraphicsPipeline& bindPipeline)
    {
        GraphicsPipeline* graphicsPipeline = bindPipeline.getPipeline();
        if (graphicsPipeline)
        {
            bool needToRegenerateGraphicsPipeline = false;
            for (auto& pipelineState : graphicsPipeline->getPipelineStates())
            {
                if (pipelineState == context.viewport)
                {
                    needToRegenerateGraphicsPipeline = true;
                    break;
                }
            }

            if (graphicsPipeline->getImplementation())
            {
                for (auto& pipelineState : graphicsPipeline->getImplementation()->_pipelineStates)
                {
                    if (pipelineState == context.viewport)
                    {
                        needToRegenerateGraphicsPipeline = true;
                        break;
                    }
                }
            }

            if (needToRegenerateGraphicsPipeline)
            {
                vsg::ref_ptr<vsg::GraphicsPipeline> new_pipeline = vsg::GraphicsPipeline::create(graphicsPipeline->getPipelineLayout(), graphicsPipeline->getShaderStages(), graphicsPipeline->getPipelineStates(), graphicsPipeline->getSubPass());

                bindPipeline.release();

                bindPipeline.setPipeline(new_pipeline);

                bindPipeline.compile(context);
            }
        }
    }

    void apply(vsg::Object& object)
    {
        object.traverse(*this);
    }

    void apply(vsg::StateGroup& sg)
    {
        for (auto& command : sg.getStateCommands())
        {
            command->accept(*this);
        }
        sg.traverse(*this);
    }
};
struct DeviceResources
{
    vsg::CollectDescriptorStats collectStats;
    vsg::ref_ptr<vsg::DescriptorPool> descriptorPool;
    vsg::ref_ptr<vsg::CompileTraversal> compile;
};

void ExecuteCommands::compile(Context& context)
{
    // find which devices are available
 /* BufferPreferences buffpref;
    using DeviceResourceMap = std::map<vsg::Device*, DeviceResources>;
    DeviceResourceMap deviceResourceMap;

        for (auto& commandGraph :_cmdgraphs)
        {
            auto& deviceResources = deviceResourceMap[commandGraph->_device];
            commandGraph->accept(deviceResources.collectStats);
        }

    // allocate DescriptorPool for each Device
    for (auto& [device, deviceResource] : deviceResourceMap)
    {
        auto physicalDevice = device->getPhysicalDevice();

        auto& collectStats = deviceResource.collectStats;
        auto maxSets = std::max(0u,collectStats.computeNumDescriptorSets());
        const auto& descriptorPoolSizes =collectStats.computeDescriptorPoolSizes();

        deviceResource.compile = new vsg::CompileTraversal(device, buffpref);
        if(!descriptorPoolSizes.empty())
        deviceResource.compile->context.descriptorPool = vsg::DescriptorPool::create(device, maxSets, descriptorPoolSizes);
        deviceResource.compile->context.commandPool = vsg::CommandPool::create(device, physicalDevice->getGraphicsFamily());
        deviceResource.compile->context.graphicsQueue = device->getQueue(physicalDevice->getGraphicsFamily());
    }

    CommandGraph * lastg=0;
    VkPipelineLayout lasty;
    //CompileTraversal trav(context.device);
 //   trav.context=context;
 /*
    for( auto sec : _cmdgraphs)
    {
        auto& deviceResource = deviceResourceMap[sec->_device];
        sec->_maxSlot = deviceResource.collectStats.maxSlot;
      //  deviceResource.compile->context=context;
        deviceResource.compile->context.renderPass=context.renderPass;
        deviceResource.compile->context.viewport=context.viewport;

        sec->accept(*deviceResource.compile);
    }
    records.clear();


   // auto recordTraversal = new RecordTraversal(nullptr,sec-> _maxSlot);

     for( auto sec : _cmdgraphs){

         ref_ptr<CommandBuffer> commandBuffer;
         for (auto& cb :   sec->commandBuffers)
         {
             if (cb->numDependentSubmissions() == 0)
             {
                 commandBuffer = cb;
             }
         }
         if(lastg)
         {

             commandBuffer->setCurrentPipelineLayout(lasty );
         }



         sec->record(records);

         lasty=sec->recordTraversal->state->_commandBuffer->getCurrentPipelineLayout();
        lastg=sec;
       //  records.emplace_back(commandBuffer);

     }* /



    for( auto sec : _cmdgraphs){

        UpdatePipeline updatePipeline( sec->_device );
        auto& deviceResource = deviceResourceMap[sec->_device];
   // sec->recordTraversal=recordTraversal;
 //   updatePipeline.context.commandPool = recordTraversal->state->_commandBuffer->getCommandPool();
    updatePipeline.context.commandPool = deviceResource.compile->context.commandPool;
        updatePipeline.context.renderPass = context.renderPass;
        if(lastg)
        {
            sec->recordTraversal=lastg->recordTraversal;
            ref_ptr<CommandBuffer> commandBuffer;
            for (auto& cb :   sec->commandBuffers)
            {
                if (cb->numDependentSubmissions() == 0)
                {
                    commandBuffer = cb;
                }
            }
            commandBuffer->setCurrentPipelineLayout(lasty );
        }
        else
        {

        }


         (sec)->traverse(updatePipeline);

        sec->record(records);// context.frameStamp, context.databasePager);
        lasty=sec->recordTraversal->state->_commandBuffer->getCurrentPipelineLayout();
       lastg=sec;

    }*/

   "context.commands.emplace_back";
}
#endif
    /*
    commandGraph.traverse(*this);
    for( auto gr : _cmdgraphs)
       gr->traverse(*this);
   /* _commandbuffers.
            _cmdgraphs
    auto seccommandGraph1 = vsg::createCommandGraphForView(window, camera, scenegraph1, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    // check if already compiled
    if (_bufferData._buffer) return;

    auto bufferDataList = vsg::createBufferAndTransferData(context, {_bufferData._data}, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE);
    if (!bufferDataList.empty())
    {
        _bufferData = bufferDataList.back();
        _indexType = computeIndexType(_bufferData._data);
    }*/

