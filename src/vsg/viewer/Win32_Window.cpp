/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include "Win32_Window.h"

#include <vsg/core/observer_ptr.h>
#include <vsg/vk/Extensions.h>

#include <iostream>

using namespace vsg;
using namespace vsgWin32;

namespace vsgWin32
{
    const std::string kWindowClassName = "vsg_Win32_Window_Class";

    std::unordered_map<HWND, Win32_Window*> Win32_Window::s_registeredWindows;

    vsg::Names vsgWin32::getInstanceExtensions()
    {
        // check the extensions are avaliable first
        Names requiredExtensions = {"VK_KHR_surface", "VK_KHR_win32_surface"};

        if (!vsg::isExtensionListSupported(requiredExtensions))
        {
            std::cout << "Error: vsg::getInstanceExtensions(...) unable to create window, VK_KHR_surface or VK_KHR_win32_surface not supported." << std::endl;
            return Names();
        }

        return requiredExtensions;
    }

    class Win32Surface : public vsg::Surface
    {
    public:
        Win32Surface(vsg::Instance* instance, HWND window, vsg::AllocationCallbacks* allocator = nullptr) :
            vsg::Surface(VK_NULL_HANDLE, instance, allocator)
        {
            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.hinstance = ::GetModuleHandle(NULL);
            surfaceCreateInfo.hwnd = window;

            auto result = vkCreateWin32SurfaceKHR(*instance, &surfaceCreateInfo, nullptr, &_surface);
        }
    };

    // our windows events callback
    LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Win32_Window* win = Win32_Window::getWindow(hwnd);
        if (win != nullptr) return win->handleWin32Messages(msg, wParam, lParam);
        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

} // namespace vsg

Win32_Window::Win32_Window(HWND window, vsg::Instance* instance, vsg::Surface* surface, vsg::PhysicalDevice* physicalDevice, vsg::Device* device, vsg::RenderPass* renderPass, bool debugLayersEnabled) :
    _window(window)
{
    _instance = instance;
    _surface = surface;
    _physicalDevice = physicalDevice;
    _device = device;
    _renderPass = renderPass;
    _debugLayersEnabled = debugLayersEnabled;

    registerWindow(_window, this);
}

Win32_Window::Result Win32_Window::create(const Traits& traits, bool debugLayer, bool apiDumpLayer, vsg::AllocationCallbacks* allocator)
{
    std::cout << "Calling CreateWindowEx(..)" << std::endl;

    HWND hwnd;

    // register window class
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = Win32WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ::GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = kWindowClassName.c_str();
    wc.hIconSm = 0;

    if (::RegisterClassEx(&wc) == 0)
    {
        auto lastError = ::GetLastError();
        if (lastError != ERROR_CLASS_ALREADY_EXISTS) return Result("Error: vsg::Win32_Window::create(...) failed to create Window, could not register window class.", VK_ERROR_INITIALIZATION_FAILED);
    }

    // fetch screen display information

    std::vector<DISPLAY_DEVICE> displayDevices;
    for (unsigned int deviceNum = 0;; ++deviceNum)
    {
        DISPLAY_DEVICE displayDevice;
        displayDevice.cb = sizeof(displayDevice);

        if (!::EnumDisplayDevices(NULL, deviceNum, &displayDevice, 0)) break;
        if (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) continue;
        if (!(displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) continue;

        displayDevices.push_back(displayDevice);
    }

    if (traits.screenNum >= displayDevices.size()) return Result("Error: vsg::Win32_Window::create(...) failed to create Window, screenNum is out of range.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

    DEVMODE deviceMode;
    deviceMode.dmSize = sizeof(deviceMode);
    deviceMode.dmDriverExtra = 0;

    if (!::EnumDisplaySettings(displayDevices[traits.screenNum].DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode)) return Result("Error: vsg::Win32_Window::create(...) failed to create Window, EnumDisplaySettings failed to fetch display settings.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

    // setup window rect and style
    RECT windowRect;
    windowRect.left = deviceMode.dmPosition.x + traits.x;
    windowRect.top = deviceMode.dmPosition.y + traits.y;
    windowRect.right = windowRect.left + traits.width;
    windowRect.bottom = windowRect.top + traits.height;

    unsigned int windowStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | (traits.decoration ? WS_CAPTION : 0);
    unsigned int extendedStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

    if (!::AdjustWindowRectEx(&windowRect, windowStyle, FALSE, extendedStyle)) return Result("Error: vsg::Win32_Window::create(...) failed to create Window, AdjustWindowRectEx failed.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

    // create the window
    hwnd = ::CreateWindowEx(extendedStyle, kWindowClassName.c_str(), traits.title.c_str(), windowStyle,
                            windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
                            NULL, NULL, ::GetModuleHandle(NULL), NULL);

    if (hwnd == nullptr) return Result("Error: vsg::Win32_Window::create(...) failed to create Window, CreateWindowEx did not return a valid window handle.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

    GetClientRect(hwnd, &windowRect);

    uint32_t finalWidth = windowRect.right - windowRect.left;
    uint32_t finalHeight = windowRect.bottom - windowRect.top;

    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    vsg::ref_ptr<Win32_Window> window;

    if (traits.shareWindow)
    {
        // create surface
        vsg::ref_ptr<vsg::Surface> surface(new vsg::Win32Surface(traits.shareWindow->instance(), hwnd, allocator));

        window = new Win32_Window(hwnd, traits.shareWindow->instance(), traits.shareWindow->surface(), traits.shareWindow->physicalDevice(), traits.shareWindow->device(), traits.shareWindow->renderPass(), traits.shareWindow->debugLayersEnabled());

        // share the _instance, _physicalDevice and _device;
        window->share(*traits.shareWindow);

        // temporary hack to force vkGetPhysicalDeviceSurfaceSupportKHR to be called as the Vulkan
        // debug layer is complaining about vkGetPhysicalDeviceSurfaceSupportKHR not being called
        // for this _surface prior to swap chain creation
        vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(traits.shareWindow->instance(), VK_QUEUE_GRAPHICS_BIT, surface);
    }
    else
    {
        vsg::Names instanceExtensions = vsgWin32::getInstanceExtensions();

        vsg::Names requestedLayers;
        if (debugLayer)
        {
            instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            requestedLayers.push_back("VK_LAYER_LUNARG_standard_validation");
            if (apiDumpLayer) requestedLayers.push_back("VK_LAYER_LUNARG_api_dump");
        }

        vsg::Names validatedNames = vsg::validateInstancelayerNames(requestedLayers);

        vsg::Names deviceExtensions;
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        vsg::ref_ptr<vsg::Instance> instance = vsg::Instance::create(instanceExtensions, validatedNames, allocator);
        if (!instance) return Result("Error: vsg::Win32_Window::create(...) failed to create Window, unable to create Vulkan instance.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        // use GLFW to create surface
        vsg::ref_ptr<vsg::Surface> surface(new vsg::Win32Surface(instance, hwnd, allocator));
        if (!surface) return Result("Error: vsg::Win32_Window::create(...) failed to create Window, unable to create Win32Surface.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        // set up device
        vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(instance, VK_QUEUE_GRAPHICS_BIT, surface);
        if (!physicalDevice) return Result("Error: vsg::Win32_Window::create(...) failed to create Window, no Vulkan PhysicalDevice supported.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        vsg::ref_ptr<vsg::Device> device = vsg::Device::create(physicalDevice, validatedNames, deviceExtensions, allocator);
        if (!device) return Result("Error: vsg::Win32_Window::create(...) failed to create Window, unable to create Vulkan logical Device.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        // set up renderpass with the imageFormat that the swap chain will use
        vsg::SwapChainSupportDetails supportDetails = vsg::querySwapChainSupport(*physicalDevice, *surface);
        VkSurfaceFormatKHR imageFormat = vsg::selectSwapSurfaceFormat(supportDetails);
        VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT; //VK_FORMAT_D32_SFLOAT; // VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_SFLOAT_S8_UINT
        vsg::ref_ptr<vsg::RenderPass> renderPass = vsg::RenderPass::create(device, imageFormat.format, depthFormat, allocator);
        if (!renderPass) return Result("Error: vsg::Win32_Window::create(...) failed to create Window, unable to create Vulkan RenderPass.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        window = new Win32_Window(hwnd, instance, surface, physicalDevice, device, renderPass, debugLayer);
    }

    window->buildSwapchain(finalWidth, finalHeight);

    return Result(window);
}

Win32_Window::~Win32_Window()
{
    clear();

    if (_window != nullptr)
    {
        std::cout << "Calling DestroyWindow(_window);" << std::endl;

        ::DestroyWindow(_window);
        unregisterWindow(_window);
        _window = nullptr;

        if (s_registeredWindows.empty())
            ::UnregisterClass(kWindowClassName.c_str(), ::GetModuleHandle(NULL));
    }
}

bool Win32_Window::pollEvents()
{
    MSG msg;

    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            // glfw uses this to close windows
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return false;
}

bool Win32_Window::resized() const
{
    RECT windowRect;
    GetClientRect(_window, &windowRect);

    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;

    return (width != int(_extent2D.width) || height != int(_extent2D.height));
}

void Win32_Window::resize()
{
    RECT windowRect;
    GetClientRect(_window, &windowRect);

    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;

    buildSwapchain(width, height);
}

LRESULT Win32_Window::handleWin32Messages(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // following event handling code is a placeholder that uses pumex code to help give us an idea of what will be needed
    switch (msg)
    {
    case WM_CLOSE:
        break;
    case WM_DESTROY:
        break;
    case WM_PAINT:
        ValidateRect(_window, NULL);
        break;
    case WM_MOUSEMOVE:
    {
        /*float mx = GET_X_LPARAM(lParam);
        float my = GET_Y_LPARAM(lParam);*/
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        /*::SetCapture(_hwnd);

        InputEvent::MouseButton button = InputEvent::BUTTON_UNDEFINED;
        if (msg == WM_LBUTTONDOWN)      button = InputEvent::LEFT;
        else if (msg == WM_MBUTTONDOWN) button = InputEvent::MIDDLE;
        else                            button = InputEvent::RIGHT;
        pressedMouseButtons.insert(button);

        float mx = GET_X_LPARAM(lParam);
        float my = GET_Y_LPARAM(lParam);
        normalizeMouseCoordinates(mx, my);
        pushInputEvent(InputEvent(timeNow, InputEvent::MOUSE_KEY_PRESSED, button, mx, my));
        */
    }
    break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        /*InputEvent::MouseButton button = InputEvent::BUTTON_UNDEFINED;
        if (msg == WM_LBUTTONUP)      button = InputEvent::LEFT;
        else if (msg == WM_MBUTTONUP) button = InputEvent::MIDDLE;
        else                          button = InputEvent::RIGHT;

        pressedMouseButtons.erase(button);
        if (pressedMouseButtons.empty())
            ::ReleaseCapture();

        float mx = GET_X_LPARAM(lParam);
        float my = GET_Y_LPARAM(lParam);
        normalizeMouseCoordinates(mx, my);
        pushInputEvent(InputEvent(timeNow, InputEvent::MOUSE_KEY_RELEASED, button, mx, my));
        */
    }
    break;
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    {
        /*::SetCapture(_hwnd);

        InputEvent::MouseButton button;
        if (msg == WM_LBUTTONDBLCLK)      button = InputEvent::LEFT;
        else if (msg == WM_MBUTTONDBLCLK) button = InputEvent::MIDDLE;
        else                              button = InputEvent::RIGHT;

        pressedMouseButtons.insert(button);

        float mx = GET_X_LPARAM(lParam);
        float my = GET_Y_LPARAM(lParam);
        normalizeMouseCoordinates(mx, my);
        pushInputEvent(InputEvent(timeNow, InputEvent::MOUSE_KEY_DOUBLE_PRESSED, button, mx, my));
        */
    }
    break;
    case WM_MOUSEWHEEL:
        break;
    case WM_MOVE:
        break;
    case WM_SIZE:
        break;
    case WM_EXITSIZEMOVE:
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        /*InputEvent::Key key = win32KeyCodeToPumex(wParam);
        pushInputEvent(InputEvent(timeNow, InputEvent::KEYBOARD_KEY_PRESSED, key));
        */
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        /*InputEvent::Key key = win32KeyCodeToPumex(wParam);
        pushInputEvent(InputEvent(timeNow, InputEvent::KEYBOARD_KEY_RELEASED, key));
        */
        break;
    }
    default:
        break;
    }
    return ::DefWindowProc(_window, msg, wParam, lParam);
}

//
// static functions for registering windows

void Win32_Window::registerWindow(HWND hwnd, Win32_Window* window)
{
    s_registeredWindows.insert({hwnd, window});
}

void Win32_Window::unregisterWindow(HWND hwnd)
{
    s_registeredWindows.erase(hwnd);
}

Win32_Window* Win32_Window::getWindow(HWND hwnd)
{
    auto it = s_registeredWindows.find(hwnd);
    if (it == end(s_registeredWindows))
        it = s_registeredWindows.find(nullptr);
    if (it == end(s_registeredWindows))
        return nullptr;
    return it->second;
}
