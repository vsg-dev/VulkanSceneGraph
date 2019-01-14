#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Data.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/vk/Device.h>

namespace vsg
{

    class VSG_DECLSPEC PushConstants : public Inherit<StateComponent, PushConstants>
    {
    public:
        PushConstants(VkShaderStageFlags shaderFlags, uint32_t offset, Data* data);

        Data* getData() noexcept { return _data; }
        const Data* getData() const noexcept { return _data; }

        void pushTo(State& state) const override;
        void popFrom(State& state) const override;
        void dispatch(CommandBuffer& commandBuffer) const override;

    protected:
        virtual ~PushConstants();

        VkShaderStageFlags _stageFlags;
        uint32_t _offset;
        ref_ptr<Data> _data;
    };
    VSG_type_name(vsg::PushConstants);

} // namespace vsg
