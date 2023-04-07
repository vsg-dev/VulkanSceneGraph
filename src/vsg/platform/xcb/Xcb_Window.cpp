/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/platform/xcb/Xcb_Window.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/vk/Extensions.h>

#include <xcb/xproto.h>

#include <chrono>
#include <cstring>
#include <thread>

namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to a Xcb_Window
    ref_ptr<Window> Window::create(vsg::ref_ptr<WindowTraits> traits)
    {
        return vsgXcb::Xcb_Window::create(traits);
    }
} // namespace vsg

namespace vsgXcb
{
    // window decoration
    struct MotifHints
    {
        enum Flags : uint32_t
        {
            FLAGS_FUNCTIONS = 1 << 0,
            FLAGS_DECORATIONS = 1 << 1,
            FLAGS_INPUT_MODE = 1 << 2,
            FLAGS_STATUS = 1 << 3,
        };

        enum Functions : uint32_t
        {
            FUNC_ALL = 1 << 0,
            FUNC_RESIZE = 1 << 1,
            FUNC_MOVE = 1 << 2,
            FUNC_MINIMUMSIZE = 1 << 3,
            FUNC_MAXIMUMSIZE = 1 << 4,
            FUNC_CLOSE = 1 << 5
        };

        enum Decorations : uint32_t
        {
            DECOR_ALL = 1 << 0,
            DECOR_BORDER = 1 << 1,
            DECOR_RESIZE = 1 << 2,
            DECOR_TITLE = 1 << 3,
            DECOR_MENU = 1 << 4,
            DECOR_MINIMUMSIZE = 1 << 5,
            DECOR_MAXIMUMSIZE = 1 << 6
        };

        static MotifHints borderless()
        {
            MotifHints hints;
            hints.flags = FLAGS_DECORATIONS;
            return hints;
        }

        static MotifHints window(bool resize = true, bool move = true, bool close = true, bool minimize = true, bool maximize = true)
        {
            MotifHints hints;
            hints.flags = FLAGS_DECORATIONS | FLAGS_FUNCTIONS;
            hints.functions = 0;
            if (resize) hints.functions |= FUNC_RESIZE;
            if (move) hints.functions |= FUNC_MOVE;
            if (close) hints.functions |= FUNC_CLOSE;
            if (minimize) hints.functions |= FUNC_MINIMUMSIZE;
            if (maximize) hints.functions |= FUNC_MAXIMUMSIZE;
            hints.decorations = DECOR_ALL;
            return hints;
        }

        uint32_t flags{};
        uint32_t functions{};
        uint32_t decorations{};
        int32_t input_mode{};
        uint32_t status{};
    };

    class AtomRequest
    {
    public:
        AtomRequest(xcb_connection_t* connection, const char* atom_name) :
            _connection(connection)
        {
            _cookie = xcb_intern_atom(connection, false, strlen(atom_name), atom_name);
        }

        operator xcb_atom_t()
        {
            if (_connection)
            {
                xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(_connection, _cookie, nullptr);
                if (reply)
                {
                    _atom = reply->atom;
                    free(reply);
                }
                _connection = nullptr;
            }
            return _atom;
        }

        xcb_connection_t* _connection{};
        xcb_intern_atom_cookie_t _cookie{};
        xcb_atom_t _atom{};
    };

    bool getWindowGeometry(xcb_connection_t* connection, xcb_drawable_t window, int32_t& x, int32_t& y, uint32_t& width, uint32_t& height)
    {
        xcb_get_geometry_cookie_t geometry_cookie = xcb_get_geometry(connection, window);
        xcb_query_tree_cookie_t tree_cookie = xcb_query_tree(connection, window);

        xcb_get_geometry_reply_t* geometry_reply = xcb_get_geometry_reply(connection, geometry_cookie, nullptr);
        xcb_query_tree_reply_t* tree_reply = xcb_query_tree_reply(connection, tree_cookie, nullptr);

        if (geometry_reply)
        {
            x = geometry_reply->x;
            y = geometry_reply->y;
            width = geometry_reply->width;
            height = geometry_reply->height;

            if (tree_reply)
            {
                xcb_translate_coordinates_cookie_t trans_cookie = xcb_translate_coordinates(connection, window, geometry_reply->root, x, y);
                xcb_translate_coordinates_reply_t* trans_reply = xcb_translate_coordinates_reply(connection, trans_cookie, nullptr);
                if (trans_reply)
                {
                    x = trans_reply->dst_x;
                    y = trans_reply->dst_y;
                    free(trans_reply);
                }

                free(tree_reply);
            }

            free(geometry_reply);
            return true;
        }
        return false;
    }

} // namespace vsgXcb

using namespace vsg;
using namespace vsgXcb;

///////////////////////////////////////////////////////////////////////////////////////
//
// KeyboardMap
//
KeyboardMap::KeyboardMap()
{
    _modifierMask = 0xff;
}

vsg::KeySymbol KeyboardMap::getKeySymbol(uint16_t keycode, uint16_t modifier)
{
    auto itr = _keycodeMap.find(KeycodeModifier(keycode, 0));
    if (itr == _keycodeMap.end()) return vsg::KEY_Undefined;

    vsg::KeySymbol baseKeySymbol = itr->second;
    if (modifier == 0) return baseKeySymbol;

    bool shift = (modifier & vsg::MODKEY_Shift) != 0;
    uint16_t index = 0;

    if (baseKeySymbol >= vsg::KEY_KP_Space && baseKeySymbol <= vsg::KEY_KP_Divide)
    {
        // numeric keypad values
        bool numLock = ((modifier & vsg::MODKEY_NumLock) != 0);
        index = (numLock && !shift) ? 1 : 0;
    }
    else
    {
        bool capsLock = (modifier & vsg::MODKEY_CapsLock) != 0;
        index = (capsLock ? !shift : shift) ? 1 : 0;
    }
    if (index == 0) return baseKeySymbol;

    if (itr = _keycodeMap.find(KeycodeModifier(keycode, index)); itr != _keycodeMap.end()) return itr->second;
    return vsg::KEY_Undefined;
}

vsg::KeyModifier KeyboardMap::getKeyModifier(vsg::KeySymbol keySym, uint16_t modifier, bool pressed)
{
// values from keysymbdefs.h
#define XK_Shift_L 0xffe1
#define XK_Shift_R 0xffe2
#define XK_Control_L 0xffe3
#define XK_Control_R 0xffe4
#define XK_Caps_Lock 0xffe5
#define XK_Shift_Lock 0xffe6

#define XK_Meta_L 0xffe7
#define XK_Meta_R 0xffe8
#define XK_Alt_L 0xffe9
#define XK_Alt_R 0xffea
#define XK_Super_L 0xffeb
#define XK_Super_R 0xffec
#define XK_Hyper_L 0xffed
#define XK_Hyper_R 0xffee

    if (keySym >= XK_Shift_L && keySym <= XK_Hyper_R)
    {
        uint16_t mask = 0;
        switch (keySym)
        {
        case (XK_Shift_L):
        case (XK_Shift_R): mask = XCB_KEY_BUT_MASK_SHIFT; break;
        case (XK_Control_L):
        case (XK_Control_R): mask = XCB_KEY_BUT_MASK_CONTROL; break;
        case (XK_Alt_L):
        case (XK_Alt_R): mask = XCB_KEY_BUT_MASK_MOD_1; break;
        case (XK_Meta_L):
        case (XK_Meta_R): mask = XCB_KEY_BUT_MASK_MOD_2; break;
        case (XK_Super_L):
        case (XK_Super_R): mask = XCB_KEY_BUT_MASK_MOD_4; break;
        case (XK_Hyper_L):
        case (XK_Hyper_R): mask = XCB_KEY_BUT_MASK_MOD_3; break;
        default: break;
        }
        if (pressed)
            modifier = (modifier | mask);
        else
            modifier = (modifier & ~mask);
    }

    return vsg::KeyModifier(modifier);
}

void KeyboardMap::add(uint16_t keycode, uint16_t modifier, KeySymbol key)
{
    _keycodeMap[KeycodeModifier(keycode, modifier)] = key;
}

void KeyboardMap::add(uint16_t keycode, std::initializer_list<std::pair<uint16_t, KeySymbol>> combinations)
{
    for (auto [modifier, key] : combinations)
    {
        add(keycode, modifier, key);
    }
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Xcb_Surface
//
Xcb_Surface::Xcb_Surface(vsg::Instance* instance, xcb_connection_t* connection, xcb_window_t window) :
    vsg::Surface(VK_NULL_HANDLE, instance)
{
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.connection = connection;
    surfaceCreateInfo.window = window;

    auto result = vkCreateXcbSurfaceKHR(*instance, &surfaceCreateInfo, _instance->getAllocationCallbacks(), &_surface);
    if (result != VK_SUCCESS)
    {
        throw Exception{"Failed to created Xcb_Surface.", result};
    }
}

///////////////////////////////////////////////////////////////////////////////////////
//
// Xcb_Window
//
Xcb_Window::Xcb_Window(vsg::ref_ptr<WindowTraits> traits) :
    Inherit(traits)
{
    bool fullscreen = traits->fullscreen;
    uint32_t override_redirect = traits->overrideRedirect;

    // open connection
    bool openConnection = true;
    if (traits->systemConnection.has_value())
    {
        auto nativeConnection = std::any_cast<xcb_connection_t*>(traits->systemConnection);
        if (nativeConnection)
        {
            _connection = nativeConnection;
            openConnection = false;
        }
    }

    int screenNum = 0;
    if (openConnection)
    {
        if (traits->display.empty())
        {
            _connection = xcb_connect(NULL, &screenNum);
        }
        else
        {
            _connection = xcb_connect(traits->display.c_str(), &screenNum);
        }
    }

    if (xcb_connection_has_error(_connection))
    {
        // close connection
        if (openConnection) xcb_disconnect(_connection);

        throw Exception{"Failed to created Window, unable able to establish xcb connection.", VK_ERROR_INVALID_EXTERNAL_HANDLE};
    }

    // get the screen
    const xcb_setup_t* setup = xcb_get_setup(_connection);

    _keyboard = new KeyboardMap;

    {
        xcb_keycode_t min_keycode = setup->min_keycode;
        xcb_keycode_t max_keycode = setup->max_keycode;

        xcb_get_keyboard_mapping_cookie_t keyboard_mapping_cookie = xcb_get_keyboard_mapping(_connection, min_keycode, (max_keycode - min_keycode) + 1);
        xcb_get_keyboard_mapping_reply_t* keyboard_mapping_reply = xcb_get_keyboard_mapping_reply(_connection, keyboard_mapping_cookie, nullptr);

        const xcb_keysym_t* keysyms = xcb_get_keyboard_mapping_keysyms(keyboard_mapping_reply);
        int length = xcb_get_keyboard_mapping_keysyms_length(keyboard_mapping_reply);

        int keysyms_per_keycode = keyboard_mapping_reply->keysyms_per_keycode;
        for (int i = 0; i < length; i += keysyms_per_keycode)
        {
            const xcb_keysym_t* keysym = &keysyms[i];
            xcb_keycode_t keycode = min_keycode + i / keysyms_per_keycode;
            for (int m = 0; m < keysyms_per_keycode; ++m)
            {
                if (keysym[m] != 0) _keyboard->add(keycode, m, static_cast<KeySymbol>(keysym[m]));
            }
        }

        free(keyboard_mapping_reply);
    }

    // select the appropriate screen for the window
    if (traits->screenNum >= 0) screenNum = traits->screenNum;

    int screenCount = xcb_setup_roots_length(setup);
    if (screenNum >= screenCount)
    {
        warn("request screenNum (", screenNum, ") too high, only ", screenCount, " screens available. Selecting screen 0 as fallback.");
        screenNum = 0;
    }

    xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(setup);

    for (int i = 0; i < screenNum; ++i)
    {
        xcb_screen_next(&screen_iterator);
    }

    _screen = screen_iterator.data;

    bool createWindow = true;

    if (traits->nativeWindow.has_value())
    {
        auto nativeWindow = std::any_cast<xcb_window_t>(traits->nativeWindow);
        if (nativeWindow)
        {
            _window = nativeWindow;
            createWindow = false;
        }
    }

    if (createWindow)
    {
        uint8_t depth = XCB_COPY_FROM_PARENT;
        xcb_window_t parent = _screen->root;
        uint16_t border_width = 0;
        uint16_t window_class = XCB_WINDOW_CLASS_INPUT_OUTPUT;
        xcb_visualid_t visual = XCB_COPY_FROM_PARENT;
        uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_BIT_GRAVITY | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
        uint32_t event_mask = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                              XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
                              XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
                              XCB_EVENT_MASK_FOCUS_CHANGE |
                              XCB_EVENT_MASK_PROPERTY_CHANGE;
        uint32_t value_list[] =
            {
                _screen->black_pixel,
                XCB_GRAVITY_NORTH_WEST,
                override_redirect,
                event_mask};

        // generate the window id
        _window = xcb_generate_id(_connection);

        // create window
        if (fullscreen)
        {
            xcb_create_window(_connection, depth, _window, parent,
                              0, 0, _screen->width_in_pixels, _screen->height_in_pixels,
                              border_width,
                              window_class,
                              visual,
                              value_mask,
                              value_list);
        }
        else
        {
            xcb_create_window(_connection, depth, _window, parent,
                              traits->x, traits->y, traits->width, traits->height,
                              border_width,
                              window_class,
                              visual,
                              value_mask,
                              value_list);
        }

        // set class of window to enable window manager configuration with rules for positioning
        xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 8, traits->windowClass.size(), traits->windowClass.data());

        // set title of window
        xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, traits->windowTitle.size(), traits->windowTitle.data());

        // make requests for the atoms
        AtomRequest protocols(_connection, "WM_PROTOCOLS");
        AtomRequest deleteWindow(_connection, "WM_DELETE_WINDOW");

        // get the atoms request replies
        _wmProtocols = protocols;
        _wmDeleteWindow = deleteWindow;
        xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, _wmProtocols, 4, 32, 1, &_wmDeleteWindow);

        if (fullscreen)
        {
            AtomRequest wmState(_connection, "_NET_WM_STATE");
            AtomRequest wmFullScreen(_connection, "_NET_WM_STATE_FULLSCREEN");
            std::vector<xcb_atom_t> stateAtoms{wmFullScreen};

            xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, wmState, XCB_ATOM_ATOM, 32, stateAtoms.size(), stateAtoms.data());
        }

        // set whether the window should have a border or not, and if so what resize/move/close functions to enable
        AtomRequest motifHintAtom(_connection, "_MOTIF_WM_HINTS");
        MotifHints hints = (fullscreen || !_traits->decoration) ? MotifHints::borderless() : MotifHints::window();
        xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, motifHintAtom, motifHintAtom, 32, 5, &hints);

        // work out the X server timestamp by checking for the property notify events that result for the above xcb_change_property calls.
        _first_xcb_timestamp = 0;
        _first_xcb_time_point = vsg::clock::now();
        {
            xcb_generic_event_t* event = nullptr;
            while (_first_xcb_timestamp == 0 && (event = xcb_wait_for_event(_connection)))
            {
                uint8_t response_type = event->response_type & ~0x80;
                if (response_type == XCB_PROPERTY_NOTIFY)
                {
                    auto propety_notify = reinterpret_cast<const xcb_property_notify_event_t*>(event);
                    _first_xcb_timestamp = propety_notify->time;
                    _first_xcb_time_point = vsg::clock::now();
                }
                free(event);
            }
        }
        xcb_map_window(_connection, _window);
        _windowMapped = true;
    }
    else
    {
        _windowMapped = true;
        _first_xcb_time_point = vsg::clock::now();
    }

    if (traits->shareWindow)
    {
        // share the _instance, _physicalDevice and _device;
        share(*traits->shareWindow);
    }

    xcb_flush(_connection);

    // sleep to give the window manage time to do any repositing and resizing
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // get the dimensions of the final window.
    xcb_get_geometry_reply_t* geometry_reply = xcb_get_geometry_reply(_connection, xcb_get_geometry(_connection, _window), nullptr);
    if (geometry_reply)
    {
        _extent2D.width = geometry_reply->width;
        _extent2D.height = geometry_reply->height;

        // assign dimensions
        traits->x = geometry_reply->x;
        traits->y = geometry_reply->y;
        traits->width = geometry_reply->width;
        traits->height = geometry_reply->height;

        free(geometry_reply);
    }

    traits->nativeWindow = _window;
    traits->systemConnection = _connection;
}

Xcb_Window::~Xcb_Window()
{
    // detach Vulkan objects
    clear();

    if (_connection != nullptr)
    {
        if (_window != 0)
        {
            xcb_destroy_window(_connection, _window);
        }

        xcb_flush(_connection);
        xcb_disconnect(_connection);
    }
}

void Xcb_Window::_initSurface()
{
    if (!_instance) _initInstance();

    _surface = new Xcb_Surface(_instance, _connection, _window);
}

bool Xcb_Window::valid() const
{
    return _window != 0;
}

bool Xcb_Window::visible() const
{
    return _window != 0 && _windowMapped;
}

void Xcb_Window::releaseWindow()
{
    _window = {};
}

void Xcb_Window::releaseConnection()
{
    _connection = {};
}

namespace
{
    // For the moment we don't want the modifier keys in pointer events.
    uint16_t maskButtons(uint16_t state)
    {
        constexpr const uint16_t buttonMask =
            XCB_BUTTON_MASK_1 |
            XCB_BUTTON_MASK_2 |
            XCB_BUTTON_MASK_3 |
            XCB_BUTTON_MASK_4 |
            XCB_BUTTON_MASK_5;
        return state & buttonMask;
    }
}

bool Xcb_Window::pollEvents(UIEvents& events)
{
    xcb_generic_event_t* event;
    int i = 0;
    while ((event = xcb_poll_for_event(_connection)))
    {
        ++i;
        uint8_t response_type = event->response_type & ~0x80;
        switch (response_type)
        {
        case (XCB_DESTROY_NOTIFY): {
            vsg::clock::time_point event_time = vsg::clock::now();
            bufferedEvents.emplace_back(vsg::TerminateEvent::create(event_time));
            break;
        }
        case (XCB_UNMAP_NOTIFY): {
            //debug("xcb_unmap_notify_event_t");
            _windowMapped = false;
            break;
        }
        case (XCB_MAP_NOTIFY): {
            //debug("xcb_map_notify_event_t");
            _windowMapped = true;
            break;
        }
        case (XCB_MAPPING_NOTIFY): {
            //debug("xcb_mapping_notify_event_t");
            break;
        }
        case (XCB_LIST_PROPERTIES): {
            //debug("xcb_list_properties_request_t");
            break;
        }
        case (XCB_PROPERTY_NOTIFY): {
            //debug("xcb_property_notify_event_t");
            break;
        }
        case (XCB_FOCUS_IN): {
            //debug("xcb_focus_in_event_t");
            break;
        }
        case (XCB_FOCUS_OUT): {
            //debug("xcb_focus_out_event_t");
            break;
        }
        case (XCB_ENTER_NOTIFY): {
            //debug("xcb_enter_notify_event_t");
            break;
        }
        case (XCB_LEAVE_NOTIFY): {
            //debug("xcb_leave_notify_event_t");
            break;
        }
        case (XCB_CONFIGURE_NOTIFY): {
            auto configure = reinterpret_cast<const xcb_configure_notify_event_t*>(event);

            // Xcb configure events x,y values can be behave differently on different window managers so use getWindowGeometry(..) to avoid inconsistencies
            int32_t x = configure->x;
            int32_t y = configure->y;
            uint32_t width = configure->width;
            uint32_t height = configure->height;
            vsgXcb::getWindowGeometry(_connection, _window, x, y, width, height);

            bool previousConfigureEventsIsEqual = false;
            for (auto previousEvent : events)
            {
                vsg::ConfigureWindowEvent* cwe = dynamic_cast<vsg::ConfigureWindowEvent*>(previousEvent.get());
                if (cwe)
                {
                    previousConfigureEventsIsEqual = (cwe->x == x) && (cwe->y == y) && (cwe->width == width) && (cwe->height == height);
                }
            }

            if (!previousConfigureEventsIsEqual)
            {
                vsg::clock::time_point event_time = vsg::clock::now();
                bufferedEvents.emplace_back(vsg::ConfigureWindowEvent::create(this, event_time, x, y, width, height));
                _extent2D.width = width;
                _extent2D.height = height;
            }

            break;
        }
        case (XCB_EXPOSE): {
            auto expose = reinterpret_cast<const xcb_expose_event_t*>(event);

            vsg::clock::time_point event_time = vsg::clock::now();
            bufferedEvents.emplace_back(vsg::ExposeWindowEvent::create(this, event_time, expose->x, expose->y, expose->width, expose->height));

            break;
        }
        case XCB_CLIENT_MESSAGE: {
            auto client_message = reinterpret_cast<const xcb_client_message_event_t*>(event);

            if (client_message->data.data32[0] == _wmDeleteWindow)
            {
                vsg::clock::time_point event_time = vsg::clock::now();
                bufferedEvents.emplace_back(vsg::CloseWindowEvent::create(this, event_time));
            }
            break;
        }
        case XCB_KEY_PRESS: {
            auto key_press = reinterpret_cast<const xcb_key_press_event_t*>(event);

            vsg::clock::time_point event_time = _first_xcb_time_point + std::chrono::milliseconds(key_press->time - _first_xcb_timestamp);
            vsg::KeySymbol keySymbol = _keyboard->getKeySymbol(key_press->detail, 0);
            vsg::KeySymbol keySymbolModified = _keyboard->getKeySymbol(key_press->detail, key_press->state);
            vsg::KeyModifier keyModifier = _keyboard->getKeyModifier(keySymbol, key_press->state, true);

            bufferedEvents.emplace_back(vsg::KeyPressEvent::create(this, event_time, keySymbol, keySymbolModified, keyModifier, 0));

            break;
        }
        case XCB_KEY_RELEASE: {
            auto key_release = reinterpret_cast<const xcb_key_release_event_t*>(event);

            vsg::clock::time_point event_time = _first_xcb_time_point + std::chrono::milliseconds(key_release->time - _first_xcb_timestamp);
            vsg::KeySymbol keySymbol = _keyboard->getKeySymbol(key_release->detail, 0);
            vsg::KeySymbol keySymbolModified = _keyboard->getKeySymbol(key_release->detail, key_release->state);
            vsg::KeyModifier keyModifier = _keyboard->getKeyModifier(keySymbol, key_release->state, false);

            bufferedEvents.emplace_back(vsg::KeyReleaseEvent::create(this, event_time, keySymbol, keySymbolModified, keyModifier, 0));

            break;
        }
        case (XCB_BUTTON_PRESS): {
            auto button_press = reinterpret_cast<const xcb_button_press_event_t*>(event);

            vsg::clock::time_point event_time = _first_xcb_time_point + std::chrono::milliseconds(button_press->time - _first_xcb_timestamp);

            if (button_press->same_screen)
            {
                // X11/Xvb treat scroll wheel up/down as button 4 and 5 so handle these as such
                if (button_press->detail == 4)
                {
                    bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(this, event_time, vsg::vec3(0.0f, 1.0f, 0.0f)));
                }
                else if (button_press->detail == 5)
                {
                    bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(this, event_time, vsg::vec3(0.0f, -1.0f, 0.0f)));
                }
                else
                {
                    uint16_t pressedButtonMask = 1 << (7 + button_press->detail);
                    uint16_t newButtonMask = maskButtons(button_press->state) | pressedButtonMask;
                    bufferedEvents.emplace_back(vsg::ButtonPressEvent::create(this, event_time, button_press->event_x, button_press->event_y, vsg::ButtonMask(newButtonMask), button_press->detail));
                }
            }

            break;
        }
        case (XCB_BUTTON_RELEASE): {
            auto button_release = reinterpret_cast<const xcb_button_release_event_t*>(event);

            // ignore button 4 and 5 as X11/Xcb use them as up/down scroll wheel events
            if (button_release->same_screen && button_release->detail != 4 && button_release->detail != 5)
            {
                vsg::clock::time_point event_time = _first_xcb_time_point + std::chrono::milliseconds(button_release->time - _first_xcb_timestamp);
                uint16_t releasedButtonMask = 1 << (7 + button_release->detail);
                uint16_t newButtonMask = maskButtons(button_release->state) & ~releasedButtonMask;
                bufferedEvents.emplace_back(vsg::ButtonReleaseEvent::create(this, event_time, button_release->event_x, button_release->event_y, vsg::ButtonMask(newButtonMask), button_release->detail));
            }

            break;
        }
        case (XCB_MOTION_NOTIFY): {
            auto motion_notify = reinterpret_cast<const xcb_motion_notify_event_t*>(event);
            if (motion_notify->same_screen)
            {
                vsg::clock::time_point event_time = _first_xcb_time_point + std::chrono::milliseconds(motion_notify->time - _first_xcb_timestamp);
                bufferedEvents.emplace_back(vsg::MoveEvent::create(this, event_time, motion_notify->event_x, motion_notify->event_y, vsg::ButtonMask(maskButtons(motion_notify->state))));
            }

            break;
        }
        case (XCB_GE_GENERIC): {
            // can't find meaningful documentation on what information is encoded in a xcb_ge_generic_event_t
            // so no way to map it to anything on the VSG side.
            //
            //auto generic_event = reinterpret_cast<const xcb_ge_generic_event_t*>(event);
            //debug("generic_event->event_type = ", generic_event->event_type);
            break;
        }
        default: {
            warn("xcb_event type not handled, response_type = ", static_cast<int>(response_type));
            break;
        }
        }
        free(event);
    }

    return Window::pollEvents(events);
}

void Xcb_Window::resize()
{
    xcb_get_geometry_reply_t* geometry_reply = xcb_get_geometry_reply(_connection, xcb_get_geometry(_connection, _window), nullptr);
    if (geometry_reply)
    {
        _extent2D.width = geometry_reply->width;
        _extent2D.height = geometry_reply->height;
        free(geometry_reply);

        buildSwapchain();
    }
}
