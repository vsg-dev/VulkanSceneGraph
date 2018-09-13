/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/DescriptorPool.h>

namespace vsg
{

DescriptorPool::DescriptorPool(VkDescriptorPool descriptorPool, Device* device, AllocationCallbacks* allocator) :
    _descriptorPool(descriptorPool),
    _device(device),
    _allocator(allocator)
{
}

DescriptorPool::~DescriptorPool()
{
    if (_descriptorPool)
    {
        vkDestroyDescriptorPool(*_device, _descriptorPool, _allocator);
    }
}

DescriptorPool::Result DescriptorPool::create(Device* device, uint32_t maxSets, const DescriptorPoolSizes& descriptorPoolSizes, AllocationCallbacks* allocator)
{
    if (!device)
    {
        return Result("Error: vsg::DescriptorPool::create(...) failed to create DescriptorPool, undefined Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
    }

    if (descriptorPoolSizes.empty())
    {
        return Result("Error: vsg::DescriptorPool::create(...) failed to create DescriptorPool, descriptorPoolSizes.empty().", VK_ERROR_INITIALIZATION_FAILED);
    }

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = descriptorPoolSizes.size();
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = maxSets;
    poolInfo.flags =VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; // will we need VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT later?

    VkDescriptorPool descriptorPool;
    VkResult result = vkCreateDescriptorPool(*device, &poolInfo, allocator, &descriptorPool);
    if (result == VK_SUCCESS)
    {
        return new DescriptorPool(descriptorPool, device, allocator);
    }
    else
    {
        return Result("Error: Failed to create DescriptorPool.", result);
    }
}

}
