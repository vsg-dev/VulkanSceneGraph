/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield
Copyright (C) 2026 BlackBerry Limited

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/platform/qnx/QNX_Window.h>

#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/ui/DropEvent.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/ui/TouchEvent.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_screen.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <thread>

namespace vsg
{

// Provide the Window::create(...) implementation that automatically maps to a QNX_Window
ref_ptr<Window> Window::create(vsg::ref_ptr<WindowTraits> traits)
{
    return vsgQNX::QNX_Window::create(traits);
}

} // namespace vsg

namespace vsgQNX
{

// x = 0 -> 0; x = 1 -> 1 ...
int findMsb(int x)
{
    int msb = 0;

    if (x > 0)
    {
#if defined(__clang__) || defined(__GNUC__)
        msb = __INT_BITS__ - __builtin_clz(x);
#else
        while(x >>= 1)
        {
            ++msb;
        }
        ++msb;
#endif
    }
    return msb;
}

uint16_t getMask(int buttons)
{
    uint16_t mask = 0;
    if (buttons & SCREEN_LEFT_MOUSE_BUTTON) mask |= vsg::BUTTON_MASK_1;
    if (buttons & SCREEN_MIDDLE_MOUSE_BUTTON) mask |= vsg::BUTTON_MASK_2;
    if (buttons & SCREEN_RIGHT_MOUSE_BUTTON) mask |= vsg::BUTTON_MASK_3;
    return mask;
}

int64_t now_ms(void)
{
    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC, &res);
    return 1000 * res.tv_sec + res.tv_nsec / 1e6;
}

int64_t event_ms(screen_event_t event)
{
    long long ts;
    screen_get_event_property_llv(event, SCREEN_PROPERTY_TIMESTAMP, &ts);
    return static_cast<int64_t>(ts);
}

} // namespace vsgQNX

using namespace vsg;
using namespace vsgQNX;

KeyboardMap::KeyboardMap()
{
    _keycodeMap =
        {
            // Normal
            {KEYCODE_SPACE, KEY_Space},
            {KEYCODE_APOSTROPHE, KEY_Quotedbl},
            {KEYCODE_COMMA, KEY_Comma},
            {KEYCODE_MINUS, KEY_Minus},
            {KEYCODE_PERIOD, KEY_Period},
            {KEYCODE_SLASH, KEY_Slash},
            {KEYCODE_ZERO, KEY_0},
            {KEYCODE_ONE, KEY_1},
            {KEYCODE_TWO, KEY_2},
            {KEYCODE_THREE, KEY_3},
            {KEYCODE_FOUR, KEY_4},
            {KEYCODE_FIVE, KEY_5},
            {KEYCODE_SIX, KEY_6},
            {KEYCODE_SEVEN, KEY_7},
            {KEYCODE_EIGHT, KEY_8},
            {KEYCODE_NINE, KEY_9},
            {KEYCODE_SEMICOLON, KEY_Semicolon},
            {KEYCODE_EQUAL, KEY_Equals},
            {KEYCODE_LEFT_BRACKET, KEY_Leftbracket},
            {KEYCODE_BACK_SLASH, KEY_Backslash},
            {KEYCODE_RIGHT_BRACKET, KEY_Rightbracket},
            {KEYCODE_GRAVE, KEY_Backquote},
            {KEYCODE_A, KEY_a},
            {KEYCODE_B, KEY_b},
            {KEYCODE_C, KEY_c},
            {KEYCODE_D, KEY_d},
            {KEYCODE_E, KEY_e},
            {KEYCODE_F, KEY_f},
            {KEYCODE_G, KEY_g},
            {KEYCODE_H, KEY_h},
            {KEYCODE_I, KEY_i},
            {KEYCODE_J, KEY_j},
            {KEYCODE_K, KEY_k},
            {KEYCODE_L, KEY_l},
            {KEYCODE_M, KEY_m},
            {KEYCODE_N, KEY_n},
            {KEYCODE_O, KEY_o},
            {KEYCODE_P, KEY_p},
            {KEYCODE_Q, KEY_q},
            {KEYCODE_R, KEY_r},
            {KEYCODE_S, KEY_s},
            {KEYCODE_T, KEY_t},
            {KEYCODE_U, KEY_u},
            {KEYCODE_V, KEY_v},
            {KEYCODE_W, KEY_w},
            {KEYCODE_X, KEY_x},
            {KEYCODE_Y, KEY_y},
            {KEYCODE_Z, KEY_z},
            {KEYCODE_UP, KEY_Up},
            {KEYCODE_DOWN, KEY_Down},
            {KEYCODE_LEFT, KEY_Left},
            {KEYCODE_PG_UP, KEY_Page_Up},
            {KEYCODE_PG_DOWN, KEY_Page_Down},
            {KEYCODE_PRINT, KEY_Print},
            {KEYCODE_SCROLL_LOCK, KEY_Scroll_Lock},
            {KEYCODE_PAUSE, KEY_Pause},
            {KEYCODE_INSERT, KEY_Insert},
            {KEYCODE_HOME, KEY_Home},
            {KEYCODE_DELETE, KEY_Delete},
            {KEYCODE_END, KEY_End},
            {KEYCODE_NUM_LOCK, KEY_Num_Lock},
            {KEYCODE_RIGHT, KEY_Right},
            {KEYCODE_RETURN, KEY_Return},
            {KEYCODE_TAB, KEY_Tab},
            {KEYCODE_ESCAPE, KEY_Escape},
            {KEYCODE_LEFT_CTRL, KEY_Control_L},
            {KEYCODE_RIGHT_CTRL, KEY_Control_R},
            {KEYCODE_LEFT_SHIFT, KEY_Shift_L},
            {KEYCODE_RIGHT_SHIFT, KEY_Shift_R},
            {KEYCODE_LEFT_ALT, KEY_Alt_L},
            {KEYCODE_RIGHT_ALT, KEY_Alt_R},
            {KEYCODE_BACKSPACE, KEY_BackSpace},
            {KEYCODE_CAPS_LOCK, KEY_Caps_Lock},
            {KEYCODE_F1, KEY_F1},
            {KEYCODE_F2, KEY_F2},
            {KEYCODE_F3, KEY_F3},
            {KEYCODE_F4, KEY_F4},
            {KEYCODE_F5, KEY_F5},
            {KEYCODE_F6, KEY_F6},
            {KEYCODE_F7, KEY_F7},
            {KEYCODE_F8, KEY_F8},
            {KEYCODE_F9, KEY_F9},
            {KEYCODE_F10, KEY_F10},
            {KEYCODE_F11, KEY_F11},
            {KEYCODE_F12, KEY_F12},
            {KEYCODE_KP_DIVIDE, KEY_KP_Divide},
            {KEYCODE_KP_MULTIPLY, KEY_KP_Multiply},
            {KEYCODE_KP_MINUS, KEY_KP_Subtract},
            {KEYCODE_KP_PLUS, KEY_KP_Add},
            {KEYCODE_KP_ENTER, KEY_KP_Enter},
            {KEYCODE_KP_INSERT, KEY_KP_0},
            {KEYCODE_KP_END, KEY_KP_1},
            {KEYCODE_KP_DOWN, KEY_KP_2},
            {KEYCODE_KP_PG_DOWN, KEY_KP_3},
            {KEYCODE_KP_LEFT, KEY_KP_4},
            {KEYCODE_KP_FIVE, KEY_KP_5},
            {KEYCODE_KP_RIGHT, KEY_KP_6},
            {KEYCODE_KP_HOME, KEY_KP_7},
            {KEYCODE_KP_UP, KEY_KP_8},
            {KEYCODE_KP_PG_UP, KEY_KP_9},
            {KEYCODE_KP_DELETE, KEY_KP_Delete},
            {KEYCODE_PLAY, KEY_Execute},
            {KEYCODE_HELP, KEY_Help},
            {KEYCODE_MENU, KEY_Menu},
            {KEYCODE_AC_SELECT_ALL, KEY_Select},
            {KEYCODE_STOP, KEY_Cancel},
            {KEYCODE_AC_UNDO, KEY_Undo},
            {KEYCODE_AC_FIND, KEY_Find},
            {KEYCODE_SYSREQ, KEY_Sys_Req},
            {KEYCODE_AC_CANCEL, KEY_Cancel},
            {KEYCODE_AC_SEARCH, KEY_Find},
            {KEYCODE_AC_HOME, KEY_Home},
            {KEYCODE_AC_BACK, KEY_Prior},
            {KEYCODE_AC_FORWARD, KEY_Next},
            {KEYCODE_AC_STOP, KEY_Cancel},
            {KEYCODE_AC_REFRESH, KEY_Redo},
            // Shift
            {KEYCODE_LESS_THAN, KEY_Less},
            {KEYCODE_GREATER_THAN, KEY_Greater},
            {KEYCODE_QUESTION, KEY_Question},
            {KEYCODE_COLON, KEY_Colon},
            {KEYCODE_QUOTE, KEY_Quote},
            {KEYCODE_LEFT_BRACE, KEY_Leftcurlybracket},
            {KEYCODE_RIGHT_BRACE, KEY_Rightcurlybracket},
            {KEYCODE_BAR, KEY_Verticalslash},
            {KEYCODE_TILDE, KEY_Tilde},
            {KEYCODE_EXCLAM, KEY_Exclaim},
            {KEYCODE_AT, KEY_At},
            {KEYCODE_NUMBER, KEY_Hash},
            {KEYCODE_DOLLAR, KEY_Dollar},
            {KEYCODE_PERCENT, KEY_Percent},
            {KEYCODE_CIRCUMFLEX, KEY_Caret},
            {KEYCODE_AMPERSAND, KEY_Ampersand},
            {KEYCODE_ASTERISK, KEY_Asterisk},
            {KEYCODE_LEFT_PAREN, KEY_Leftparen},
            {KEYCODE_RIGHT_PAREN, KEY_Rightparen},
            {KEYCODE_UNDERSCORE, KEY_Underscore},
            {KEYCODE_PLUS, KEY_Plus},
            {KEYCODE_CAPITAL_A, KEY_A},
            {KEYCODE_CAPITAL_B, KEY_B},
            {KEYCODE_CAPITAL_C, KEY_C},
            {KEYCODE_CAPITAL_D, KEY_D},
            {KEYCODE_CAPITAL_E, KEY_E},
            {KEYCODE_CAPITAL_F, KEY_F},
            {KEYCODE_CAPITAL_G, KEY_G},
            {KEYCODE_CAPITAL_H, KEY_H},
            {KEYCODE_CAPITAL_I, KEY_I},
            {KEYCODE_CAPITAL_J, KEY_J},
            {KEYCODE_CAPITAL_K, KEY_K},
            {KEYCODE_CAPITAL_L, KEY_L},
            {KEYCODE_CAPITAL_M, KEY_M},
            {KEYCODE_CAPITAL_N, KEY_N},
            {KEYCODE_CAPITAL_O, KEY_O},
            {KEYCODE_CAPITAL_P, KEY_P},
            {KEYCODE_CAPITAL_Q, KEY_Q},
            {KEYCODE_CAPITAL_R, KEY_R},
            {KEYCODE_CAPITAL_S, KEY_S},
            {KEYCODE_CAPITAL_T, KEY_T},
            {KEYCODE_CAPITAL_U, KEY_U},
            {KEYCODE_CAPITAL_V, KEY_V},
            {KEYCODE_CAPITAL_W, KEY_W},
            {KEYCODE_CAPITAL_X, KEY_X},
            {KEYCODE_CAPITAL_Y, KEY_Y},
            {KEYCODE_CAPITAL_Z, KEY_Z},
        };

    _keymodMap =
        {
            {0, MODKEY_OFF},
            {KEYMOD_SCROLL_LOCK, MODKEY_OFF},
            {KEYMOD_SHIFT, MODKEY_Shift},
            {KEYMOD_SHIFT_LOCK, MODKEY_Shift},
            {KEYMOD_CAPS_LOCK, MODKEY_CapsLock},
            {KEYMOD_CTRL, MODKEY_Control},
            {KEYMOD_CTRL_LOCK, MODKEY_Control},
            {KEYMOD_ALT, MODKEY_Alt},
            {KEYMOD_ALTGR, MODKEY_Alt},
            {KEYMOD_ALT_LOCK, MODKEY_Alt},
            {KEYMOD_ALTGR_LOCK, MODKEY_Alt},
            {KEYMOD_NUM_LOCK, MODKEY_NumLock},
            // Lump everything remaining under MODKEY_Meta
            {KEYMOD_SHL3, MODKEY_Meta},
            {KEYMOD_SHL3_LOCK, MODKEY_Meta},
            {KEYMOD_MOD6, MODKEY_Meta},
            {KEYMOD_MOD6_LOCK, MODKEY_Meta},
            {KEYMOD_MOD7, MODKEY_Meta},
            {KEYMOD_MOD7_LOCK, MODKEY_Meta},
            {KEYMOD_MOD8, MODKEY_Meta},
            {KEYMOD_MOD8_LOCK, MODKEY_Meta},
        };
}

bool KeyboardMap::getKeySymbol(int keycap, int keysym, int keymod, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier)
{
    // The keycode mapping is not necessarily complete, but unassigned addresses
    // are initialized to 0, which maps to KEY_Undefined.
    if ((keycap < 0) || (static_cast<size_t>(keycap) >= _keycodeMap.size())
        || (keysym < 0) || (static_cast<size_t>(keysym) >= _keycodeMap.size()))
    {
        return false;
    }

    keySymbol = _keycodeMap[keycap];
    if (keySymbol == KEY_Undefined)
    {
        return false;
    }

    modifiedKeySymbol = _keycodeMap[keysym];
    if (modifiedKeySymbol == KEY_Undefined)
    {
        return false;
    }

    int msbIndex = vsgQNX::findMsb(keymod);

    keyModifier = MODKEY_OFF;
    for (int i = 0; i < msbIndex; ++i)
    {
        if (!!(keymod >> i) && (static_cast<size_t>(i) < _keymodMap.size()))
        {
            keyModifier = static_cast<vsg::KeyModifier>(keyModifier | _keymodMap[i]);
        }
    }

    return true;
}

QNX_Surface::QNX_Surface(vsg::Instance* instance, screen_context_t context, screen_window_t window) :
    vsg::Surface(VK_NULL_HANDLE, instance)
{
    VkScreenSurfaceCreateInfoQNX surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.context = context;
    surfaceCreateInfo.window = window;

    auto result = vkCreateScreenSurfaceQNX(*instance, &surfaceCreateInfo, _instance->getAllocationCallbacks(), &_surface);
    if (result != VK_SUCCESS)
    {
        throw Exception{"Failed to created QNX_Surface.", result};
    }
}

QNX_Window::QNX_Window(vsg::ref_ptr<WindowTraits> traits) :
    Inherit(traits)
{
    bool fullscreen = traits->fullscreen;

    // open connection
    bool openConnection = true;
    if (traits->systemConnection.has_value())
    {
        auto nativeConnection = std::any_cast<screen_context_t>(traits->systemConnection);
        if (nativeConnection)
        {
            _context = nativeConnection;
            openConnection = false;
        }
    }

    if (openConnection)
    {
        if (screen_create_context(&_context, 0) < 0)
        {
            throw Exception{"Failed to create Window, could not create screen context.", VK_ERROR_OUT_OF_HOST_MEMORY};
        }
    }

    if (screen_create_event(&_event) < 0)
    {
        throw Exception{"Failed to create Window, could not create screen event.", VK_ERROR_OUT_OF_HOST_MEMORY};
    }

    bool createWindow = true;

    if (traits->nativeWindow.has_value())
    {
        auto nativeWindow = std::any_cast<screen_window_t>(traits->nativeWindow);
        if (nativeWindow)
        {
            _window = nativeWindow;
            createWindow = false;
        }
    }

    if (createWindow)
    {
        // create window
        if (screen_create_window(&_window, _context) < 0)
        {
            throw Exception{"Failed to create Window, could not create screen window.", VK_ERROR_OUT_OF_HOST_MEMORY};
        }

        if (screen_get_window_property_pv(_window, SCREEN_PROPERTY_DISPLAY, (void **)&_display) < 0)
        {
            throw Exception{"Failed to create Window, could not retrieve screen display pointer.", VK_ERROR_INITIALIZATION_FAILED};
        }

        int size[2];
        int pos[2];
        if (fullscreen)
        {
            if (screen_get_display_property_iv(_display, SCREEN_PROPERTY_SIZE, size) < 0)
            {
                throw Exception{"Failed to create Window, could not retrieve screen display size.", VK_ERROR_INITIALIZATION_FAILED};
            }

            pos[0] = 0;
            pos[1] = 0;
        }
        else
        {
            size[0] = traits->width;
            size[1] = traits->height;
            pos[0] = traits->x;
            pos[1] = traits->y;
        }

        if (screen_set_window_property_iv(_window, SCREEN_PROPERTY_SIZE, size) < 0)
        {
            throw Exception{"Failed to create Window, could not set window size.", VK_ERROR_INITIALIZATION_FAILED};
        }

        if (screen_set_window_property_iv(_window, SCREEN_PROPERTY_POSITION, pos) < 0)
        {
            throw Exception{"Failed to create Window, could not set window position.", VK_ERROR_INITIALIZATION_FAILED};
        }

        int usage = SCREEN_USAGE_VULKAN;
        if (screen_set_window_property_iv(_window, SCREEN_PROPERTY_USAGE, &usage) < 0)
        {
            throw Exception{"Failed to create Window, could not set window usage.", VK_ERROR_INITIALIZATION_FAILED};
        }

        if (screen_create_window_buffers(_window, 2) < 0)
        {
            throw Exception{"Failed to create Window, could not set window usage.", VK_ERROR_OUT_OF_DEVICE_MEMORY};
        }

        if (screen_flush_context(_context, 0) < 0)
        {
            throw Exception{"Failed to create Window, error when flushing context.", VK_ERROR_UNKNOWN};
        }

        _first_qnx_timestamp = now_ms();
        _first_qnx_time_point = vsg::clock::now();
    }
    else
    {
        if (screen_get_window_property_pv(_window, SCREEN_PROPERTY_DISPLAY, (void **)&_display) < 0)
        {
            throw Exception{"Failed to create Window, could not retrieve screen display pointer.", VK_ERROR_INITIALIZATION_FAILED};
        }

        _first_qnx_timestamp = now_ms();
        _first_qnx_time_point = vsg::clock::now();
    }
    _windowVisible = true;
    _windowInFocus = false; // Let pollEvents send the first focus enter event.
    _pointerPosPrev[0] = 0;
    _pointerPosPrev[1] = 0;
    _buttonsPressedPrev = 0;

    _keyboard = new KeyboardMap;

    if (traits->device)
    {
        share(traits->device);
    }

    int final_size[2];
    if (screen_get_window_property_iv(_window, SCREEN_PROPERTY_SIZE, final_size) < 0)
    {
        throw Exception{"Failed to create Window, could not get window size.", VK_ERROR_INITIALIZATION_FAILED};
    }

    int final_pos[2];
    if (screen_get_window_property_iv(_window, SCREEN_PROPERTY_POSITION, final_pos) < 0)
    {
        throw Exception{"Failed to create Window, could not get window position.", VK_ERROR_INITIALIZATION_FAILED};
    }

    _extent2D.width = final_size[0];
    _extent2D.height = final_size[1];

    traits->x = final_pos[0];
    traits->y = final_pos[1];
    traits->width = final_size[0];
    traits->height = final_size[1];

    traits->nativeWindow = _window;
    traits->systemConnection = _context;
}

QNX_Window::~QNX_Window()
{
    clear();

    if (_window != nullptr)
    {
        screen_destroy_window(_window);
    }
    if (_event != nullptr)
    {
        screen_destroy_event(_event);
    }
    if (_context != nullptr)
    {
        screen_destroy_context(_context);
    }
}

void QNX_Window::_initSurface()
{
    if (!_instance) _initInstance();

    _surface = new QNX_Surface(_instance, _context, _window);
}

bool QNX_Window::pollEvents(vsg::UIEvents& events)
{
    int windowInFocus_i;
    bool windowInFocus;
    if (screen_get_window_property_iv(_window, SCREEN_PROPERTY_FOCUS, &windowInFocus_i) == 0)
    {
        windowInFocus = static_cast<bool>(windowInFocus_i);

        if (_windowInFocus != windowInFocus)
        {
            if (windowInFocus)
            {
                vsg::clock::time_point event_time = vsg::clock::now();
                bufferedEvents.emplace_back(vsg::FocusInEvent::create(this, event_time));
            }
            else
            {
                vsg::clock::time_point event_time = vsg::clock::now();
                bufferedEvents.emplace_back(vsg::FocusOutEvent::create(this, event_time));
            }

            _windowInFocus = windowInFocus;
        }
    }

    for (;;)
    {
        if (screen_get_event(_context, _event, 0) < 0)
        {
            break;
        }

        int type;
        if (screen_get_event_property_iv(_event, SCREEN_PROPERTY_TYPE, &type) < 0)
        {
            break;
        }

        if (type == SCREEN_EVENT_NONE)
        {
            break;
        }

        vsg::clock::time_point event_time = _first_qnx_time_point + std::chrono::milliseconds(event_ms(_event) - _first_qnx_timestamp);
        switch (type)
        {
            case SCREEN_EVENT_KEYBOARD:
                {
                    vsg::KeySymbol keySymbol;
                    vsg::KeySymbol keySymbolModified;
                    vsg::KeyModifier keyModifier;
                    int keycap;
                    int keysym;
                    int keymod;
                    int val;

                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_KEY_CAP, &keycap);
                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_SYM, &keysym);
                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_MODIFIERS, &keymod);

                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_FLAGS, &val);

                    if (val & KEY_DOWN)
                    {
                        if (_keyboard->getKeySymbol(keycap, keysym, keymod, keySymbol, keySymbolModified, keyModifier))
                        {
                            bufferedEvents.emplace_back(vsg::KeyPressEvent::create(this, event_time, keySymbol, keySymbolModified, keyModifier, 0));
                        }
                    }
                    else
                    {
                        if (_keyboard->getKeySymbol(keycap, keysym, keymod, keySymbol, keySymbolModified, keyModifier))
                        {
                            bufferedEvents.emplace_back(vsg::KeyReleaseEvent::create(this, event_time, keySymbol, keySymbolModified, keyModifier, 0));
                        }
                    }
                }

                break;

            case SCREEN_EVENT_POINTER:
                {
                    int pos[2] = {0,0};
                    int buttons;
                    int mouse_wheel;
                    int mouse_h_wheel;

                    int16_t buttonMask = vsgQNX::getMask(_buttonsPressedPrev);

                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_POSITION, pos);
                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_BUTTONS, &buttons);
                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_MOUSE_WHEEL, &mouse_wheel);
                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_MOUSE_HORIZONTAL_WHEEL, &mouse_h_wheel);

                    if (!((pos[0] == _pointerPosPrev[0]) && (pos[1] == _pointerPosPrev[1])))
                    {
                        bufferedEvents.emplace_back(vsg::MoveEvent::create(this, event_time, pos[0], pos[1], vsg::ButtonMask(buttonMask)));
                        _pointerPosPrev[0] = pos[0];
                        _pointerPosPrev[1] = pos[1];
                    }

                    if (buttons != _buttonsPressedPrev)
                    {
                        // Apply transformations to the mask as individual events.
                        for (int i = 0; i < 3; ++i)
                        {
                            int buttonVal = 1 << i;
                            if (!!((buttons ^ _buttonsPressedPrev) & buttonVal))
                            {
                                if (!!(buttons & buttonVal))
                                {
                                    buttonMask |= vsgQNX::getMask(buttonVal);
                                    bufferedEvents.emplace_back(vsg::ButtonPressEvent::create(this, event_time, pos[0], pos[1], vsg::ButtonMask(buttonMask), i+1));
                                }
                                else
                                {
                                    buttonMask &= ~vsgQNX::getMask(buttonVal);
                                    bufferedEvents.emplace_back(vsg::ButtonReleaseEvent::create(this, event_time, pos[0], pos[1], vsg::ButtonMask(buttonMask), i+1));
                                }
                            }
                        }
                        _buttonsPressedPrev = buttons;
                    }

                    if (!!mouse_wheel || !!mouse_h_wheel)
                    {
                        bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(this, event_time, vsg::vec3((float) mouse_h_wheel, (float) mouse_wheel, 0.0f)));
                    }
                }

                break;

            case SCREEN_EVENT_MTOUCH_TOUCH:
                {
                    int pos[2] = {0,0};

                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_POSITION, pos);

                    bufferedEvents.emplace_back(vsg::TouchDownEvent::create(this, event_time, pos[0], pos[0], 0));
                }

                break;

            case SCREEN_EVENT_MTOUCH_MOVE:
                {
                    int pos[2] = {0,0};

                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_POSITION, pos);

                    bufferedEvents.emplace_back(vsg::TouchMoveEvent::create(this, event_time, pos[0], pos[1], 0));
                }

                break;

            case SCREEN_EVENT_MTOUCH_RELEASE:
                {
                    int pos[2] = {0,0};

                    screen_get_event_property_iv(_event, SCREEN_PROPERTY_POSITION, pos);

                    bufferedEvents.emplace_back(vsg::TouchUpEvent::create(this, event_time, pos[0], pos[1], 0));
                }

                break;

            default:
                break;
        }
    }
    return Window::pollEvents(events);
}

bool QNX_Window::valid() const
{
    return _window != nullptr;
}

bool QNX_Window::visible() const
{
    return (_window != nullptr) && _windowVisible;
}

void QNX_Window::releaseWindow()
{
    _window = {};
}

void QNX_Window::releaseConnection()
{
    _context = {};
}

void QNX_Window::resize()
{
    int final_size[2];
    if (screen_get_window_property_iv(_window, SCREEN_PROPERTY_SIZE, final_size) == 0)
    {
        _extent2D.width = final_size[0];
        _extent2D.height = final_size[1];

        buildSwapchain();
    }
    else
    {
        vsg::warn("Failed to get window dimensions, error: ", errno);
    }
}
