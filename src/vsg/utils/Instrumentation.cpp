/* <editor-fold desc="MIT License">

Copyright(c) 2023 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Options.h>
#include <vsg/utils/Instrumentation.h>
#include <vsg/vk/CommandBuffer.h>
#include <vsg/vk/Queue.h>

using namespace vsg;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Instrumentation base class
//
Instrumentation::Instrumentation()
{
}

void Instrumentation::init(vsg::ref_ptr<Device>, vsg::ref_ptr<Queue>, vsg::ref_ptr<CommandBuffer>)
{
}

Instrumentation::~Instrumentation()
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// VulkanAnnotation uses VK_debug_utils to pass annotation to Vulkan
//
VulkanAnnotation::VulkanAnnotation()
{
}

VulkanAnnotation::~VulkanAnnotation()
{
}

void VulkanAnnotation::enter(const SourceLocation* sl, uint64_t&) const
{
    if (!commandBuffer) return;

    auto extensions = commandBuffer->getDevice()->getInstance()->getExtensions();

    VkDebugUtilsLabelEXT markerInfo = {};
    markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
    if (sl->name)
        markerInfo.pLabelName = sl->name;
    else
        markerInfo.pLabelName = sl->function;
    markerInfo.color[0] = static_cast<float>(sl->color[0]) / 255.0;
    markerInfo.color[1] = static_cast<float>(sl->color[1]) / 255.0;
    markerInfo.color[2] = static_cast<float>(sl->color[2]) / 255.0;
    markerInfo.color[3] = static_cast<float>(sl->color[3]) / 255.0;

    extensions->vkCmdBeginDebugUtilsLabelEXT(*commandBuffer, &markerInfo);
}

void VulkanAnnotation::leave(const SourceLocation*, uint64_t&) const
{
    if (!commandBuffer) return;

    auto extensions = commandBuffer->getDevice()->getInstance()->getExtensions();
    extensions->vkCmdEndDebugUtilsLabelEXT(*commandBuffer);
}
