#pragma once

#include <vsg/core/Data.h>
#include <vsg/vk/Device.h>
#include <vsg/nodes/StateGroup.h>

namespace vsg
{

   class VSG_EXPORT PushConstants : public StateComponent
    {
    public:
        PushConstants(VkShaderStageFlags shaderFlags, uint32_t offset, Data* data);

        virtual void accept(Visitor& visitor) { visitor.apply(*this); }

        Data* getData() { return _data; }
        const Data* getData() const { return _data; }

        virtual void pushTo(State& state) override;
        virtual void popFrom(State& state) override;
        virtual void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~PushConstants();

        VkShaderStageFlags  _stageFlags;
        uint32_t            _offset;
        ref_ptr<Data>       _data;
    };


}
