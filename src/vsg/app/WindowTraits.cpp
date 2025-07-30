/* <editor-fold desc="MIT License">

Copyright(c) 2022 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/app/WindowTraits.h>
#include <vsg/io/Logger.h>
#include <vsg/utils/CommandLine.h>
#include <vsg/vk/vulkan.h>

#include <iostream>

using namespace vsg;

WindowTraits::WindowTraits()
{
    defaults();
}

WindowTraits::WindowTraits(CommandLine& arguments)
{
    defaults();

    if (arguments.read("--args")) std::cout << arguments << std::endl;

    windowTitle = vsg::make_string(arguments);
    debugLayer = arguments.read({"--debug", "-d"});
    apiDumpLayer = arguments.read({"--api", "-a"});
    synchronizationLayer = arguments.read("--sync");

    if (arguments.read("--double-buffer")) swapchainPreferences.imageCount = 2;
    if (arguments.read("--triple-buffer")) swapchainPreferences.imageCount = 3; // default
    if (arguments.read("--IMMEDIATE")) { swapchainPreferences.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; }
    if (arguments.read("--FIFO")) swapchainPreferences.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (arguments.read("--FIFO_RELAXED")) swapchainPreferences.presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    if (arguments.read("--MAILBOX")) swapchainPreferences.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

    if (arguments.read({"--fullscreen", "--fs"})) fullscreen = true;
    if (arguments.read({"--window", "-w"}, width, height)) { fullscreen = false; }
    if (arguments.read({"--no-frame"})) decoration = false;
    if (arguments.read("--or")) overrideRedirect = true;

    if (arguments.read("--d32")) depthFormat = VK_FORMAT_D32_SFLOAT;
    if (arguments.read("--sRGB")) swapchainPreferences.surfaceFormat = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    if (arguments.read("--RGB")) swapchainPreferences.surfaceFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    arguments.read("--screen", screenNum);
    arguments.read("--display", display);
    arguments.read("--samples", samples);
}

WindowTraits::WindowTraits(const WindowTraits& traits, const CopyOp& copyop) :
    Inherit(traits, copyop),
    x(traits.x),
    y(traits.y),
    width(traits.width),
    height(traits.height),
    fullscreen(traits.fullscreen),
    display(traits.display),
    screenNum(traits.screenNum),
    windowClass(traits.windowClass),
    windowTitle(traits.windowTitle),
    decoration(traits.decoration),
    hdpi(traits.hdpi),
    overrideRedirect(traits.overrideRedirect),
    vulkanVersion(traits.vulkanVersion),
    swapchainPreferences(traits.swapchainPreferences),
    depthFormat(traits.depthFormat),
    depthImageUsage(traits.depthImageUsage),
    queueFlags(traits.queueFlags),
    queuePiorities(traits.queuePiorities),
    imageAvailableSemaphoreWaitFlag(traits.imageAvailableSemaphoreWaitFlag),
    debugLayer(traits.debugLayer),
    synchronizationLayer(traits.synchronizationLayer),
    apiDumpLayer(traits.apiDumpLayer),
    debugUtils(traits.debugUtils),
    device(traits.device),
    instanceExtensionNames(traits.instanceExtensionNames),
    requestedLayers(traits.requestedLayers),
    deviceExtensionNames(traits.deviceExtensionNames),
    deviceTypePreferences(traits.deviceTypePreferences),
    deviceFeatures(traits.deviceFeatures),
    samples(traits.samples) /*,
    nativeWindow(traits.nativeWindow),
    systemConnection(traits.systemConnection)*/
{
}

WindowTraits::WindowTraits(const std::string& title) :
    windowTitle(title)
{
    defaults();
}

WindowTraits::WindowTraits(int32_t in_x, int32_t in_y, uint32_t in_width, uint32_t in_height, const std::string& title) :
    x(in_x),
    y(in_y),
    width(in_width),
    height(in_height),
    windowTitle(title)
{
    defaults();
}

WindowTraits::WindowTraits(uint32_t in_width, uint32_t in_height, const std::string& title) :
    width(in_width),
    height(in_height),
    windowTitle(title)
{
    defaults();
}

void WindowTraits::defaults()
{
#if !defined(__ANDROID__)
    // query the vulkan instance version available
    vkEnumerateInstanceVersion(&vulkanVersion);
#endif

    // vsg::DeviceFeatures uses the instance extension
    instanceExtensionNames.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    // provide anisotropic filtering as standard.
    if (!deviceFeatures) deviceFeatures = vsg::DeviceFeatures::create();
    deviceFeatures->get().samplerAnisotropy = VK_TRUE;

    // prefer discrete gpu over integrated gpu over virtual gpu
    deviceTypePreferences = {VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU};
}

void WindowTraits::validate()
{
    if (debugLayer || apiDumpLayer || synchronizationLayer)
    {
        instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    if (debugUtils && isExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        instanceExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    if (apiDumpLayer) requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");
    if (debugLayer) requestedLayers.push_back("VK_LAYER_KHRONOS_validation");
    if (synchronizationLayer) requestedLayers.push_back("VK_LAYER_KHRONOS_synchronization2");

    requestedLayers = vsg::validateInstancelayerNames(requestedLayers);
}
