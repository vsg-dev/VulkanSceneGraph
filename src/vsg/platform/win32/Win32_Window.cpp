/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/io/Logger.h>
#include <vsg/platform/win32/Win32_Window.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>

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
            if (result != VK_SUCCESS)
            {
                throw Exception{"Failed to created Win32Surface.", result};
            }
        }
    };

    // our windows events callback
    LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Win32_Window* win = reinterpret_cast<Win32_Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (win != nullptr)
        {
#if 1
            // return if vsg::Win32_Window is able to handle it, otherwise fallback to the ::DefWindowProc(..) below
            if (win->handleWin32Messages(msg, wParam, lParam)) return 0;
#else
            win->handleWin32Messages(msg, wParam, lParam);
#endif
        }
        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

} // namespace vsgWin32

vsg::Exception vsgWin32::getLastErrorAsException(const std::string_view& prefix)
{
    DWORD errorCode = GetLastError();
    if (errorCode == NOERROR) return Exception{std::string(prefix), 0};

    // Hopefully the error will be representable with the current eight-bit code page as Exception doesn't support wide strings
    LPSTR buffer;
    // Ignore the dodgy cast from pointer-to-pointer to pointer
    // the argument should really be a union of those types as it's interpreted differently depending on the flags passed
    std::size_t length = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                        nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, nullptr);

    std::string message(buffer, length);
    LocalFree(buffer);

    if (!prefix.empty()) message.insert(message.begin(), prefix.begin(), prefix.end());
    return Exception{message, static_cast<int>(errorCode)};
}

KeyboardMap::KeyboardMap()
{
    // Note that Windows virtual key 'A' etc. correspond to the unmodified character 'a', hence the map below assigns capital letters to their corresponding lowercase ones.
    // will modify this map.
    // clang-format off
    _vk2vsg =
        {
            {0x0                                  ,              KEY_Undefined},
            {'0'                                  ,              KEY_0},
            {'1'                                  ,              KEY_1},
            {'2'                                  ,              KEY_2},
            {'3'                                  ,              KEY_3},
            {'4'                                  ,              KEY_4},
            {'5'                                  ,              KEY_5},
            {'6'                                  ,              KEY_6},
            {'7'                                  ,              KEY_7},
            {'8'                                  ,              KEY_8},
            {'9'                                  ,              KEY_9},
            {'A'                                  ,              KEY_a},
            {'B'                                  ,              KEY_b},
            {'C'                                  ,              KEY_c},
            {'D'                                  ,              KEY_d},
            {'E'                                  ,              KEY_e},
            {'F'                                  ,              KEY_f},
            {'G'                                  ,              KEY_g},
            {'H'                                  ,              KEY_h},
            {'I'                                  ,              KEY_i},
            {'J'                                  ,              KEY_j},
            {'K'                                  ,              KEY_k},
            {'L'                                  ,              KEY_l},
            {'M'                                  ,              KEY_m},
            {'N'                                  ,              KEY_n},
            {'O'                                  ,              KEY_o},
            {'P'                                  ,              KEY_p},
            {'Q'                                  ,              KEY_q},
            {'R'                                  ,              KEY_r},
            {'S'                                  ,              KEY_s},
            {'T'                                  ,              KEY_t},
            {'U'                                  ,              KEY_u},
            {'V'                                  ,              KEY_v},
            {'W'                                  ,              KEY_w},
            {'X'                                  ,              KEY_x},
            {'Y'                                  ,              KEY_y},
            {'Z'                                  ,              KEY_z},
            {VK_LBUTTON                           ,              KEY_Undefined},
            {VK_RBUTTON                           ,              KEY_Undefined},
            {VK_CANCEL                            ,              KEY_Undefined},
            {VK_MBUTTON                           ,              KEY_Undefined},
            {VK_XBUTTON1                          ,              KEY_Undefined}, /* NOT contiguous with L & RBUTTON */
            {VK_XBUTTON2                          ,              KEY_Undefined}, /* NOT contiguous with L & RBUTTON */
            {VK_BACK                              ,              KEY_BackSpace},
            {VK_TAB                               ,              KEY_Tab},
            {VK_CLEAR                             ,              KEY_Clear},
            {VK_RETURN                            ,              KEY_Return},
            {VK_SHIFT                             ,              KEY_Undefined},
            {VK_CONTROL                           ,              KEY_Undefined},
            {VK_MENU                              ,              KEY_Alt_L},
            {VK_PAUSE                             ,              KEY_Pause},
            {VK_CAPITAL                           ,              KEY_Undefined},
            {VK_KANA                              ,              KEY_Undefined},
            {VK_HANGEUL                           ,              KEY_Undefined},
            {VK_HANGUL                            ,              KEY_Undefined},
            {VK_JUNJA                             ,              KEY_Undefined},
            {VK_FINAL                             ,              KEY_Undefined},
            {VK_HANJA                             ,              KEY_Undefined},
            {VK_KANJI                             ,              KEY_Undefined},
//            {VK_IME_ON                            ,              KEY_Undefined},
//            {VK_IME_OFF                           ,              KEY_Undefined},
            {VK_ESCAPE                            ,              KEY_Escape   },
            {VK_CONVERT                           ,              KEY_Undefined},
            {VK_NONCONVERT                        ,              KEY_Undefined},
            {VK_ACCEPT                            ,              KEY_Undefined},
            {VK_MODECHANGE                        ,              KEY_Undefined},
            {VK_SPACE                             ,              KEY_Space    },
            {VK_PRIOR                             ,              KEY_Prior    },
            {VK_NEXT                              ,              KEY_Next     },
            {VK_END                               ,              KEY_End      },
            {VK_HOME                              ,              KEY_Home     },
            {VK_LEFT                              ,              KEY_Left     },
            {VK_UP                                ,              KEY_Up       },
            {VK_RIGHT                             ,              KEY_Right    },
            {VK_DOWN                              ,              KEY_Down     },
            {VK_SELECT                            ,              KEY_Select},
            {VK_SNAPSHOT                          ,              KEY_Print},
            {VK_EXECUTE                           ,              KEY_Execute},
            {VK_SNAPSHOT                          ,              KEY_Undefined},
            {VK_INSERT                            ,              KEY_Insert},
            {VK_DELETE                            ,              KEY_Delete},
            {VK_HELP                              ,              KEY_Help},
            {VK_LWIN                              ,              KEY_Super_L},
            {VK_RWIN                              ,              KEY_Super_R},
            {VK_APPS                              ,              KEY_Menu},
            {VK_SLEEP                             ,              KEY_Undefined},
            {VK_NUMPAD0                           ,              KEY_KP_0},
            {VK_NUMPAD1                           ,              KEY_KP_1},
            {VK_NUMPAD2                           ,              KEY_KP_2},
            {VK_NUMPAD3                           ,              KEY_KP_3},
            {VK_NUMPAD4                           ,              KEY_KP_4},
            {VK_NUMPAD5                           ,              KEY_KP_5},
            {VK_NUMPAD6                           ,              KEY_KP_6},
            {VK_NUMPAD7                           ,              KEY_KP_7},
            {VK_NUMPAD8                           ,              KEY_KP_8},
            {VK_NUMPAD9                           ,              KEY_KP_9},
            {VK_MULTIPLY                          ,              KEY_KP_Multiply},
            {VK_ADD                               ,              KEY_KP_Add},
            {VK_SEPARATOR                         ,              KEY_KP_Separator},
            {VK_SUBTRACT                          ,              KEY_KP_Subtract},
            {VK_DECIMAL                           ,              KEY_KP_Decimal},
            {VK_DIVIDE                            ,              KEY_KP_Divide},
            {VK_F1                                ,              KEY_F1},
            {VK_F2                                ,              KEY_F2},
            {VK_F3                                ,              KEY_F3},
            {VK_F4                                ,              KEY_F4},
            {VK_F5                                ,              KEY_F5},
            {VK_F6                                ,              KEY_F6},
            {VK_F7                                ,              KEY_F7},
            {VK_F8                                ,              KEY_F8},
            {VK_F9                                ,              KEY_F9},
            {VK_F10                               ,              KEY_F10},
            {VK_F11                               ,              KEY_F11},
            {VK_F12                               ,              KEY_F12},
            {VK_F13                               ,              KEY_F13},
            {VK_F14                               ,              KEY_F14},
            {VK_F15                               ,              KEY_F15},
            {VK_F16                               ,              KEY_F16},
            {VK_F17                               ,              KEY_F17},
            {VK_F18                               ,              KEY_F18},
            {VK_F19                               ,              KEY_F19},
            {VK_F20                               ,              KEY_F20},
            {VK_F21                               ,              KEY_F21},
            {VK_F22                               ,              KEY_F22},
            {VK_F23                               ,              KEY_F23},
            {VK_F24                               ,              KEY_F24},
            {VK_NAVIGATION_VIEW                   ,              KEY_Undefined},   // reserved
            {VK_NAVIGATION_MENU                   ,              KEY_Undefined},   // reserved
            {VK_NAVIGATION_UP                     ,              KEY_Undefined},   // reserved
            {VK_NAVIGATION_DOWN                   ,              KEY_Undefined},   // reserved
            {VK_NAVIGATION_LEFT                   ,              KEY_Undefined},   // reserved
            {VK_NAVIGATION_RIGHT                  ,              KEY_Undefined},   // reserved
            {VK_NAVIGATION_ACCEPT                 ,              KEY_Undefined},   // reserved
            {VK_NAVIGATION_CANCEL                 ,              KEY_Undefined},   // reserved
            {VK_NUMLOCK                           ,              KEY_Undefined},
            {VK_SCROLL                            ,              KEY_Scroll_Lock},
            {VK_LSHIFT                            ,              KEY_Shift_L},
            {VK_RSHIFT                            ,              KEY_Shift_R},
            {VK_LCONTROL                          ,              KEY_Control_L},
            {VK_RCONTROL                          ,              KEY_Control_R},
            {VK_LMENU                             ,              KEY_Alt_L},
            {VK_RMENU                             ,              KEY_Alt_R},
            {VK_BROWSER_BACK                      ,              KEY_Undefined},
            {VK_BROWSER_FORWARD                   ,              KEY_Undefined},
            {VK_BROWSER_REFRESH                   ,              KEY_Undefined},
            {VK_BROWSER_STOP                      ,              KEY_Undefined},
            {VK_BROWSER_SEARCH                    ,              KEY_Undefined},
            {VK_BROWSER_FAVORITES                 ,              KEY_Undefined},
            {VK_BROWSER_HOME                      ,              KEY_Undefined},
            {VK_VOLUME_MUTE                       ,              KEY_Undefined},
            {VK_VOLUME_DOWN                       ,              KEY_Undefined},
            {VK_VOLUME_UP                         ,              KEY_Undefined},
            {VK_MEDIA_NEXT_TRACK                  ,              KEY_Undefined},
            {VK_MEDIA_PREV_TRACK                  ,              KEY_Undefined},
            {VK_MEDIA_STOP                        ,              KEY_Undefined},
            {VK_MEDIA_PLAY_PAUSE                  ,              KEY_Undefined},
            {VK_LAUNCH_MAIL                       ,              KEY_Undefined},
            {VK_LAUNCH_MEDIA_SELECT               ,              KEY_Undefined},
            {VK_LAUNCH_APP1                       ,              KEY_Undefined},
            {VK_LAUNCH_APP2                       ,              KEY_Undefined},
            {VK_OEM_1                             ,              KEY_Semicolon},    // ';:' for US
            {VK_OEM_PLUS                          ,              KEY_Equals     },    // '+' any country
            {VK_OEM_COMMA                         ,              KEY_Comma    },    // ',' any country
            {VK_OEM_MINUS                         ,              KEY_Minus    },    // '-' any country
            {VK_OEM_PERIOD                        ,              KEY_Period   },    // '.' any country
            {VK_OEM_2                             ,              KEY_Slash    },    // '/?' for US
            {VK_OEM_3                             ,              KEY_Tilde    },    // '`~' for US
            {VK_GAMEPAD_A                         ,              KEY_Undefined},
            {VK_GAMEPAD_B                         ,              KEY_Undefined},
            {VK_GAMEPAD_X                         ,              KEY_Undefined},
            {VK_GAMEPAD_Y                         ,              KEY_Undefined},
            {VK_GAMEPAD_RIGHT_SHOULDER            ,              KEY_Undefined},
            {VK_GAMEPAD_LEFT_SHOULDER             ,              KEY_Undefined},
            {VK_GAMEPAD_LEFT_TRIGGER              ,              KEY_Undefined},
            {VK_GAMEPAD_RIGHT_TRIGGER             ,              KEY_Undefined},
            {VK_GAMEPAD_DPAD_UP                   ,              KEY_Undefined},
            {VK_GAMEPAD_DPAD_DOWN                 ,              KEY_Undefined},
            {VK_GAMEPAD_DPAD_LEFT                 ,              KEY_Undefined},
            {VK_GAMEPAD_DPAD_RIGHT                ,              KEY_Undefined},
            {VK_GAMEPAD_MENU                      ,              KEY_Undefined},
            {VK_GAMEPAD_VIEW                      ,              KEY_Undefined},
            {VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON    ,              KEY_Undefined},
            {VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON   ,              KEY_Undefined},
            {VK_GAMEPAD_LEFT_THUMBSTICK_UP        ,              KEY_Undefined},
            {VK_GAMEPAD_LEFT_THUMBSTICK_DOWN      ,              KEY_Undefined},
            {VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT     ,              KEY_Undefined},
            {VK_GAMEPAD_LEFT_THUMBSTICK_LEFT      ,              KEY_Undefined},
            {VK_GAMEPAD_RIGHT_THUMBSTICK_UP       ,              KEY_Undefined},
            {VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN     ,              KEY_Undefined},
            {VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT    ,              KEY_Undefined},
            {VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT     ,              KEY_Undefined},
            {VK_OEM_4                             ,              KEY_Leftbracket},  //  '[{' for US
            {VK_OEM_5                             ,              KEY_Backslash},    //  '\|' for US
            {VK_OEM_6                             ,              KEY_Rightbracket}, //  ']}' for US
            {VK_OEM_7                             ,              KEY_Quote},        // ''"' for US
            {VK_OEM_8                             ,              KEY_Undefined},
            {VK_OEM_AX                            ,              KEY_Undefined},    //  'AX' key on Japanese AX kbd
            {VK_OEM_102                           ,              KEY_Undefined},    //  "<>" or "\|" on RT 102-key kbd.
            {VK_ICO_HELP                          ,              KEY_Undefined},    //  Help key on ICO
            {VK_ICO_00                            ,              KEY_Undefined},    //  00 key on ICO
            {VK_PROCESSKEY                        ,              KEY_Undefined},
            {VK_ICO_CLEAR                         ,              KEY_Undefined},
            {VK_PACKET                            ,              KEY_Undefined},
            {VK_OEM_RESET                         ,              KEY_Undefined},
            {VK_OEM_JUMP                          ,              KEY_Undefined},
            {VK_OEM_PA1                           ,              KEY_Undefined},
            {VK_OEM_PA2                           ,              KEY_Undefined},
            {VK_OEM_PA3                           ,              KEY_Undefined},
            {VK_OEM_WSCTRL                        ,              KEY_Undefined},
            {VK_OEM_CUSEL                         ,              KEY_Undefined},
            {VK_OEM_ATTN                          ,              KEY_Undefined},
            {VK_OEM_FINISH                        ,              KEY_Undefined},
            {VK_OEM_COPY                          ,              KEY_Undefined},
            {VK_OEM_AUTO                          ,              KEY_Undefined},
            {VK_OEM_ENLW                          ,              KEY_Undefined},
            {VK_OEM_BACKTAB                       ,              KEY_Undefined},
            {VK_ATTN                              ,              KEY_Undefined},
            {VK_CRSEL                             ,              KEY_Undefined},
            {VK_EXSEL                             ,              KEY_Undefined},
            {VK_EREOF                             ,              KEY_Undefined},
            {VK_PLAY                              ,              KEY_Undefined},
            {VK_ZOOM                              ,              KEY_Undefined},
            {VK_NONAME                            ,              KEY_Undefined},
            {VK_PA1                               ,              KEY_Undefined},
            {VK_OEM_CLEAR                         ,              KEY_Undefined}
        };
    // clang-format on
}

Win32_Window::Win32_Window(vsg::ref_ptr<WindowTraits> traits) :
    Inherit(traits),
    _window(nullptr)
{
    _keyboard = new KeyboardMap;

#ifdef UNICODE
    std::wstring windowClass;
    convert_utf(traits->windowClass, windowClass);
    std::wstring windowTitle;
    convert_utf(traits->windowTitle, windowTitle);
#else
    const auto& windowClass = traits->windowClass;
    const auto& windowTitle = traits->windowTitle;
#endif

    bool createWindow = true;

    if (traits->nativeWindow.has_value())
    {
        auto nativeWindow = std::any_cast<HWND>(traits->nativeWindow);
        if (nativeWindow)
        {
            _window = nativeWindow;
            createWindow = false;
        }
    }

    if (createWindow)
    {
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
        wc.lpszClassName = windowClass.c_str();
        wc.hIconSm = 0;

        if (::RegisterClassEx(&wc) == 0)
        {
            auto lastError = ::GetLastError();
            if (lastError != ERROR_CLASS_ALREADY_EXISTS) throw getLastErrorAsException("vsg::Win32_Window::Win32_Window(...) failed to create Window, could not register window class :");
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
        if (screenNum >= static_cast<int32_t>(displayDevices.size()))
            throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, screenNum is out of range.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

        DEVMODE deviceMode;
        deviceMode.dmSize = sizeof(deviceMode);
        deviceMode.dmDriverExtra = 0;

        if (!::EnumDisplaySettings(displayDevices[screenNum].DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode))
            throw getLastErrorAsException("vsg::Win32_Window::Win32_Window(...) failed to create Window, EnumDisplaySettings failed to fetch display settings : ");

        // setup window rect and style
        int32_t screenx = 0;
        int32_t screeny = 0;
        RECT windowRect;

        uint32_t windowStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        uint32_t extendedStyle = 0;

        if (!traits->fullscreen)
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
                if (!::AdjustWindowRectEx(&windowRect, windowStyle, FALSE, extendedStyle))
                    throw getLastErrorAsException("vsg::Win32_Window::Win32_Window(...) failed to create Window, AdjustWindowRectEx failed : ");
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
        _window = ::CreateWindowEx(extendedStyle, windowClass.c_str(), windowTitle.c_str(), windowStyle,
                                   windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
                                   NULL, NULL, ::GetModuleHandle(NULL), NULL);

        if (_window == nullptr) throw getLastErrorAsException("vsg::Win32_Window::Win32_Window(...) failed to create Window, CreateWindowEx did not return a valid window handle : ");

        // set window handle user data pointer to hold ref to this so we can retrieve in WindowsProc
        // this function lies about failing so we must jump through some hoops
        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowlongptra#return-value
        SetLastError(NOERROR);
        if (!SetWindowLongPtr(_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this)) && GetLastError() != NOERROR)
            throw getLastErrorAsException("vsg::Win32_Window::Win32_Window(...) SetWindowLongPtr(..) failed : ");

        // reposition once the window has been created to account for borders etc
        if (!::SetWindowPos(_window, nullptr, screenx, screeny, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 0))
            throw getLastErrorAsException("vsg::Win32_Window::Win32_Window(...) SetWindowPos(..) failed : ");

        traits->x = windowRect.left;
        traits->y = windowRect.top;
        traits->systemConnection = wc.hInstance;

        ShowWindow(_window, SW_SHOW);
        SetForegroundWindow(_window);
        if (!SetFocus(_window))
            throw getLastErrorAsException("vsg::Win32_Window::Win32_Window(...) SetFocus(..) failed : ");
    }

    // get client rect to find final width and height of the view
    RECT clientRect;
    if (!::GetClientRect(_window, &clientRect))
        throw getLastErrorAsException("vsg::Win32_Window::Win32_Window(...) GetClientRect(..) failed : ");

    uint32_t finalWidth = clientRect.right - clientRect.left;
    uint32_t finalHeight = clientRect.bottom - clientRect.top;

    if (traits->device)
    {
        share(traits->device);
    }

    _extent2D.width = finalWidth;
    _extent2D.height = finalHeight;

    // assign dimensions
    traits->width = finalWidth;
    traits->height = finalHeight;

    traits->nativeWindow = _window;

    _windowMapped = true;
}

Win32_Window::~Win32_Window()
{
    clear();

    if (_window != nullptr)
    {
        TCHAR className[MAX_PATH];
        GetClassName(_window, className, MAX_PATH);

        if (!::DestroyWindow(_window))
        {
            vsg::warn(getLastErrorAsException("Win32_Window::~Win32_Window() ::DestroyWindow(_window) failed : "));
        }

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
    return _window != 0 && _windowMapped;
}

void Win32_Window::releaseWindow()
{
    _window = {};
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
    if (GetClientRect(_window, &windowRect))
    {
        _extent2D.width = windowRect.right - windowRect.left;
        _extent2D.height = windowRect.bottom - windowRect.top;

        buildSwapchain();
    }
    else
    {
        vsg::warn("Win32_Window::resize() GetClientRect(..) call failed unable to rebuild swapchain.");
    }
}

bool Win32_Window::handleWin32Messages(UINT msg, WPARAM wParam, LPARAM lParam)
{
    vsg::clock::time_point event_time = vsg::clock::now();

    // get the current window rect
    auto getWindowExtents = [](HWND window, int32_t& x, int32_t& y, int32_t& width, int32_t& height) -> bool {
        RECT windowRect;
        if (GetClientRect(window, &windowRect) == 0) return false;

        x = windowRect.left;
        y = windowRect.top;
        width = windowRect.right - windowRect.left;
        height = windowRect.bottom - windowRect.top;
        return true;
    };

    switch (msg)
    {
    case WM_CLOSE:
        bufferedEvents.emplace_back(vsg::CloseWindowEvent::create(this, event_time));
        return true;
    case WM_SHOWWINDOW: {
        int32_t winx, winy, winw, winh;
        if (getWindowExtents(_window, winx, winy, winw, winh))
        {
            bufferedEvents.emplace_back(vsg::ExposeWindowEvent::create(this, event_time, winx, winy, winw, winh));
        }
        return true;
    }
    case WM_DESTROY:
        bufferedEvents.emplace_back(vsg::TerminateEvent::create(event_time));
        _windowMapped = false;
        return true;
    case WM_PAINT:
        ValidateRect(_window, NULL);
        return true;
    case WM_MOUSEMOVE: {
        int32_t mx = GET_X_LPARAM(lParam);
        int32_t my = GET_Y_LPARAM(lParam);

        bufferedEvents.emplace_back(vsg::MoveEvent::create(this, event_time, mx, my, getButtonMask(wParam)));
        return true;
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK: {
        int32_t mx = GET_X_LPARAM(lParam);
        int32_t my = GET_Y_LPARAM(lParam);

        bufferedEvents.emplace_back(vsg::ButtonPressEvent::create(this, event_time, mx, my, getButtonMask(wParam), getButtonDownEventDetail(msg, HIWORD(wParam))));
        //::SetCapture(_window);
        return true;
    }
    break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP: {
        int32_t mx = GET_X_LPARAM(lParam);
        int32_t my = GET_Y_LPARAM(lParam);

        bufferedEvents.emplace_back(vsg::ButtonReleaseEvent::create(this, event_time, mx, my, getButtonMask(wParam), getButtonUpEventDetail(msg, HIWORD(wParam))));
        //::ReleaseCapture(); // should only release once all mouse buttons are released ??
        return true;
    }
    break;
    case WM_MOUSEWHEEL: {
        bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(this, event_time, GET_WHEEL_DELTA_WPARAM(wParam) < 0 ? vec3(0.0f, -1.0f, 0.0f) : vec3(0.0f, 1.0f, 0.0f)));
        return true;
    }
    case WM_MOVE: {
        int32_t winx, winy, winw, winh;
        if (getWindowExtents(_window, winx, winy, winw, winh))
        {
            bufferedEvents.emplace_back(vsg::ConfigureWindowEvent::create(this, event_time, winx, winy, winw, winh));
        }
        return true;
    }
    case WM_SIZE: {
        int32_t winx, winy, winw, winh;
        if (getWindowExtents(_window, winx, winy, winw, winh))
        {
            if (wParam == SIZE_MINIMIZED || wParam == SIZE_MAXHIDE || winw == 0 || winh == 0)
            {
                _windowMapped = false;
            }
            else
            {
                _windowMapped = true;
                bufferedEvents.emplace_back(vsg::ConfigureWindowEvent::create(this, event_time, winx, winy, winw, winh));
            }
        }
        return true;
    }
    case WM_EXITSIZEMOVE:
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        vsg::KeySymbol keySymbol, modifiedKeySymbol;
        vsg::KeyModifier keyModifier;
        if (_keyboard->getKeySymbol(wParam, lParam, keySymbol, modifiedKeySymbol, keyModifier))
        {
            int32_t repeatCount = (lParam & 0xffff);
            bufferedEvents.emplace_back(vsg::KeyPressEvent::create(this, event_time, keySymbol, modifiedKeySymbol, keyModifier, repeatCount));
        }
        return true;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        vsg::KeySymbol keySymbol, modifiedKeySymbol;
        vsg::KeyModifier keyModifier;
        if (_keyboard->getKeySymbol(wParam, lParam, keySymbol, modifiedKeySymbol, keyModifier))
        {
            bufferedEvents.emplace_back(vsg::KeyReleaseEvent::create(this, event_time, keySymbol, modifiedKeySymbol, keyModifier, 0));
        }
        return true;
    }
    case WM_SETFOCUS: {
        bufferedEvents.emplace_back(vsg::FocusInEvent::create(this, event_time));
        return true;
    }
    case WM_KILLFOCUS: {
        bufferedEvents.emplace_back(vsg::FocusOutEvent::create(this, event_time));
        return true;
    }
    default:
        break;
    }
    return false;
}
