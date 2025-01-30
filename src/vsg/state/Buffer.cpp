/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/state/Buffer.h>
#include <vsg/vk/Context.h>

#define REPORT_STATS 1

using namespace vsg;

void Buffer::VulkanData::release()
{
    if (buffer)
    {
        vkDestroyBuffer(*device, buffer, device->getAllocationCallbacks());
    }

    if (deviceMemory)
    {
        //deviceMemory->release(memoryOffset, memorySlots.totalMemorySize());
        deviceMemory->release(memoryOffset, size);
    }
}

Buffer::Buffer(VkDeviceSize in_size, VkBufferUsageFlags in_usage, VkSharingMode in_sharingMode) :
    flags(0),
    size(in_size),
    usage(in_usage),
    sharingMode(in_sharingMode),
    _memorySlots(in_size)
{
}

Buffer::~Buffer()
{
#if REPORT_STATS
    debug("start of Buffer::~Buffer() ", this);
#endif

    for (auto& vd : _vulkanData) vd.release();

#if REPORT_STATS
    debug("end of Buffer::~Buffer() ", this);
#endif
}

VkMemoryRequirements Buffer::getMemoryRequirements(uint32_t deviceID) const
{
    const VulkanData& vd = _vulkanData[deviceID];
    if (!vd.buffer) return {};

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*vd.device, vd.buffer, &memRequirements);
    return memRequirements;
}

VkResult Buffer::bind(DeviceMemory* deviceMemory, VkDeviceSize memoryOffset)
{
    VulkanData& vd = _vulkanData[deviceMemory->getDevice()->deviceID];

    if (vd.deviceMemory)
    {
        warn("Buffer::bind(", deviceMemory, ", ", memoryOffset, ") failed, buffer already bound to ", vd.deviceMemory);
        return VK_ERROR_UNKNOWN;
    }

    VkResult result = vkBindBufferMemory(*vd.device, vd.buffer, *deviceMemory, memoryOffset);
    if (result == VK_SUCCESS)
    {
        vd.deviceMemory = deviceMemory;
        vd.memoryOffset = memoryOffset;
        vd.size = size;
    }

    return result;
}

bool Buffer::compile(Device* device)
{
    VulkanData& vd = _vulkanData[device->deviceID];
    if (vd.buffer)
    {
        return false;
    }

    vd.device = device;
    vd.size = size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.flags = flags;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;

    if (VkResult result = vkCreateBuffer(*device, &bufferInfo, device->getAllocationCallbacks(), &vd.buffer); result != VK_SUCCESS)
    {
        throw Exception{"Error: Failed to create VkBuffer.", result};
    }

    return true;
}

bool Buffer::compile(Context& context)
{
    return compile(context.device);
}

MemorySlots::OptionalOffset Buffer::reserve(VkDeviceSize in_size, VkDeviceSize alignment)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _memorySlots.reserve(in_size, alignment);
}

void Buffer::release(VkDeviceSize offset, VkDeviceSize in_size)
{
    std::scoped_lock<std::mutex> lock(_mutex);
    _memorySlots.release(offset, in_size);
}

bool Buffer::full() const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _memorySlots.full();
}

size_t Buffer::maximumAvailableSpace() const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _memorySlots.maximumAvailableSpace();
}

size_t Buffer::totalAvailableSize() const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _memorySlots.totalAvailableSize();
}

size_t Buffer::totalReservedSize() const
{
    std::scoped_lock<std::mutex> lock(_mutex);
    return _memorySlots.totalReservedSize();
}

ref_ptr<Buffer> vsg::createBufferAndMemory(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryProperties)
{
    auto buffer = vsg::Buffer::create(size, usage, sharingMode);
    buffer->compile(device);

    auto memRequirements = buffer->getMemoryRequirements(device->deviceID);
    auto memory = vsg::DeviceMemory::create(device, memRequirements, memoryProperties);

    buffer->bind(memory, 0);
    return buffer;
}
