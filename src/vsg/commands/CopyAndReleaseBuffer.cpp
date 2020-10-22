/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/commands/CopyAndReleaseBuffer.h>
#include <vsg/io/Options.h>

using namespace vsg;

CopyAndReleaseBuffer::CopyAndReleaseBuffer(BufferInfo src, BufferInfo dest)
{
    add(src, dest);
}

CopyAndReleaseBuffer::~CopyAndReleaseBuffer()
{
   for (auto& copyData : completed) copyData.source.release();
   for (auto& copyData : pending) copyData.source.release();
}

void CopyAndReleaseBuffer::add(BufferInfo src, BufferInfo dest)
{
    pending.push_back(CopyData{src, dest});
}

void CopyAndReleaseBuffer::CopyData::record(CommandBuffer& commandBuffer) const
{
    //std::cout<<"CopyAndReleaseBuffer::CopyData::record(CommandBuffer& commandBuffer) source.offset = "<<source.offset<<", "<<destination.offset<<std::endl;
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = source.offset;
    copyRegion.dstOffset = destination.offset;
    copyRegion.size = source.range;
    vkCmdCopyBuffer(commandBuffer, source.buffer->vk(commandBuffer.deviceID), destination.buffer->vk(commandBuffer.deviceID), 1, &copyRegion);
}

void CopyAndReleaseBuffer::record(CommandBuffer& commandBuffer) const
{
    for (auto& copyData : completed) copyData.source.release();

    completed.clear();

    for (auto& copyData : pending)
    {
        copyData.record(commandBuffer);
    }

    pending.swap(completed);
}
