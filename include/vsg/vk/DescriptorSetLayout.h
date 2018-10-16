#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Device.h>

namespace vsg
{
    using DescriptorSetLayoutBindings = std::vector<VkDescriptorSetLayoutBinding>;

    class VSG_DECLSPEC DescriptorSetLayout : public Inherit<Object, DescriptorSetLayout>
    {
    public:
        DescriptorSetLayout(Device* device, VkDescriptorSetLayout DescriptorSetLayout, AllocationCallbacks* allocator=nullptr);

        using Result = vsg::Result<DescriptorSetLayout, VkResult, VK_SUCCESS>;

        static Result create(Device* device, const DescriptorSetLayoutBindings& descriptorSetLayoutBindings, AllocationCallbacks* allocator=nullptr);

        operator const VkDescriptorSetLayout& () const { return _descriptorSetLayout; }

    protected:
        virtual ~DescriptorSetLayout();

        ref_ptr<Device>                 _device;
        VkDescriptorSetLayout           _descriptorSetLayout;
        ref_ptr<AllocationCallbacks>    _allocator;
    };

}
