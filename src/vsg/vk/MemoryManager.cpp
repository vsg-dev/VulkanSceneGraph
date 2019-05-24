/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/vk/Buffer.h>
#include <vsg/vk/Image.h>
#include <vsg/vk/MemoryManager.h>

#include <cstring>

// Useful links:
//    AMD's Vulkan Device Memory discussion : https://gpuopen.com/vulkan-device-memory/
//    NVIDIA's Vulkan Memory Management  : https://developer.nvidia.com/vulkan-memory-management
//
// General comments
//
// Device local resources - use VkMemoryPropertyFlags of VK_DEVICE_LOCAL and not HOST_VISIBLE
//
// If Device local allocations fail fallback to HOST_VISIBLE with HOST_COHERENT but without HOST_CACHED
//
// Allocate high priority resources first i.e. Render Targets, then lower priority objects
//
// On Window resizes be prepared to free all resources, then reallocated high priority ones
//
// CPU->GPU data flow Use DEVICE_LOCAL with HOST_VISIBLE for cases where you want to update Device local memory, ie. uploading constant data, keep allocation sizes below 256MB)
//
// GPU->CPU data flow use HOST_VISIBLE with HOST_COHERENT and HOST_CACHED, only memory type that supports cached reads by the CPU.  Screen captures, compute results etc.
//
// Pool resources as OS Window allocations can be expensive, place buffers and textures in single pools, 256MB is reasonable base
//

using namespace vsg;

MemoryManager::MemoryManager(Device* device, AllocationCallbacks* allocator) :
    _device(device),
    _allocator(allocator)
{
}

MemoryManager::~MemoryManager()
{
}

Buffer* MemoryManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode)
{
    ref_ptr<Buffer> buffer = Buffer::create(_device, size, usage, sharingMode, _allocator);
    return buffer.release();
}

DeviceMemory* MemoryManager::createMemory(const VkMemoryRequirements& memRequirements, VkMemoryPropertyFlags properties)
{
    ref_ptr<DeviceMemory> memory = DeviceMemory::create(_device, memRequirements, properties, _allocator);
    return memory.release();
}
