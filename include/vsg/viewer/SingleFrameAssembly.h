#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/viewer/FrameAssembly.h>

namespace vsg
{
    class VSG_DECLSPEC SingleFrameAssembly : public Inherit<FrameAssembly, SingleFrameAssembly>
    {
    public:
        SingleFrameAssembly(ref_ptr<Framebuffer> frameBuffer, ref_ptr<RenderPass> renderPass,
                            const ClearValues clearValues,
                            const VkExtent2D& extent2D = VkExtent2D{},
                            VkSampleCountFlagBits sampleBits = VK_SAMPLE_COUNT_1_BIT)
            : _frameBuffer(std::move(frameBuffer)), _renderPass(std::move(renderPass)),
              _clearValues(clearValues),
              _extent2D(extent2D),
              _sampleBits(sampleBits)
        {
        }
        FrameRender getFrameRender() override;
        ref_ptr<Device> getDevice() const override;
        const VkExtent2D& getExtent2D() const override;
        void setExtent2D(VkExtent2D extent2D)
        {
            _extent2D = std::move(extent2D);
        }
    protected:
        ref_ptr<Framebuffer> _frameBuffer;
        ref_ptr<RenderPass> _renderPass;
        ClearValues _clearValues;
        VkExtent2D _extent2D;
        VkSampleCountFlagBits _sampleBits;
    };
}
