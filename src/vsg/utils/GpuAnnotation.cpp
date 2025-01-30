/* <editor-fold desc="MIT License">

Copyright(c) 2023 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/utils/GpuAnnotation.h>
#include <vsg/vk/CommandBuffer.h>

using namespace vsg;

GpuAnnotation::GpuAnnotation()
{
}

GpuAnnotation::~GpuAnnotation()
{
}

void GpuAnnotation::enterCommandBuffer(const SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer) const
{
    enter(sl, reference, commandBuffer, nullptr);
}

void GpuAnnotation::leaveCommandBuffer(const SourceLocation* sl, uint64_t& reference, CommandBuffer& commandBuffer) const
{
    leave(sl, reference, commandBuffer, nullptr);
}

void GpuAnnotation::enter(const SourceLocation* sl, uint64_t&, vsg::CommandBuffer& commandBuffer, const Object* object) const
{
    auto extensions = commandBuffer.getDevice()->getInstance()->getExtensions();

    VkDebugUtilsLabelEXT markerInfo = {};
    markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    if (sl->name && labelType == SourceLocation_name)
        markerInfo.pLabelName = sl->name;
    else if (labelType == Object_className && object)
        markerInfo.pLabelName = object->className();
    else
        markerInfo.pLabelName = sl->function;

    markerInfo.color[0] = static_cast<float>(sl->color.r) / 255.0f;
    markerInfo.color[1] = static_cast<float>(sl->color.g) / 255.0f;
    markerInfo.color[2] = static_cast<float>(sl->color.b) / 255.0f;
    markerInfo.color[3] = static_cast<float>(sl->color.a) / 255.0f;

    extensions->vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &markerInfo);
}

void GpuAnnotation::leave(const SourceLocation*, uint64_t&, vsg::CommandBuffer& commandBuffer, const Object*) const
{
    auto extensions = commandBuffer.getDevice()->getInstance()->getExtensions();
    extensions->vkCmdEndDebugUtilsLabelEXT(commandBuffer);
}
