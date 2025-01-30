/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/vk/Instance.h>
#include <vsg/vk/InstanceExtensions.h>
#include <vsg/vk/PhysicalDevice.h>

#include <algorithm>
#include <cstring>
#include <set>

using namespace vsg;

InstanceExtensions::InstanceExtensions(const Instance* instance)
{
    // VK_EXT_debug_utils
    instance->getProcAddr(vkSetDebugUtilsObjectNameEXT, "vkSetDebugUtilsObjectNameEXT");
    instance->getProcAddr(vkSetDebugUtilsObjectTagEXT, "vkSetDebugUtilsObjectTagEXT");
    instance->getProcAddr(vkQueueBeginDebugUtilsLabelEXT, "vkQueueBeginDebugUtilsLabelEXT");
    instance->getProcAddr(vkQueueEndDebugUtilsLabelEXT, "vkQueueEndDebugUtilsLabelEXT");
    instance->getProcAddr(vkQueueInsertDebugUtilsLabelEXT, "vkQueueInsertDebugUtilsLabelEXT");
    instance->getProcAddr(vkCmdBeginDebugUtilsLabelEXT, "vkCmdBeginDebugUtilsLabelEXT");
    instance->getProcAddr(vkCmdEndDebugUtilsLabelEXT, "vkCmdEndDebugUtilsLabelEXT");
    instance->getProcAddr(vkCmdInsertDebugUtilsLabelEXT, "vkCmdInsertDebugUtilsLabelEXT");
    instance->getProcAddr(vkCreateDebugUtilsMessengerEXT, "vkCreateDebugUtilsMessengerEXT");
    instance->getProcAddr(vkDestroyDebugUtilsMessengerEXT, "vkDestroyDebugUtilsMessengerEXT");
    instance->getProcAddr(vkSubmitDebugUtilsMessageEXT, "vkSubmitDebugUtilsMessageEXT");

    // VK_EXT_calibrated_timestamps
    instance->getProcAddr(vkGetPhysicalDeviceCalibrateableTimeDomainsEXT, "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR", "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
    instance->getProcAddr(vkGetCalibratedTimestampsEXT, "vkGetCalibratedTimestampsKHR", "vkGetCalibratedTimestampsEXT");
}
