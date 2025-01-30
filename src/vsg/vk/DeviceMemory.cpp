/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/vk/DeviceMemory.h>

#include <atomic>
#include <cstring>

using namespace vsg;

#define DO_CHECK 0

static std::mutex s_DeviceMemoryListMutex;
static std::list<vsg::observer_ptr<DeviceMemory>> s_DeviceMemoryList;

DeviceMemoryList vsg::getActiveDeviceMemoryList(VkMemoryPropertyFlagBits propertyFlags)
{
    std::scoped_lock<std::mutex> lock(s_DeviceMemoryListMutex);
    DeviceMemoryList dml;
    for (auto& dm : s_DeviceMemoryList)
    {
        auto dm_ref_ptr = dm.ref_ptr();
        if ((dm_ref_ptr->getMemoryPropertyFlags() & propertyFlags) != 0)
        {
            dml.push_back(dm_ref_ptr);
        }
    }
    return dml;
}

///////////////////////////////////////////////////////////////////////////////
//
// DeviceMemory
//
DeviceMemory::DeviceMemory(Device* device, const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties, void* pNextAllocInfo) :
    _memoryRequirements(memRequirements),
    _properties(properties),
    _device(device),
    _memorySlots(memRequirements.size)
{
    uint32_t typeFilter = memRequirements.memoryTypeBits;

    // find the memory type to use
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(*(device->getPhysicalDevice()), &memProperties);
    uint32_t i;
    for (i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) break;
    }
    if (i >= memProperties.memoryTypeCount)
    {
        throw Exception{"Error: vsg::DeviceMemory::create(...) failed to create DeviceMemory, no usable memory type found.", VK_ERROR_FORMAT_NOT_SUPPORTED};
    }
    uint32_t memoryTypeIndex = i;

#if DO_CHECK
    if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    {
        static VkDeviceSize s_TotalDeviceMemoryAllocated = 0;
        s_TotalDeviceMemoryAllocated += memRequirements.size;
        debug("Device Local DeviceMemory::DeviceMemory() ", std::dec, memRequirements.size, ", ", memRequirements.alignment, ", ", memRequirements.memoryTypeBits, ",  s_TotalMemoryAllocated = ", s_TotalDeviceMemoryAllocated);
    }
    else
    {
        static VkDeviceSize s_TotalHostMemoryAllocated = 0;
        s_TotalHostMemoryAllocated += memRequirements.size;
        debug("Staging DeviceMemory::DeviceMemory()  ", std::dec, memRequirements.size, ", ", memRequirements.alignment, ", ", memRequirements.memoryTypeBits, ",  s_TotalMemoryAllocated = ", s_TotalHostMemoryAllocated);
    }
#endif

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memRequirements.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;
    allocateInfo.pNext = pNextAllocInfo;

    if (VkResult result = vkAllocateMemory(*device, &allocateInfo, _device->getAllocationCallbacks(), &_deviceMemory); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to allocate DeviceMemory.", result};
    }

    {
        std::scoped_lock<std::mutex> lock(s_DeviceMemoryListMutex);
        s_DeviceMemoryList.emplace_back(this);
        vsg::debug("DeviceMemory::DeviceMemory() added to s_DeviceMemoryList, s_DeviceMemoryList.size() = ", s_DeviceMemoryList.size());
    }
}

DeviceMemory::~DeviceMemory()
{
    if (_deviceMemory)
    {
#if DO_CHECK
        debug("DeviceMemory::~DeviceMemory() vkFreeMemory(*_device, ", _deviceMemory, ", _allocator);");
#endif

        vkFreeMemory(*_device, _deviceMemory, _device->getAllocationCallbacks());
    }

    {
        std::scoped_lock<std::mutex> lock(s_DeviceMemoryListMutex);
        auto itr = std::find(s_DeviceMemoryList.begin(), s_DeviceMemoryList.end(), this);
        if (itr != s_DeviceMemoryList.end())
        {
            s_DeviceMemoryList.erase(itr);
            vsg::debug("DeviceMemory::~DeviceMemory() removed from s_DeviceMemoryList, s_DeviceMemoryList.size() = ", s_DeviceMemoryList.size());
        }
        else
        {
            vsg::warn("DeviceMemory::~DeviceMemory() could not find in  s_DeviceMemoryList");
        }
    }
}

VkResult DeviceMemory::map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData)
{
    return vkMapMemory(*_device, _deviceMemory, offset, size, flags, ppData);
}

void DeviceMemory::unmap()
{
    vkUnmapMemory(*_device, _deviceMemory);
}

void DeviceMemory::copy(VkDeviceSize offset, VkDeviceSize size, const void* src_data)
{
    // should we have checks against buffer having enough memory for copied data?

    void* buffer_data;
    map(offset, size, 0, &buffer_data);

    std::memcpy(buffer_data, src_data, (size_t)size);

    unmap();
}

void DeviceMemory::copy(VkDeviceSize offset, const Data* data)
{
    copy(offset, data->dataSize(), data->dataPointer());
}

MemorySlots::OptionalOffset DeviceMemory::reserve(VkDeviceSize size)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _memorySlots.reserve(size, _memoryRequirements.alignment);
}

void DeviceMemory::release(VkDeviceSize offset, VkDeviceSize size)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    _memorySlots.release(offset, size);
}

bool DeviceMemory::full() const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _memorySlots.full();
}

VkDeviceSize DeviceMemory::maximumAvailableSpace() const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _memorySlots.maximumAvailableSpace();
}

size_t DeviceMemory::totalAvailableSize() const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _memorySlots.totalAvailableSize();
}

size_t DeviceMemory::totalReservedSize() const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _memorySlots.totalReservedSize();
}

size_t DeviceMemory::totalMemorySize() const
{
    return _memorySlots.totalMemorySize();
}
