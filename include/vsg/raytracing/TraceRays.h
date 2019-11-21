#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2019 Thomas Hogarth

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Command.h>
#include <vsg/vk/Buffer.h>

namespace vsg
{

    class VSG_DECLSPEC TraceRays : public Inherit<Command, TraceRays>
    {
    public:

        TraceRays();

        void dispatch(CommandBuffer& commandBuffer) const override;

#if 0
        using BufferType = ref_ptr<Buffer>;
#else
        using BufferType = VkBuffer;
#endif

        BufferType raygenShaderBindingTableBuffer;
        VkDeviceSize raygenShaderBindingOffset = 0;
        BufferType missShaderBindingTableBuffer;
        VkDeviceSize missShaderBindingOffset = 0;
        VkDeviceSize missShaderBindingStride = 0;
        BufferType hitShaderBindingTableBuffer;
        VkDeviceSize hitShaderBindingOffset;
        VkDeviceSize hitShaderBindingStride = 0;
        BufferType callableShaderBindingTableBuffer;
        VkDeviceSize callableShaderBindingOffset = 0;
        VkDeviceSize callableShaderBindingStride = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t depth = 0;
    };

} // namespace vsg
