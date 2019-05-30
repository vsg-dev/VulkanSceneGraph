/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/traversals/CompileTraversal.h>

#include <vsg/nodes/Commands.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/LOD.h>
#include <vsg/nodes/QuadGroup.h>
#include <vsg/nodes/StateGroup.h>

#include <vsg/vk/Command.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/RenderPass.h>
#include <vsg/vk/State.h>

#include <iostream>

using namespace vsg;

MemoryBufferPools::MemoryBufferPools(const std::string& in_name, Device* in_device, BufferPreferences preferences):
    name(in_name),
    device(in_device),
    bufferPreferences(preferences)
{
}


BufferData MemoryBufferPools::reserveBufferData(VkDeviceSize totalSize, VkDeviceSize alignment, VkBufferUsageFlags bufferUsageFlags, VkSharingMode sharingMode, VkMemoryPropertyFlags memoryProperties)
{
    BufferData bufferData;
    for (auto& bufferFromPool : bufferPools)
    {
        if (!bufferFromPool->full() && bufferFromPool->usage() == bufferUsageFlags)
        {
            Buffer::OptionalBufferOffset reservedBufferSlot = bufferFromPool->reserve(totalSize, alignment);
            if (reservedBufferSlot.first)
            {
                bufferData._buffer = bufferFromPool;
                bufferData._offset = reservedBufferSlot.second;
                bufferData._range = totalSize;

                std::cout<<name<<" : MemoryBufferPools::reserveBufferData("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") _offset = "<<bufferData._offset<<std::endl;

                return bufferData;
            }
        }
    }
    std::cout<<name<<" : Failed to space in existing buffers with  MemoryBufferPools::reserveBufferData("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") bufferPools.size() = "<<bufferPools.size()<<" looking to allocated new Buffer."<<std::endl;

    VkDeviceSize deviceSize = totalSize;

    VkDeviceSize minumumBufferSize = bufferPreferences.minimumBufferSize;
    if (deviceSize < minumumBufferSize)
    {
        deviceSize = minumumBufferSize;
    }

    bufferData._buffer = vsg::Buffer::create(device, deviceSize, bufferUsageFlags, sharingMode);

    Buffer::OptionalBufferOffset reservedBufferSlot = bufferData._buffer->reserve(totalSize, alignment);
    bufferData._offset = reservedBufferSlot.second;
    bufferData._range = totalSize;


    std::cout<<name<<" : Created new Buffer "<<bufferData._buffer.get()<<" totalSize "<<totalSize<<" deviceSize = "<<deviceSize<<std::endl;

    if (!bufferData._buffer->full())
    {
        std::cout<<name<<"  inserting new Buffer into Context.bufferPools"<<std::endl;
        bufferPools.push_back(bufferData._buffer);
    }

    std::cout<<name<<" : bufferData._offset = "<<bufferData._offset<<std::endl;

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, *bufferData._buffer, &memRequirements);

    ref_ptr<DeviceMemory> deviceMemory;
    DeviceMemory::OptionalMemoryOffset reservedMemorySlot(false, 0);

    for (auto& memoryFromPool : memoryPools)
    {
        if (!memoryFromPool->full() && memoryFromPool->getMemoryRequirements().memoryTypeBits == memRequirements.memoryTypeBits)
        {
            reservedMemorySlot = memoryFromPool->reserve(deviceSize);
            if (reservedMemorySlot.first)
            {
                deviceMemory = memoryFromPool;
                break;
            }
        }
    }

    if (!deviceMemory)
    {
        VkDeviceSize minumumDeviceMemorySize = bufferPreferences.minimumBufferDeviceMemorySize;

        // clamp to an aligned size
        minumumDeviceMemorySize = ((minumumDeviceMemorySize + memRequirements.alignment - 1) / memRequirements.alignment) * memRequirements.alignment;

        if (memRequirements.size < minumumDeviceMemorySize) memRequirements.size = minumumDeviceMemorySize;

        deviceMemory = vsg::DeviceMemory::create(device, memRequirements, memoryProperties); // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        if (deviceMemory)
        {
            reservedMemorySlot = deviceMemory->reserve(deviceSize);
            if (!deviceMemory->full())
            {
                memoryPools.push_back(deviceMemory);
            }
        }
    }
    else
    {
        if (deviceMemory->full())
        {
            std::cout<<name<<" : DeviceMemory is full "<<deviceMemory.get()<<std::endl;
        }
    }

    if (!reservedMemorySlot.first)
    {
        std::cout<<name<<" : Completely Failed to space for MemoryBufferPools::reserveBufferData("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") "<<std::endl;
        return BufferData();
    }

    std::cout<<name<<" : Allocated new buffer, MemoryBufferPools::reserveBufferData("<<totalSize<<", "<<alignment<<", "<<bufferUsageFlags<<") "<<std::endl;
    bufferData._buffer->bind(deviceMemory, reservedMemorySlot.second);

    return bufferData;
}

MemoryBufferPools::DeviceMemoryOffset MemoryBufferPools::reserveMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags memoryProperties)
{
    VkDeviceSize totalSize = memRequirements.size;

    ref_ptr<DeviceMemory> deviceMemory;
    DeviceMemory::OptionalMemoryOffset reservedSlot(false, 0);

    for (auto& memoryPool : memoryPools)
    {
        if (!memoryPool->full() && memoryPool->getMemoryRequirements().memoryTypeBits == memRequirements.memoryTypeBits)
        {
            reservedSlot = memoryPool->reserve(totalSize);
            if (reservedSlot.first)
            {
                deviceMemory = memoryPool;
                break;
            }
        }
    }

    if (!deviceMemory)
    {
        VkDeviceSize minumumDeviceMemorySize = bufferPreferences.minimumImageDeviceMemorySize;

        // clamp to an aligned size
        minumumDeviceMemorySize = ((minumumDeviceMemorySize + memRequirements.alignment - 1) / memRequirements.alignment) * memRequirements.alignment;

        //std::cout<<"Creating new local DeviceMemory"<<std::endl;
        if (memRequirements.size < minumumDeviceMemorySize) memRequirements.size = minumumDeviceMemorySize;

        deviceMemory = vsg::DeviceMemory::create(device, memRequirements, memoryProperties);
        if (deviceMemory)
        {
            reservedSlot = deviceMemory->reserve(totalSize);
            if (!deviceMemory->full())
            {
                //std::cout<<"  inserting DeviceMemory into memoryPool "<<deviceMemory.get()<<std::endl;
                memoryPools.push_back(deviceMemory);
            }
        }
    }
    else
    {
        if (deviceMemory->full())
        {
            //std::cout<<"DeviceMemory is full "<<deviceMemory.get()<<std::endl;
        }
    }

    if (!reservedSlot.first)
    {
        std::cout << "Failed to reserve slot" << std::endl;
        return DeviceMemoryOffset();
    }

    //std::cout << "MemoryBufferPools::reserveMemory() allocated memory at " << reservedSlot.second << std::endl;
    return MemoryBufferPools::DeviceMemoryOffset(deviceMemory, reservedSlot.second);
}


Context::Context(Device* in_device,  BufferPreferences bufferPreferences):
    device(in_device),
    deviceMemoryBufferPools("Device_MemoryBufferPool", device, bufferPreferences),
    stagingMemoryBufferPools("Staging_MemoryBufferPool", device, bufferPreferences)
{
}

CompileTraversal::CompileTraversal(Device* in_device,  BufferPreferences bufferPreferences):
    context(in_device, bufferPreferences)
{
}

CompileTraversal::~CompileTraversal()
{
}

void CompileTraversal::apply(Object& object)
{
    object.traverse(*this);
}

void CompileTraversal::apply(Command& command)
{
    command.compile(context);
}

void CompileTraversal::apply(Commands& commands)
{
    commands.compile(context);
}

void CompileTraversal::apply(StateGroup& stateGroup)
{
    stateGroup.compile(context);
    stateGroup.traverse(*this);
}

void CompileTraversal::apply(Geometry& geometry)
{
    geometry.compile(context);
    geometry.traverse(*this);
}
