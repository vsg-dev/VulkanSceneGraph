#pragma once

#include <vsg/core/Data.h>
#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>

#include <iostream>

namespace vsg
{

   class PushConstants : public Command
    {
    public:
        PushConstants(VkShaderStageFlags shaderFlags, uint32_t offset, Data* data);

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        Data* getData() { return _data; }
        const Data* getData() const { return _data; }

        virtual void dispatch(CommandBuffer& commandBuffer) const
        {
            const PipelineLayout* pipelineLayout = commandBuffer.getCurrentPipelineLayout();
            std::cout<<"vkCmdPushConstants(pipeline="<<commandBuffer.getCurrentPipeline()<<", pipelineLayout="<<pipelineLayout<<",_stageFlags="<<_stageFlags<<" ._offset="<<_offset<<" _data->dataSize()="<<_data->dataSize()<<std::endl;
            if (pipelineLayout)
            {

                vkCmdPushConstants(commandBuffer, *pipelineLayout, _stageFlags, _offset, _data->dataSize(), _data->dataPointer());
            }
        }

    protected:
        virtual ~PushConstants();

        VkShaderStageFlags  _stageFlags;
        uint32_t            _offset;
        ref_ptr<Data>       _data;
    };


}
