/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/platform/win32/Win32_Window.h>

#include <vsg/vk/Extensions.h>

#include <iostream>

using namespace vsg;
using namespace vsgWin32;

namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to a Win32_Window
    Window::Result Window::create(vsg::ref_ptr<Window::Traits> traits)
    {
        return vsgWin32::Win32_Window::create(traits);
    }

    vsg::Names Window::getInstanceExtensions()
    {
        // check the extensions are available first
        Names requiredExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

        if (!vsg::isExtensionListSupported(requiredExtensions))
        {
            std::cout << "Error: vsg::getInstanceExtensions(...) unable to create window, VK_KHR_surface or VK_KHR_win32_surface not supported." << std::endl;
            return Names();
        }

        return requiredExtensions;
    }

} // namespace vsg

namespace vsgWin32
{
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
            surfaceCreateInfo.pNext = nullptr;

            auto result = vkCreateWin32SurfaceKHR(*instance, &surfaceCreateInfo, nullptr, &_surface);
        }
    };

    // our windows events callback
    LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Win32_Window* win = (Win32_Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (win != nullptr) return win->handleWin32Messages(msg, wParam, lParam);
        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

} // namespace vsgWin32

KeyboardMap::KeyboardMap()
{
    _keycodeMap =
        {
            {0x0, KEY_Undefined},

            {VK_SPACE, KEY_Space},

            {'0', KEY_0},
            {'1', KEY_1},
            {'2', KEY_2},
            {'3', KEY_3},
            {'4', KEY_4},
            {'5', KEY_5},
            {'6', KEY_6},
            {'7', KEY_7},
            {'8', KEY_8},
            {'9', KEY_9},

            {'a', KEY_a},
            {'b', KEY_b},
            {'c', KEY_c},
            {'d', KEY_d},
            {'e', KEY_e},
            {'f', KEY_f},
            {'g', KEY_g},
            {'h', KEY_h},
            {'i', KEY_i},
            {'j', KEY_j},
            {'k', KEY_k},
            {'l', KEY_l},
            {'m', KEY_m},
            {'n', KEY_n},
            {'o', KEY_o},
            {'p', KEY_p},
            {'q', KEY_q},
            {'r', KEY_r},
            {'s', KEY_s},
            {'t', KEY_t},
            {'u', KEY_u},
            {'z', KEY_v},
            {'w', KEY_w},
            {'x', KEY_x},
            {'y', KEY_y},
            {'z', KEY_z},

            {'A', KEY_A},
            {'B', KEY_B},
            {'C', KEY_C},
            {'D', KEY_D},
            {'E', KEY_E},
            {'F', KEY_F},
            {'G', KEY_G},
            {'H', KEY_H},
            {'I', KEY_I},
            {'J', KEY_J},
            {'K', KEY_K},
            {'L', KEY_L},
            {'M', KEY_M},
            {'N', KEY_N},
            {'O', KEY_O},
            {'P', KEY_P},
            {'Q', KEY_Q},
            {'R', KEY_R},
            {'S', KEY_S},
            {'T', KEY_T},
            {'U', KEY_U},
            {'V', KEY_V},
            {'W', KEY_W},
            {'X', KEY_X},
            {'Y', KEY_Y},
            {'Z', KEY_Z},

            /* Cursor control & motion */

            {VK_HOME, KEY_Home},
            {VK_LEFT, KEY_Left},   /* Move left, left arrow */
            {VK_UP, KEY_Up},       /* Move up, up arrow */
            {VK_RIGHT, KEY_Right}, /* Move right, right arrow */
            {VK_DOWN, KEY_Down},   /* Move down, down arrow */
            {VK_PRIOR, KEY_Prior}, /* Prior, previous */
            //{ VK_, KEY_Page_Up = 0xFF55,
            {VK_NEXT, KEY_Next}, /* Next */
            //KEY_Page_Down = 0xFF56,
            {VK_END, KEY_End}, /* EOL */
            //{ KEY_Begin = 0xFF58, /* BOL */

            {'!', KEY_Exclaim},
            {'"', KEY_Quotedbl},
            {'#', KEY_Hash},
            {'$', KEY_Dollar},
            {'&', KEY_Ampersand},
            {VK_OEM_7, KEY_Quote},
            {'(', KEY_Leftparen},
            {')', KEY_Rightparen},
            {'*', KEY_Asterisk},
            {'+', KEY_Plus},
            {VK_OEM_COMMA, KEY_Comma},
            {VK_OEM_MINUS, KEY_Minus},
            {VK_OEM_PERIOD, KEY_Period},
            {VK_OEM_2, KEY_Slash},
            {':', KEY_Colon},
            {VK_OEM_1, KEY_Semicolon},
            {'<', KEY_Less},
            {VK_OEM_PLUS, KEY_Equals}, // + isn't an unmodded key, why does windows map is as a virtual??
            {'>', KEY_Greater},
            {'?', KEY_Question},
            {'@', KEY_At},
            {VK_OEM_4, KEY_Leftbracket},
            {VK_OEM_5, KEY_Backslash},
            {VK_OEM_6, KEY_Rightbracket},
            {'|', KEY_Caret},
            {'_', KEY_Underscore},
            {0xc0, KEY_Backquote},

            {VK_BACK, KEY_BackSpace}, /* back space, back char */
            {VK_TAB, KEY_Tab},
            //    KEY_Linefeed = 0xFF0A, /* Linefeed, LF */
            {VK_CLEAR, KEY_Clear},
            {VK_RETURN, KEY_Return}, /* Return, enter */
            {VK_PAUSE, KEY_Pause},   /* Pause, hold */
            {VK_SCROLL, KEY_Scroll_Lock},
            //    KEY_Sys_Req = 0xFF15,
            {VK_ESCAPE, KEY_Escape},
            {VK_DELETE, KEY_Delete}, /* Delete, rubout */

            /* Misc Functions */

            {VK_SELECT, KEY_Select}, /* Select, mark */
            {VK_PRINT, KEY_Print},
            {VK_EXECUTE, KEY_Execute}, /* Execute, run, do */
            {VK_INSERT, KEY_Insert},   /* Insert, insert here */
            //{ KEY_Undo = 0xFF65,    /* Undo, oops */
            //KEY_Redo = 0xFF66,    /* redo, again */
            {VK_APPS, KEY_Menu}, /* On Windows, this is VK_APPS, the context-menu key */
            // KEY_Find = 0xFF68,    /* Find, search */
            {VK_CANCEL, KEY_Cancel}, /* Cancel, stop, abort, exit */
            {VK_HELP, KEY_Help},     /* Help */
            //{ KEY_Break = 0xFF6B,
            //KEY_Mode_switch = 0xFF7E,   /* Character set switch */
            //KEY_Script_switch = 0xFF7E, /* Alias for mode_switch */
            {VK_NUMLOCK, KEY_Num_Lock},

            /* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

            //KEY_KP_Space = 0xFF80, /* space */
            //KEY_KP_Tab = 0xFF89,
            //KEY_KP_Enter = 0xFF8D, /* enter */
            //KEY_KP_F1 = 0xFF91,    /* PF1, KP_A, ... */
            //KEY_KP_F2 = 0xFF92,
            //KEY_KP_F3 = 0xFF93,
            //KEY_KP_F4 = 0xFF94,
            //KEY_KP_Home = 0xFF95,
            //KEY_KP_Left = 0xFF96,
            //KEY_KP_Up = 0xFF97,
            //KEY_KP_Right = 0xFF98,
            //KEY_KP_Down = 0xFF99,
            //KEY_KP_Prior = 0xFF9A,
            //KEY_KP_Page_Up = 0xFF9A,
            //KEY_KP_Next = 0xFF9B,
            //KEY_KP_Page_Down = 0xFF9B,
            //KEY_KP_End = 0xFF9C,
            //KEY_KP_Begin = 0xFF9D,
            //KEY_KP_Insert = 0xFF9E,
            //KEY_KP_Delete = 0xFF9F,
            //KEY_KP_Equal = 0xFFBD, /* equals */
            //KEY_KP_Multiply = 0xFFAA,
            //KEY_KP_Add = 0xFFAB,
            //KEY_KP_Separator = 0xFFAC, /* separator, often comma */
            //KEY_KP_Subtract = 0xFFAD,
            //KEY_KP_Decimal = 0xFFAE,
            //KEY_KP_Divide = 0xFFAF,

            {VK_NUMPAD0, KEY_KP_0},
            {VK_NUMPAD1, KEY_KP_1},
            {VK_NUMPAD2, KEY_KP_2},
            {VK_NUMPAD3, KEY_KP_3},
            {VK_NUMPAD4, KEY_KP_4},
            {VK_NUMPAD5, KEY_KP_5},
            {VK_NUMPAD6, KEY_KP_6},
            {VK_NUMPAD7, KEY_KP_7},
            {VK_NUMPAD8, KEY_KP_8},
            {VK_NUMPAD9, KEY_KP_9},

            /*
        * Auxiliary Functions; note the duplicate definitions for left and right
        * function keys;  Sun keyboards and a few other manufactures have such
        * function key groups on the left and/or right sides of the keyboard.
        * We've not found a keyboard with more than 35 function keys total.
        */

            {VK_F1, KEY_F1},
            {VK_F2, KEY_F2},
            {VK_F3, KEY_F3},
            {VK_F4, KEY_F4},
            {VK_F5, KEY_F5},
            {VK_F6, KEY_F6},
            {VK_F7, KEY_F7},
            {VK_F8, KEY_F8},
            {VK_F9, KEY_F9},
            {VK_F10, KEY_F10},
            {VK_F11, KEY_F11},
            {VK_F12, KEY_F12},
            {VK_F13, KEY_F13},
            {VK_F14, KEY_F14},
            {VK_F15, KEY_F15},
            {VK_F16, KEY_F16},
            {VK_F17, KEY_F17},
            {VK_F18, KEY_F18},
            {VK_F19, KEY_F19},
            {VK_F20, KEY_F20},
            {VK_F21, KEY_F21},
            {VK_F22, KEY_F22},
            {VK_F23, KEY_F23},
            {VK_F24, KEY_F24},

            //KEY_F25 = 0xFFD6,
            //KEY_F26 = 0xFFD7,
            //KEY_F27 = 0xFFD8,
            //KEY_F28 = 0xFFD9,
            //KEY_F29 = 0xFFDA,
            //KEY_F30 = 0xFFDB,
            //KEY_F31 = 0xFFDC,
            //KEY_F32 = 0xFFDD,
            //KEY_F33 = 0xFFDE,
            //KEY_F34 = 0xFFDF,
            //KEY_F35 = 0xFFE0,

            /* Modifiers */

            {VK_LSHIFT, KEY_Shift_L},     /* Left shift */
            {VK_RSHIFT, KEY_Shift_R},     /* Right shift */
            {VK_LCONTROL, KEY_Control_L}, /* Left control */
            {VK_RCONTROL, KEY_Control_R}, /* Right control */
            {VK_CAPITAL, KEY_Caps_Lock},  /* Caps lock */
            //KEY_Shift_Lock = 0xFFE6, /* Shift lock */

            //KEY_Meta_L = 0xFFE7,  /* Left meta */
            //KEY_Meta_R = 0xFFE8,  /* Right meta */
            {VK_LMENU, KEY_Alt_L},  /* Left alt */
            {VK_RMENU, KEY_Alt_R},  /* Right alt */
            {VK_LWIN, KEY_Super_L}, /* Left super */
            {VK_RWIN, KEY_Super_R}  /* Right super */
            //KEY_Hyper_L = 0xFFED, /* Left hyper */
            //KEY_Hyper_R = 0xFFEE  /* Right hyper */
        };
}

Win32_Window::Result Win32_Window::create(vsg::ref_ptr<Window::Traits> traits, vsg::AllocationCallbacks* allocator)
{
    try
    {
        ref_ptr<Window> window(new Win32_Window(traits, allocator));
        return Result(window);
    }
    catch (vsg::Window::Result result)
    {
        return result;
    }
}

Win32_Window::Win32_Window(vsg::ref_ptr<Window::Traits> traits, vsg::AllocationCallbacks* allocator) :
    Window(traits, allocator),
    _window(nullptr)
{
    _keyboard = new KeyboardMap;

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
    wc.lpszClassName = traits->windowClass.c_str();
    wc.hIconSm = 0;

    if (::RegisterClassEx(&wc) == 0)
    {
        auto lastError = ::GetLastError();
        if (lastError != ERROR_CLASS_ALREADY_EXISTS) throw Result("Error: vsg::Win32_Window::create(...) failed to create Window, could not register window class.", VK_ERROR_INITIALIZATION_FAILED);
    }

    // fetch screen display information

    std::vector<DISPLAY_DEVICE> displayDevices;
    DISPLAY_DEVICE displayDevice;
    displayDevice.cb = sizeof(displayDevice);

    for (uint32_t deviceNum = 0; EnumDisplayDevices(nullptr, deviceNum, &displayDevice, 0); ++deviceNum)
    {
        if (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) continue;
        if (!(displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) continue;

        displayDevices.push_back(displayDevice);
    }

    if (traits->screenNum >= displayDevices.size()) throw Result("Error: vsg::Win32_Window::create(...) failed to create Window, screenNum is out of range.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

    DEVMODE deviceMode;
    deviceMode.dmSize = sizeof(deviceMode);
    deviceMode.dmDriverExtra = 0;

    if (!::EnumDisplaySettings(displayDevices[traits->screenNum].DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode)) throw Result("Error: vsg::Win32_Window::create(...) failed to create Window, EnumDisplaySettings failed to fetch display settings.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

    // setup window rect and style
    int32_t screenx = 0;
    int32_t screeny = 0;
    RECT windowRect;

    unsigned int windowStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    unsigned int extendedStyle = 0;

    if(!traits->fullscreen)
    {
        screenx = deviceMode.dmPosition.x + traits->x;
        screeny = deviceMode.dmPosition.y + traits->y;

        windowRect.left = screenx;
        windowRect.top = screeny;
        windowRect.right = windowRect.left + traits->width;
        windowRect.bottom = windowRect.top + traits->height;

        if (traits->decoration)
        {
            windowStyle |= WS_OVERLAPPEDWINDOW;

            extendedStyle |= WS_EX_WINDOWEDGE | 
                WS_EX_APPWINDOW |
                WS_EX_OVERLAPPEDWINDOW |
                WS_EX_ACCEPTFILES |
                WS_EX_LTRREADING;

            // if decorated call adjust to account for borders etc
            if (!::AdjustWindowRectEx(&windowRect, windowStyle, FALSE, extendedStyle)) throw Result("Error: vsg::Win32_Window::create(...) failed to create Window, AdjustWindowRectEx failed.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

        }
    }
    else
    {
        screenx = deviceMode.dmPosition.x;
        screeny = deviceMode.dmPosition.y;

        windowRect.left = screenx;
        windowRect.top = screeny;
        windowRect.right = windowRect.left + deviceMode.dmPelsWidth;
        windowRect.bottom = windowRect.top + deviceMode.dmPelsHeight;
    }

    // create the window
    _window = ::CreateWindowEx(extendedStyle, traits->windowClass.c_str(), traits->windowTitle.c_str(), windowStyle,
                               windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
                               NULL, NULL, ::GetModuleHandle(NULL), NULL);

    if (_window == nullptr) throw Result("Error: vsg::Win32_Window::create(...) failed to create Window, CreateWindowEx did not return a valid window handle.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

    // set window handle user data pointer to hold ref to this so we can retrieve in WindowsProc
    SetWindowLongPtr(_window, GWLP_USERDATA, (LONG_PTR)this);

    // reposition once the window has been created to account for borders etc
    ::SetWindowPos(_window, nullptr, screenx, screeny, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 0);

    // get client rect to find final width height of the view
    RECT clientRect;
    ::GetClientRect(_window, &clientRect);

    uint32_t finalWidth = clientRect.right - clientRect.left;
    uint32_t finalHeight = clientRect.bottom - clientRect.top;

    vsg::ref_ptr<Win32_Window> window;

    if (traits->shareWindow)
    {
        // share the _instance, _physicalDevice and _device;
        window->share(*traits->shareWindow);

        // create surface
        vsg::ref_ptr<vsg::Surface> surface(new vsgWin32::Win32Surface(traits->shareWindow->instance(), _window, allocator));
        _surface = surface;

        // temporary hack to force vkGetPhysicalDeviceSurfaceSupportKHR to be called as the Vulkan
        // debug layer is complaining about vkGetPhysicalDeviceSurfaceSupportKHR not being called
        // for this _surface prior to swap chain creation
        vsg::ref_ptr<vsg::PhysicalDevice> physicalDevice = vsg::PhysicalDevice::create(traits->shareWindow->instance(), VK_QUEUE_GRAPHICS_BIT, surface);
    }
    else
    {
        // create win32 surface
        vsg::ref_ptr<vsg::Surface> surface(new vsgWin32::Win32Surface(_instance, _window, allocator));
        if (!surface) throw Result("Error: vsg::Win32_Window::create(...) failed to create Window, unable to create Win32Surface.", VK_ERROR_INVALID_EXTERNAL_HANDLE);
        _surface = surface;

        // set up device
        initaliseDevice();
    }

    buildSwapchain(finalWidth, finalHeight);

    ShowWindow(_window, SW_SHOW);
    SetForegroundWindow(_window);
    SetFocus(_window);
}

Win32_Window::~Win32_Window()
{
    clear();

    if (_window != nullptr)
    {
        std::cout << "Calling DestroyWindow(_window);" << std::endl;

        TCHAR className[MAX_PATH];
        GetClassName(_window, className, MAX_PATH);

        ::DestroyWindow(_window);
        _window = nullptr;

        // when should we unregister??
        ::UnregisterClass(className, ::GetModuleHandle(NULL));
    }
}

bool Win32_Window::pollEvents(vsg::Events& events)
{
    vsg::clock::time_point event_time = vsg::clock::now();

    MSG msg;

    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            // somehow close all windows
            events.emplace_back(new vsg::CloseWindowEvent(this, event_time));
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    if (_bufferedEvents.size() > 0)
    {
        events.splice(events.end(), _bufferedEvents);
        _bufferedEvents.clear();
        return true;
    }

    return false;
}

bool Win32_Window::resized() const
{
    RECT windowRect;
    GetClientRect(_window, &windowRect);

    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;

    // at the moment the close event is occurring then the check for resize is happening, which means the window is rect returns 0. So for now ignore potential resize if should close
    return width != int(_extent2D.width) || height != int(_extent2D.height);
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
    vsg::clock::time_point event_time = vsg::clock::now();

    // get the current window rect
    RECT windowRect;
    GetClientRect(_window, &windowRect);

    int32_t winx = windowRect.left;
    int32_t winy = windowRect.top;
    int32_t winw = windowRect.right - windowRect.left;
    int32_t winh = windowRect.bottom - windowRect.top;

    switch (msg)
    {
    case WM_CLOSE:
        std::cout << "close window" << std::endl;
        _bufferedEvents.emplace_back(new vsg::CloseWindowEvent(this, event_time));
        break;
    case WM_SHOWWINDOW:
        _bufferedEvents.emplace_back(new vsg::ExposeWindowEvent(this, event_time, winx, winy, winw, winh));
        break;
    case WM_DESTROY:
        break;
    case WM_PAINT:
        ValidateRect(_window, NULL);
        break;
    case WM_MOUSEMOVE:
    {
        int32_t mx = GET_X_LPARAM(lParam);
        int32_t my = GET_Y_LPARAM(lParam);

        _bufferedEvents.emplace_back(new vsg::MoveEvent(this, event_time, mx, my, getButtonMask(wParam)));
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        uint32_t mx = GET_X_LPARAM(lParam);
        uint32_t my = GET_Y_LPARAM(lParam);

        _bufferedEvents.emplace_back(new vsg::ButtonPressEvent(this, event_time, mx, my, getButtonMask(wParam), getButtonEventDetail(msg)));

        //::SetCapture(_window);
    }
    break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        uint32_t mx = GET_X_LPARAM(lParam);
        uint32_t my = GET_Y_LPARAM(lParam);

        uint32_t button = msg == WM_LBUTTONDOWN ? 1 : (msg == WM_RBUTTONDOWN ? 2 : msg == WM_MBUTTONDOWN ? 3 : (msg == WM_XBUTTONDOWN ? 4 : 0)); // need to determine x1, x2
        _bufferedEvents.emplace_back(new vsg::ButtonReleaseEvent(this, event_time, mx, my, getButtonMask(wParam), getButtonEventDetail(msg)));

        //::ReleaseCapture(); // should only release once all mouse buttons are released ??
        break;
    }
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    {
        //::SetCapture(_window);
    }
    break;
    case WM_MOUSEWHEEL:
        break;
    case WM_MOVE:
    case WM_SIZE:
    {
        _bufferedEvents.emplace_back(new vsg::ConfigureWindowEvent(this, event_time, winx, winy, winw, winh));
        break;
    }
    case WM_EXITSIZEMOVE:
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        vsg::KeySymbol keySymbol, modifiedKeySymbol;
        vsg::KeyModifier keyModifier;
        if (_keyboard->getKeySymbol(wParam, lParam, keySymbol, modifiedKeySymbol, keyModifier))
        {
            int repeatCount = (lParam & 0xffff);
            _bufferedEvents.emplace_back(new vsg::KeyPressEvent(this, event_time, keySymbol, modifiedKeySymbol, keyModifier, repeatCount));
        }
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        vsg::KeySymbol keySymbol, modifiedKeySymbol;
        vsg::KeyModifier keyModifier;
        if (_keyboard->getKeySymbol(wParam, lParam, keySymbol, modifiedKeySymbol, keyModifier))
        {
            _bufferedEvents.emplace_back(new vsg::KeyReleaseEvent(this, event_time, keySymbol, modifiedKeySymbol, keyModifier, 0));
        }

        break;
    }
    default:
        break;
    }
    return ::DefWindowProc(_window, msg, wParam, lParam);
}
