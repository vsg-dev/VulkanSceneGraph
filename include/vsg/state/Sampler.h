#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Device.h>
#include <vsg/vk/vk_buffer.h>

namespace vsg
{
    class Context;

    class VSG_DECLSPEC Sampler : public Inherit<Object, Sampler>
    {
    public:
        Sampler();

        void read(Input& input) override;
        void write(Output& output) const override;

        void compile(Context& context);

        void release(uint32_t deviceID) { _implementation[deviceID] = {}; }
        void release() { _implementation.clear(); }

        VkSamplerCreateInfo& info() { return _samplerInfo; }
        const VkSamplerCreateInfo& info() const { return _samplerInfo; }

        VkSampler vk(uint32_t deviceID) const { return _implementation[deviceID]->_sampler; }

    protected:
        virtual ~Sampler();

        struct Implementation : public Inherit<Object, Implementation>
        {
            Implementation(Device* device, const VkSamplerCreateInfo& createSamplerInfo);

            virtual ~Implementation();

            VkSampler _sampler;
            ref_ptr<Device> _device;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;

        VkSamplerCreateInfo _samplerInfo;
    };
    VSG_type_name(vsg::Sampler)

} // namespace vsg
