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
#include <vsg/vk/BufferData.h>
#include <vsg/vk/ImageData.h>

namespace vsg
{
    struct BufferPreferences
    {
        VkDeviceSize minimumBufferSize = 16 * 1024 * 1024;
        VkDeviceSize minimumBufferDeviceMemorySize = 16 * 1024 * 1024;
        VkDeviceSize minimumImageDeviceMemorySize = 16 * 1024 * 1024;
    };

    class VSG_DECLSPEC MemoryBufferPools : public Inherit<Object, MemoryBufferPools>
    {
    public:
        MemoryBufferPools(const std::string& name, Device* in_device, BufferPreferences preferences);

        std::string name;
        ref_ptr<Device> device;
        BufferPreferences bufferPreferences;

        // transfer data settings
        // used by BufferData.cpp, ImageData.cpp
        using MemoryPools = std::vector<ref_ptr<DeviceMemory>>;
        MemoryPools memoryPools;

        using BufferPools = std::vector<ref_ptr<Buffer>>;
        BufferPools bufferPools;

        VkDeviceSize computeMemoryTotalAvailable() const;
        VkDeviceSize computeMemoryTotalReserved() const;
        VkDeviceSize computeBufferTotalAvailable() const;
        VkDeviceSize computeBufferTotalReserved() const;

        BufferData reserveBufferData(VkDeviceSize totalSize, VkDeviceSize alignment, VkBufferUsageFlags bufferUsageFlags, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryProperties);

        using DeviceMemoryOffset = std::pair<ref_ptr<DeviceMemory>, VkDeviceSize>;
        DeviceMemoryOffset reserveMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memoryProperties, void* pNextAllocInfo = nullptr);

        using CopyPair = std::pair<BufferData, BufferData>;
        using CopyQueue = std::deque<CopyPair>;

        CopyQueue bufferDataToCopy;
    };

} // namespace vsg
