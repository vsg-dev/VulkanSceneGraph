/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/platform/win32/Win32_Window.h>
#include <vsg/core/Exception.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/vk/Extensions.h>
#include <vsg/io/Options.h>

#include <iostream>

using namespace vsg;
using namespace vsgWin32;

namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to a Win32_Window
    ref_ptr<Window> Window::create(vsg::ref_ptr<WindowTraits> traits)
    {
        return vsgWin32::Win32_Window::create(traits);
    }

} // namespace vsg

namespace vsgWin32
{
    class VSG_DECLSPEC Win32Surface : public vsg::Surface
    {
    public:
        Win32Surface(vsg::Instance* instance, HWND window) :
            vsg::Surface(VK_NULL_HANDLE, instance)
        {
            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.hinstance = ::GetModuleHandle(NULL);
            surfaceCreateInfo.hwnd = window;
            surfaceCreateInfo.pNext = nullptr;

            auto result = vkCreateWin32SurfaceKHR(*instance, &surfaceCreateInfo, _instance->getAllocationCallbacks(), &_surface);
        }
    };

    // our windows events callback
    LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Win32_Window* win = reinterpret_cast<Win32_Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (win != nullptr) return win->handleWin32Messages(msg, wParam, lParam);
        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

} // namespace vsgWin32

KeyboardMap::KeyboardMap()
{
    _keycodeMap =
        {
            {0x0, KeySymbol::KEY_Undefined},

            {VK_SPACE, KeySymbol::KEY_Space},

            {'0', KeySymbol::KEY_0},
            {'1', KeySymbol::KEY_1},
            {'2', KeySymbol::KEY_2},
            {'3', KeySymbol::KEY_3},
            {'4', KeySymbol::KEY_4},
            {'5', KeySymbol::KEY_5},
            {'6', KeySymbol::KEY_6},
            {'7', KeySymbol::KEY_7},
            {'8', KeySymbol::KEY_8},
            {'9', KeySymbol::KEY_9},

            {'a', KeySymbol::KEY_a},
            {'b', KeySymbol::KEY_b},
            {'c', KeySymbol::KEY_c},
            {'d', KeySymbol::KEY_d},
            {'e', KeySymbol::KEY_e},
            {'f', KeySymbol::KEY_f},
            {'g', KeySymbol::KEY_g},
            {'h', KeySymbol::KEY_h},
            {'i', KeySymbol::KEY_i},
            {'j', KeySymbol::KEY_j},
            {'k', KeySymbol::KEY_k},
            {'l', KeySymbol::KEY_l},
            {'m', KeySymbol::KEY_m},
            {'n', KeySymbol::KEY_n},
            {'o', KeySymbol::KEY_o},
            {'p', KeySymbol::KEY_p},
            {'q', KeySymbol::KEY_q},
            {'r', KeySymbol::KEY_r},
            {'s', KeySymbol::KEY_s},
            {'t', KeySymbol::KEY_t},
            {'u', KeySymbol::KEY_u},
            {'z', KeySymbol::KEY_v},
            {'w', KeySymbol::KEY_w},
            {'x', KeySymbol::KEY_x},
            {'y', KeySymbol::KEY_y},
            {'z', KeySymbol::KEY_z},

            {'A', KeySymbol::KEY_A},
            {'B', KeySymbol::KEY_B},
            {'C', KeySymbol::KEY_C},
            {'D', KeySymbol::KEY_D},
            {'E', KeySymbol::KEY_E},
            {'F', KeySymbol::KEY_F},
            {'G', KeySymbol::KEY_G},
            {'H', KeySymbol::KEY_H},
            {'I', KeySymbol::KEY_I},
            {'J', KeySymbol::KEY_J},
            {'K', KeySymbol::KEY_K},
            {'L', KeySymbol::KEY_L},
            {'M', KeySymbol::KEY_M},
            {'N', KeySymbol::KEY_N},
            {'O', KeySymbol::KEY_O},
            {'P', KeySymbol::KEY_P},
            {'Q', KeySymbol::KEY_Q},
            {'R', KeySymbol::KEY_R},
            {'S', KeySymbol::KEY_S},
            {'T', KeySymbol::KEY_T},
            {'U', KeySymbol::KEY_U},
            {'V', KeySymbol::KEY_V},
            {'W', KeySymbol::KEY_W},
            {'X', KeySymbol::KEY_X},
            {'Y', KeySymbol::KEY_Y},
            {'Z', KeySymbol::KEY_Z},

            /* Cursor control & motion */

            {VK_HOME, KeySymbol::KEY_Home},
            {VK_LEFT, KeySymbol::KEY_Left},   /* Move left, left arrow */
            {VK_UP, KeySymbol::KEY_Up},       /* Move up, up arrow */
            {VK_RIGHT, KeySymbol::KEY_Right}, /* Move right, right arrow */
            {VK_DOWN, KeySymbol::KEY_Down},   /* Move down, down arrow */
            {VK_PRIOR, KeySymbol::KEY_Prior}, /* Prior, previous */
            //{ VK_, KEY_Page_Up = 0xFF55,
            {VK_NEXT, KeySymbol::KEY_Next}, /* Next */
            //KEY_Page_Down = 0xFF56,
            {VK_END, KeySymbol::KEY_End}, /* EOL */
            //{ KEY_Begin = 0xFF58, /* BOL */

            {'!', KeySymbol::KEY_Exclaim},
            {'"', KeySymbol::KEY_Quotedbl},
            {'#', KeySymbol::KEY_Hash},
            {'$', KeySymbol::KEY_Dollar},
            {'&', KeySymbol::KEY_Ampersand},
            {VK_OEM_7, KeySymbol::KEY_Quote},
            {'(', KeySymbol::KEY_Leftparen},
            {')', KeySymbol::KEY_Rightparen},
            {'*', KeySymbol::KEY_Asterisk},
            {'+', KeySymbol::KEY_Plus},
            {VK_OEM_COMMA, KeySymbol::KEY_Comma},
            {VK_OEM_MINUS, KeySymbol::KEY_Minus},
            {VK_OEM_PERIOD, KeySymbol::KEY_Period},
            {VK_OEM_2, KeySymbol::KEY_Slash},
            {':', KeySymbol::KEY_Colon},
            {VK_OEM_1, KeySymbol::KEY_Semicolon},
            {'<', KeySymbol::KEY_Less},
            {VK_OEM_PLUS, KeySymbol::KEY_Equals}, // + isn't an unmodded key, why does windows map is as a virtual??
            {'>', KeySymbol::KEY_Greater},
            {'?', KeySymbol::KEY_Question},
            {'@', KeySymbol::KEY_At},
            {VK_OEM_4, KeySymbol::KEY_Leftbracket},
            {VK_OEM_5, KeySymbol::KEY_Backslash},
            {VK_OEM_6, KeySymbol::KEY_Rightbracket},
            {'|', KeySymbol::KEY_Caret},
            {'_', KeySymbol::KEY_Underscore},
            {0xc0, KeySymbol::KEY_Backquote},

            {VK_BACK, KeySymbol::KEY_BackSpace}, /* back space, back char */
            {VK_TAB, KeySymbol::KEY_Tab},
            //    KEY_Linefeed = 0xFF0A, /* Linefeed, LF */
            {VK_CLEAR, KeySymbol::KEY_Clear},
            {VK_RETURN, KeySymbol::KEY_Return}, /* Return, enter */
            {VK_PAUSE, KeySymbol::KEY_Pause},   /* Pause, hold */
            {VK_SCROLL, KeySymbol::KEY_Scroll_Lock},
            //    KEY_Sys_Req = 0xFF15,
            {VK_ESCAPE, KeySymbol::KEY_Escape},
            {VK_DELETE, KeySymbol::KEY_Delete}, /* Delete, rubout */

            /* Misc Functions */

            {VK_SELECT, KeySymbol::KEY_Select}, /* Select, mark */
            {VK_PRINT, KeySymbol::KEY_Print},
            {VK_EXECUTE, KeySymbol::KEY_Execute}, /* Execute, run, do */
            {VK_INSERT, KeySymbol::KEY_Insert},   /* Insert, insert here */
            //{ KEY_Undo = 0xFF65,    /* Undo, oops */
            //KEY_Redo = 0xFF66,    /* redo, again */
            {VK_APPS, KeySymbol::KEY_Menu}, /* On Windows, this is VK_APPS, the context-menu key */
            // KEY_Find = 0xFF68,    /* Find, search */
            {VK_CANCEL, KeySymbol::KEY_Cancel}, /* Cancel, stop, abort, exit */
            {VK_HELP, KeySymbol::KEY_Help},     /* Help */
            //{ KEY_Break = 0xFF6B,
            //KEY_Mode_switch = 0xFF7E,   /* Character set switch */
            //KEY_Script_switch = 0xFF7E, /* Alias for mode_switch */
            {VK_NUMLOCK, KeySymbol::KEY_Num_Lock},

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

            {VK_NUMPAD0, KeySymbol::KEY_KP_0},
            {VK_NUMPAD1, KeySymbol::KEY_KP_1},
            {VK_NUMPAD2, KeySymbol::KEY_KP_2},
            {VK_NUMPAD3, KeySymbol::KEY_KP_3},
            {VK_NUMPAD4, KeySymbol::KEY_KP_4},
            {VK_NUMPAD5, KeySymbol::KEY_KP_5},
            {VK_NUMPAD6, KeySymbol::KEY_KP_6},
            {VK_NUMPAD7, KeySymbol::KEY_KP_7},
            {VK_NUMPAD8, KeySymbol::KEY_KP_8},
            {VK_NUMPAD9, KeySymbol::KEY_KP_9},

            /*
        * Auxiliary Functions; note the duplicate definitions for left and right
        * function keys;  Sun keyboards and a few other manufactures have such
        * function key groups on the left and/or right sides of the keyboard.
        * We've not found a keyboard with more than 35 function keys total.
        */

            {VK_F1, KeySymbol::KEY_F1},
            {VK_F2, KeySymbol::KEY_F2},
            {VK_F3, KeySymbol::KEY_F3},
            {VK_F4, KeySymbol::KEY_F4},
            {VK_F5, KeySymbol::KEY_F5},
            {VK_F6, KeySymbol::KEY_F6},
            {VK_F7, KeySymbol::KEY_F7},
            {VK_F8, KeySymbol::KEY_F8},
            {VK_F9, KeySymbol::KEY_F9},
            {VK_F10, KeySymbol::KEY_F10},
            {VK_F11, KeySymbol::KEY_F11},
            {VK_F12, KeySymbol::KEY_F12},
            {VK_F13, KeySymbol::KEY_F13},
            {VK_F14, KeySymbol::KEY_F14},
            {VK_F15, KeySymbol::KEY_F15},
            {VK_F16, KeySymbol::KEY_F16},
            {VK_F17, KeySymbol::KEY_F17},
            {VK_F18, KeySymbol::KEY_F18},
            {VK_F19, KeySymbol::KEY_F19},
            {VK_F20, KeySymbol::KEY_F20},
            {VK_F21, KeySymbol::KEY_F21},
            {VK_F22, KeySymbol::KEY_F22},
            {VK_F23, KeySymbol::KEY_F23},
            {VK_F24, KeySymbol::KEY_F24},

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

            {VK_LSHIFT, KeySymbol::KEY_Shift_L},     /* Left shift */
            {VK_RSHIFT, KeySymbol::KEY_Shift_R},     /* Right shift */
            {VK_LCONTROL, KeySymbol::KEY_Control_L}, /* Left control */
            {VK_RCONTROL, KeySymbol::KEY_Control_R}, /* Right control */
            {VK_CAPITAL, KeySymbol::KEY_Caps_Lock},  /* Caps lock */
            //KEY_Shift_Lock = 0xFFE6, /* Shift lock */

            //KEY_Meta_L = 0xFFE7,  /* Left meta */
            //KEY_Meta_R = 0xFFE8,  /* Right meta */
            {VK_LMENU, KeySymbol::KEY_Alt_L},  /* Left alt */
            {VK_RMENU, KeySymbol::KEY_Alt_R},  /* Right alt */
            {VK_LWIN, KeySymbol::KEY_Super_L}, /* Left super */
            {VK_RWIN, KeySymbol::KEY_Super_R}  /* Right super */
            //KEY_Hyper_L = 0xFFED, /* Left hyper */
            //KEY_Hyper_R = 0xFFEE  /* Right hyper */
        };
}

Win32_Window::Win32_Window(vsg::ref_ptr<WindowTraits> traits) :
    Inherit(traits),
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
        if (lastError != ERROR_CLASS_ALREADY_EXISTS) throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, could not register window class.", VK_ERROR_INITIALIZATION_FAILED};
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

    // assume a traits->screenNum of < 0 will default to screen 0
    int32_t screenNum = traits->screenNum < 0 ? 0 : traits->screenNum;
    if (screenNum >= displayDevices.size()) throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, screenNum is out of range.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

    DEVMODE deviceMode;
    deviceMode.dmSize = sizeof(deviceMode);
    deviceMode.dmDriverExtra = 0;

    if (!::EnumDisplaySettings(displayDevices[screenNum].DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode)) throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, EnumDisplaySettings failed to fetch display settings.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

    // setup window rect and style
    int32_t screenx = 0;
    int32_t screeny = 0;
    RECT windowRect;

    uint32_t windowStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    uint32_t extendedStyle = 0;

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
            if (!::AdjustWindowRectEx(&windowRect, windowStyle, FALSE, extendedStyle)) throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, AdjustWindowRectEx failed.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

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

    if (_window == nullptr) throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, CreateWindowEx did not return a valid window handle.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

    // set window handle user data pointer to hold ref to this so we can retrieve in WindowsProc
    SetWindowLongPtr(_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // reposition once the window has been created to account for borders etc
    ::SetWindowPos(_window, nullptr, screenx, screeny, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 0);

    // get client rect to find final width height of the view
    RECT clientRect;
    ::GetClientRect(_window, &clientRect);

    uint32_t finalWidth = clientRect.right - clientRect.left;
    uint32_t finalHeight = clientRect.bottom - clientRect.top;

    if (traits->shareWindow)
    {
        // share the _instance, _physicalDevice and _device;
        share(*traits->shareWindow);
    }

    _extent2D.width = finalWidth;
    _extent2D.height = finalHeight;

    // assign dimensions
    traits->x = windowRect.left;
    traits->y = windowRect.top;
    traits->width = finalWidth;
    traits->height = finalHeight;

    ShowWindow(_window, SW_SHOW);
    SetForegroundWindow(_window);
    SetFocus(_window);
    _windowMapped = true;

    traits->systemConnection = wc.hInstance;
    traits->nativeWindow = _window;
}

Win32_Window::~Win32_Window()
{
    clear();

    if (_window != nullptr)
    {
        // std::cout << "Calling DestroyWindow(_window);" << std::endl;

        TCHAR className[MAX_PATH];
        GetClassName(_window, className, MAX_PATH);

        ::DestroyWindow(_window);
        _window = nullptr;

        // when should we unregister??
        ::UnregisterClass(className, ::GetModuleHandle(NULL));
    }
}

void Win32_Window::_initSurface()
{
    if (!_instance) _initInstance();

    _surface = new vsgWin32::Win32Surface(_instance, _window);
}

bool Win32_Window::visible() const
{
    return _window!=0 && _windowMapped;
}

bool Win32_Window::pollEvents(vsg::UIEvents& events)
{
    vsg::clock::time_point event_time = vsg::clock::now();

    MSG msg;

    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            // somehow close all windows
            events.emplace_back(vsg::CloseWindowEvent::create(this, event_time));
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return Window::pollEvents(events);
}

void Win32_Window::resize()
{
    RECT windowRect;
    GetClientRect(_window, &windowRect);

    _extent2D.width = windowRect.right - windowRect.left;
    _extent2D.height = windowRect.bottom - windowRect.top;

    buildSwapchain();
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
        // std::cout << "close window" << std::endl;
        bufferedEvents.emplace_back(vsg::CloseWindowEvent::create(this, event_time));
        break;
    case WM_SHOWWINDOW:
        bufferedEvents.emplace_back(vsg::ExposeWindowEvent::create(this, event_time, winx, winy, winw, winh));
        break;
    case WM_DESTROY:
        _windowMapped = false;
        break;
    case WM_PAINT:
        ValidateRect(_window, NULL);
        break;
    case WM_MOUSEMOVE:
    {
        int32_t mx = GET_X_LPARAM(lParam);
        int32_t my = GET_Y_LPARAM(lParam);

        bufferedEvents.emplace_back(vsg::MoveEvent::create(this, event_time, mx, my, getButtonMask(wParam)));
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        int32_t mx = GET_X_LPARAM(lParam);
        int32_t my = GET_Y_LPARAM(lParam);

        bufferedEvents.emplace_back(vsg::ButtonPressEvent::create(this, event_time, mx, my, getButtonMask(wParam), getButtonDownEventDetail(msg)));

        //::SetCapture(_window);
    }
    break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        int32_t mx = GET_X_LPARAM(lParam);
        int32_t my = GET_Y_LPARAM(lParam);

        bufferedEvents.emplace_back(vsg::ButtonReleaseEvent::create(this, event_time, mx, my, getButtonMask(wParam), getButtonUpEventDetail(msg)));

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
    {
        bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(this, event_time, GET_WHEEL_DELTA_WPARAM(wParam)<0 ? vec3(0.0f, -1.0f, 0.0f) : vec3(0.0f, 1.0f, 0.0f)));
        break;
    }
    case WM_MOVE:
    {
        bufferedEvents.emplace_back(vsg::ConfigureWindowEvent::create(this, event_time, winx, winy, winw, winh));
        break;
    }
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED || wParam == SIZE_MAXHIDE || winw==0 || winh==0)
        {
            _windowMapped = false;
        }
        else
        {
            _windowMapped = true;
            bufferedEvents.emplace_back(vsg::ConfigureWindowEvent::create(this, event_time, winx, winy, winw, winh));
        }
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
            int32_t repeatCount = (lParam & 0xffff);
            bufferedEvents.emplace_back(vsg::KeyPressEvent::create(this, event_time, keySymbol, modifiedKeySymbol, keyModifier, repeatCount));
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
            bufferedEvents.emplace_back(vsg::KeyReleaseEvent::create(this, event_time, keySymbol, modifiedKeySymbol, keyModifier, 0));
        }

        break;
    }
    default:
        break;
    }
    return ::DefWindowProc(_window, msg, wParam, lParam);
}
