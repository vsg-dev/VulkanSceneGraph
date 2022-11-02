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
    // forward declare
    class Context;

    using DescriptorSetLayoutBindings = std::vector<VkDescriptorSetLayoutBinding>;
    using DescriptorPoolSizes = std::vector<VkDescriptorPoolSize>;

    /// DescriptorSetLayout encapsulates VkDescriptorSetLayout and VkDescriptorSetLayoutCreateInfo settings used to set it up.
    class VSG_DECLSPEC DescriptorSetLayout : public Inherit<Object, DescriptorSetLayout>
    {
    public:
        DescriptorSetLayout();
        explicit DescriptorSetLayout(const DescriptorSetLayoutBindings& descriptorSetLayoutBindings);

        /// Vulkan VkDescriptorSetLayout handle
        virtual VkDescriptorSetLayout vk(uint32_t deviceID) const { return _implementation[deviceID]->_descriptorSetLayout; }

        /// VkDescriptorSetLayoutCreateInfo settings
        DescriptorSetLayoutBindings bindings;

        /// map the descriptor bindings to the descriptor pool sizes that will be required to represent them.
        void getDescriptorPoolSizes(DescriptorPoolSizes& descriptorPoolSizes);

        int compare(const Object& rhs_object) const override;

        void read(Input& input) override;
        void write(Output& output) const override;

        // compile the Vulkan object, context parameter used for Device
        virtual void compile(Context& context);

        // remove the local reference to the Vulkan implementation
        void release(uint32_t deviceID) { _implementation[deviceID] = {}; }
        void release() { _implementation.clear(); }

    protected:
        virtual ~DescriptorSetLayout();

        struct Implementation : public Inherit<Object, Implementation>
        {
            Implementation(Device* device, const DescriptorSetLayoutBindings& descriptorSetLayoutBindings);

            virtual ~Implementation();

            ref_ptr<Device> _device;
            VkDescriptorSetLayout _descriptorSetLayout;
        };

        vk_buffer<ref_ptr<Implementation>> _implementation;
    };
    VSG_type_name(vsg::DescriptorSetLayout);

    using DescriptorSetLayouts = std::vector<vsg::ref_ptr<vsg::DescriptorSetLayout>>;

} // namespace vsg
