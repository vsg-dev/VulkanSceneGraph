#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <deque>
#include <memory>

#include <vsg/core/Object.h>
#include <vsg/state/BufferInfo.h>
#include <vsg/vk/ResourceRequirements.h>

namespace vsg
{

    /// MemoryBufferPools manages a pool of vsg::DeviceMemory and vsg::Buffer that use them.
    /// Methods are provided for getting Buffer from the pool, sharing memory to make better use of device memory.
    class VSG_DECLSPEC MemoryBufferPools : public Inherit<Object, MemoryBufferPools>
    {
    public:
        MemoryBufferPools(const std::string& name, ref_ptr<Device> in_device, const ResourceRequirements& in_resourceRequirements = {});

        std::string name;
        ref_ptr<Device> device;
        VkDeviceSize minimumBufferSize = 16 * 1024 * 1024;
        VkDeviceSize minimumDeviceMemorySize = 16 * 1024 * 1024;

        /// Ratio of available device memory that can be allocated.
        /// Ratios less than 1.0 require VK_EXT_memory_budget extension to be supported.
        /// Ratio of 1.0 (or greater) will switch off checks for available memory and keep allocating till Vulkan memory allocations fail.
        double allocatedMemoryLimit = 1.0;

        /// throw vsg::Exception when reserveMemory() fails to allocated memory on device.
        bool throwOutOfDeviceMemoryException = true;

        VkDeviceSize computeMemoryTotalAvailable() const;
        VkDeviceSize computeMemoryTotalReserved() const;
        VkDeviceSize computeBufferTotalAvailable() const;
        VkDeviceSize computeBufferTotalReserved() const;

        ref_ptr<BufferInfo> reserveBuffer(VkDeviceSize totalSize, VkDeviceSize alignment, VkBufferUsageFlags bufferUsageFlags, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryProperties);

        using DeviceMemoryOffset = std::pair<ref_ptr<DeviceMemory>, VkDeviceSize>;
        DeviceMemoryOffset reserveMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memoryProperties, void* pNextAllocInfo = nullptr);

        VkResult reserve(ResourceRequirements& requirements);

    protected:
        mutable std::mutex _mutex;

        // transfer data settings
        using MemoryPools = std::vector<ref_ptr<DeviceMemory>>;
        MemoryPools memoryPools;

        using BufferPools = std::vector<ref_ptr<Buffer>>;
        BufferPools bufferPools;
    };
    VSG_type_name(vsg::MemoryBufferPools);

} // namespace vsg
