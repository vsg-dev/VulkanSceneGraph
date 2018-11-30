/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include "Xcb_Window.h"

#include <vsg/vk/Extensions.h>

#include <xcb/xproto.h>

#include <thread>
#include <chrono>
#include <iostream>

namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to a Xcb_Window
    Window::Result Window::create(const Window::Traits& traits, bool debugLayer, bool apiDumpLayer, vsg::AllocationCallbacks* allocator)
    {
        return xcb::Xcb_Window::create(traits, debugLayer, apiDumpLayer, allocator);
    }
}

using namespace vsg;
using namespace xcb;

KeyboardMap::KeyboardMap()
{
    _modifierMask=0xff;
}

void KeyboardMap::add(uint16_t keycode, uint16_t modifier, KeySymbol key)
{
    //std::cout<<"Added ["<<keycode<<", "<<modifier<<"] "<<(int)key<<std::endl;
    _keycodeMap[KeycodeModifier(keycode, modifier)] = key;
}

void KeyboardMap::add(uint16_t keycode, std::initializer_list<std::pair<uint16_t, KeySymbol> > combinations)
{
    for(auto [modifier, key] : combinations)
    {
        add(keycode, modifier, key);
    }
}

// reference
// https://harrylovescode.gitbooks.io/vulkan-api/content/chap04/chap04-linux.html

vsg::Window::Result Xcb_Window::create(const Traits& traits, bool debugLayer, bool apiDumpLayer, vsg::AllocationCallbacks* allocator)
{
    ref_ptr<Window> window(new Xcb_Window(traits, debugLayer, apiDumpLayer, allocator));

    return Result("Failed to created Window, no Xcb implementation yet.", VK_ERROR_INVALID_EXTERNAL_HANDLE);

//    return window;
}


Xcb_Window::Xcb_Window(const Traits& traits, bool debugLayer, bool apiDumpLayer, vsg::AllocationCallbacks* allocator)
{
    std::cout<<"Xcb_Window() "<<traits.x<<", "<<traits.y<<", "<<traits.width<<", "<<traits.height<<std::endl;

    const char* displayName = 0;
    int screenNum = traits.screenNum;

    bool fullscreen = false;//true;
    uint32_t override_redirect = 0; // fullscreen ? 1 : 0;

    // open connection
    _connection = xcb_connect(displayName, &screenNum);
    if (xcb_connection_has_error(_connection))
    {
        // close connection
        xcb_disconnect(_connection);
        //return Result("Failed to created Window, unable able to establish xcb connection.", VK_ERROR_INVALID_EXTERNAL_HANDLE);  TODO need to throw?
        return;
    }

    std::cout<<"    created connection "<<_connection<<" screenNum="<<screenNum<<" "<<std::endl;

    // TODO, should record Traits within Window? Should pass back selected screeenNum?

    // get the screeen
    const xcb_setup_t* setup = xcb_get_setup(_connection);

    _keyboard = new KeyboardMap;

    {
        std::cout<<"   *** stting up : min_keycode="<<(int)(setup->min_keycode)<<", max_keycode="<<(int)(setup->max_keycode)<<std::endl;

        xcb_keycode_t min_keycode = setup->min_keycode;
        xcb_keycode_t max_keycode = setup->max_keycode;

#if 0
        for(int keycode = min_keycode; keycode <= max_keycode; ++keycode)
        {
            std::cout<<"       keycode = "<<(int)keycode<<std::endl;
        }
#endif
        xcb_get_keyboard_mapping_cookie_t keyboard_mapping_cookie = xcb_get_keyboard_mapping(_connection, min_keycode, (max_keycode-min_keycode)+1);
        xcb_get_keyboard_mapping_reply_t * keyboard_mapping_reply = xcb_get_keyboard_mapping_reply(_connection, keyboard_mapping_cookie, nullptr);


        std::cout<<"     keysyms_per_keycode = "<<int(keyboard_mapping_reply->keysyms_per_keycode)<<std::endl;
        std::cout<<"     sequence = "<<keyboard_mapping_reply->sequence<<std::endl;
        std::cout<<"     length = "<<keyboard_mapping_reply->length<<std::endl;

        const xcb_keysym_t * keysyms = xcb_get_keyboard_mapping_keysyms (keyboard_mapping_reply);
        int length = xcb_get_keyboard_mapping_keysyms_length (keyboard_mapping_reply);

        std::cout<<"     xcb_get_keyboard_mapping_keysyms_length() = "<<length<<std::endl;
        int keysyms_per_keycode = keyboard_mapping_reply->keysyms_per_keycode;
        for(int i=0; i<length; i+=keysyms_per_keycode)
        {
            const xcb_keysym_t * keysym = &keysyms[i];
            xcb_keycode_t keycode = min_keycode +i/keysyms_per_keycode;
            std::cout<<"     keycode = "<<int(keycode)<<std::endl;
            for(int m=0; m<keysyms_per_keycode; ++m)
            {
                std::cout<<"          keysym["<<m<<"] "<<int(keysym[m])<<" "<<std::hex<<int(keysym[m])<<std::dec;
                if (keysym[m]>=32 && keysym[m]<256) std::cout<<" ["<<uint8_t(keysym[m])<<"]";
                std::cout<<std::endl;
                if (keysym[m]!=0) _keyboard->add(keycode, m, static_cast<KeySymbol>(keysym[m]));
            }
        }

        free(keyboard_mapping_reply);
    }




    xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(setup);
    for (;screenNum>0; --screenNum) xcb_screen_next(&screen_iterator);
    _screen = screen_iterator.data;
    std::cout<<"    selected screen "<<_screen<<std::endl;

    // generate the widnow id
    _window = xcb_generate_id(_connection);

    std::cout<<"    _window "<<_window<<std::endl;

    std::cout<<"    screen.width_in_pixels = "<<_screen->width_in_pixels<<std::endl;
    std::cout<<"    screen.height_in_pixels = "<<_screen->height_in_pixels<<std::endl;
    std::cout<<"    width_in_millimeters = "<<_screen->width_in_millimeters<<std::endl;
    std::cout<<"    height_in_millimeters = "<<_screen->height_in_millimeters<<std::endl;

    uint8_t depth = XCB_COPY_FROM_PARENT;
    xcb_window_t parent = _screen->root;
    uint16_t border_width = 0;
    uint16_t window_class = XCB_WINDOW_CLASS_INPUT_OUTPUT;
    xcb_visualid_t visual = XCB_COPY_FROM_PARENT;
    uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
    uint32_t value_list[] =
    {
        _screen->black_pixel,
        override_redirect,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_PROPERTY_CHANGE
    };

    // ceate window
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
            traits.x, traits.y, traits.width, traits.height,
            border_width,
            window_class,
            visual,
            value_mask,
            value_list);
    }

    // set class of window to enable window manager configuration with rules for positioning
    xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING,  8, traits.windowClass.size(), traits.windowClass.data());

    // set title of window
    xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING,  8, traits.windowTitle.size(), traits.windowTitle.data());

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
        std::vector<xcb_atom_t> stateAtoms{ wmFullScreen };

        xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, wmState, XCB_ATOM_ATOM, 32, stateAtoms.size(), stateAtoms.data());

        std::cout<<"Set up full screen"<<std::endl;
    }


    // set whethert the window should have a border or not, and if so what resize/move/close functions to enable
    AtomRequest motifHintAtom(_connection, "_MOTIF_WM_HINTS");
    MotifHints hints = fullscreen ? MotifHints::borderless() : MotifHints::window();
    xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, motifHintAtom, motifHintAtom, 32, 5, &hints);

#if 0
    // allowed actions
    AtomRequest wmAllowedActions(_connection, "_NET_WM_ALLOWED_ACTIONS");
    AtomRequest wmActionFullScreen(_connection, "_NET_WM_ACTION_FULLSCREEN");
    std::vector<xcb_atom_t> actionAtoms
    {
        wmActionFullScreen
    };

    xcb_change_property(_connection, XCB_PROP_MODE_REPLACE, _window, wmAllowedActions, XCB_ATOM_ATOM, 32, actionAtoms.size(), actionAtoms.data());

#endif

    std::cout<<"Create window : "<<traits.windowTitle<<std::endl;


    // work out the X server timestamp by checking for the property notify events that result for the above xcb_change_property calls.
    _first_xcb_timestamp = 0;
    _first_xcb_time_point = vsg::clock::now();
    {
        xcb_generic_event_t *event = nullptr;
        while (_first_xcb_timestamp==0 && (event = xcb_wait_for_event(_connection)))
        {
            uint8_t response_type = event->response_type & ~0x80;
            if (response_type==XCB_PROPERTY_NOTIFY)
            {
                auto propety_notify = reinterpret_cast<const xcb_property_notify_event_t*>(event);
                _first_xcb_timestamp = propety_notify->time;
                _first_xcb_time_point = vsg::clock::now();
            }
            free(event);
        }
    }
    vsg::clock::time_point start_point = _first_xcb_time_point;

    xcb_map_window(_connection, _window);


#if 0
    // reconfigure the window position and size.
    const uint32_t values[] = { 100, 200, 300, 400 };
    xcb_configure_window (_connection, _window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, values);
    xcb_flush(_connection);
#endif

    xcb_flush(_connection);






    // trial and error has found that querying window geometry immediately does not reflect windowing manager resizing so have to add in a short sleep for it to complete
    std::this_thread::sleep_for( std::chrono::milliseconds(10) );








    std::map<std::string, xcb_drawable_t>  windows;
    windows["main"] = _window;

    xcb_query_tree_cookie_t tree_cookie = xcb_query_tree(_connection, _window);
    xcb_query_tree_reply_t* tree_reply = xcb_query_tree_reply(_connection, tree_cookie, nullptr);

    xcb_drawable_t root{};

    if (tree_reply!=nullptr)
    {
        root = tree_reply->root;

        windows["parent"] = tree_reply->parent;

        std::cout<<"number children = "<<tree_reply->children_len<<std::endl;

        free(tree_reply);
    }

#if 1
    for(int i=0; i<1; ++i)
    {
        for(auto[name, window] : windows)
        {
            int32_t x, y;
            uint32_t width, height;

            xcb_get_geometry_reply_t* geometry_reply = xcb_get_geometry_reply(_connection, xcb_get_geometry(_connection, window), nullptr);
            if (geometry_reply!=nullptr)
            {
                x = geometry_reply->x;
                y = geometry_reply->y;
                width  = geometry_reply->width;
                height = geometry_reply->height;

                std::cout<<"  geometry_rely : "<<name<<", "<<window<<" ["<<x<<", "<<y<<", "<<width<<", "<<height<<"]"<<std::endl;

                xcb_translate_coordinates_cookie_t trans_cookie = xcb_translate_coordinates(_connection, window, root, -x, -y);
                xcb_translate_coordinates_reply_t* trans_reply = xcb_translate_coordinates_reply(_connection, trans_cookie, nullptr);
                if (trans_reply)
                {
                    std::cout<<"      trans->dst_x = "<<trans_reply->dst_x<<", trans->dst_y = "<<trans_reply->dst_y<<std::endl;
                    free(trans_reply);
                }

                free(geometry_reply);
            }
            else
            {
                std::cout<<"  no geometry_rely : "<<name<<", "<<window<<std::endl;
            }
        }
        std::this_thread::sleep_for( std::chrono::milliseconds(10) );
    }
#endif


    bool close = false;

    while(!close)
    {
        using Events = std::list<ref_ptr<vsg::UIEvent>>;
        Events events;

        xcb_generic_event_t *event;
        int i=0;
        while ((event = xcb_poll_for_event(_connection)) )
        {
            ++i;
            uint8_t response_type = event->response_type & ~0x80;
            switch(response_type)
            {
                case(XCB_EXPOSE):
                {
                    auto expose = reinterpret_cast<const xcb_expose_event_t*>(event);
                    std::cout<<"expose "<<expose->window<<", "<<expose->x<<", "<<expose->y<<", "<<expose->width<<", "<<expose->height<<", count="<<expose->count<<std::endl;
                    break;
                }
                case XCB_CLIENT_MESSAGE:
                {
                    auto client_message = reinterpret_cast<const xcb_client_message_event_t*>(event);
                    if (client_message->data.data32[0]==_wmDeleteWindow)
                    {
                        std::cout<<"client message "<<client_message->data.data32[0]<<std::endl;
                        std::cout<<"     _wmDeleteWindow = "<<_wmDeleteWindow<<std::endl;
                        close = true;
                    }
                    break;
                }
                case XCB_KEY_PRESS:
                {
                    auto key_press = reinterpret_cast<const xcb_key_press_event_t*>(event);

                    vsg::clock::time_point event_time = _first_xcb_time_point + std::chrono::milliseconds(key_press->time-_first_xcb_timestamp);
                    vsg::KeySymbol keySymbol = _keyboard->getKeySymbol(key_press->detail, 0);
                    vsg::KeySymbol keySymbolModified = _keyboard->getKeySymbol(key_press->detail, key_press->state);
                    events.emplace_back(new vsg::KeyPressEvent(0, event_time, keySymbol, keySymbolModified, KeyModifier(key_press->state), 0));

                    break;
                }
                case XCB_KEY_RELEASE:
                {
                    auto key_release = reinterpret_cast<const xcb_key_release_event_t*>(event);

                    vsg::clock::time_point event_time = _first_xcb_time_point + std::chrono::milliseconds(key_release->time-_first_xcb_timestamp);
                    vsg::KeySymbol keySymbol = _keyboard->getKeySymbol(key_release->detail, 0);
                    vsg::KeySymbol keySymbolModified = _keyboard->getKeySymbol(key_release->detail, key_release->state);
                    events.emplace_back(new vsg::KeyReleaseEvent(0, event_time, keySymbol, keySymbolModified, KeyModifier(key_release->state), 0));

                    break;
                }
                default:
                {
                    std::cout<<"event not handled, response_type = "<<(int)response_type<<std::endl;
                    break;
                }
            }
            free(event);
        }

        struct PrintEvents : public vsg::Visitor
        {
            vsg::clock::time_point start_point;

            PrintEvents(vsg::clock::time_point in_start_point) : start_point(in_start_point) {}

            void apply(vsg::UIEvent& event)
            {
                std::cout<<"event : "<<event.className()<<", "<<std::chrono::duration<double>(event.time - start_point).count()<<std::endl;
            }

            void apply(vsg::KeyReleaseEvent& keyRelease)
            {
                std::cout<<"KeyReleaeEvent : "<<keyRelease.className()<<", "<<std::chrono::duration<double>(keyRelease.time - start_point).count()<<", "<<keyRelease.keyBase<<std::endl;
            }

            void apply(vsg::KeyPressEvent& keyPress)
            {
                std::cout<<"KeyPressEvent : "<<keyPress.className()<<", "<<std::chrono::duration<double>(keyPress.time - start_point).count()<<", "<<keyPress.keyBase<<", "<<keyPress.keyModified<<std::endl;
            }
        };

        PrintEvents print(start_point);

        for(auto& vsg_event : events)
        {
            vsg_event->accept(print);
        }

        if (i>0) std::cout<<std::endl;
    }


#if 0
    for(int i=0; i<1000; ++i)
    {
        int32_t x, y;
        uint32_t width, height;
        if (xcb::getWindowGeometry(_connection, _window, x, y, width, height))
        {
            std::cout<<"Window geometry : "<<x<<", "<<y<<", "<<width<<", "<<height<<std::endl;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds(10) );
    }
#endif

#if 0
    std::this_thread::sleep_for( std::chrono::seconds(4) );

#if 1
    std::cout<<"Ummap window"<<std::endl;

    xcb_unmap_window(_connection, _window);
    xcb_flush(_connection);

    std::this_thread::sleep_for( std::chrono::seconds(2) );
#endif

#if 1
    std::cout<<"map window"<<std::endl;

    xcb_map_window(_connection, _window);
    xcb_flush(_connection);

    std::this_thread::sleep_for( std::chrono::seconds(4) );
#endif


#endif

    std::cout<<"Destroy window"<<std::endl;

    xcb_destroy_window(_connection, _window);

    xcb_flush(_connection);



    // close connection
    xcb_disconnect(_connection);

    std::cout<<"Disconnect"<<std::endl;
}

Xcb_Window::~Xcb_Window()
{
    std::cout<<"Destruction Xcb_Widnow"<<std::endl;
}

bool Xcb_Window::valid() const
{
    return _window!=9;
}

bool Xcb_Window::pollEvents(Events& events)
{
    unsigned numEventsBefore = events.size();
    xcb_generic_event_t *event;
    int i=0;
    while ((event = xcb_poll_for_event(_connection)) )
    {
        ++i;
        uint8_t response_type = event->response_type & ~0x80;
        switch(response_type)
        {
            case(XCB_EXPOSE):
            {
                auto expose = reinterpret_cast<const xcb_expose_event_t*>(event);
                std::cout<<"expose "<<expose->window<<", "<<expose->x<<", "<<expose->y<<", "<<expose->width<<", "<<expose->height<<", count="<<expose->count<<std::endl;
                break;
            }
            case XCB_CLIENT_MESSAGE:
            {
                auto client_message = reinterpret_cast<const xcb_client_message_event_t*>(event);
                if (client_message->data.data32[0]==_wmDeleteWindow)
                {
                    std::cout<<"client message "<<client_message->data.data32[0]<<std::endl;
                    std::cout<<"     _wmDeleteWindow = "<<_wmDeleteWindow<<std::endl;
                }
                break;
            }
            case XCB_KEY_PRESS:
            {
                auto key_press = reinterpret_cast<const xcb_key_press_event_t*>(event);

                vsg::clock::time_point event_time = _first_xcb_time_point + std::chrono::milliseconds(key_press->time-_first_xcb_timestamp);
                vsg::KeySymbol keySymbol = _keyboard->getKeySymbol(key_press->detail, 0);
                vsg::KeySymbol keySymbolModified = _keyboard->getKeySymbol(key_press->detail, key_press->state);
                events.emplace_back(new vsg::KeyPressEvent(0, event_time, keySymbol, keySymbolModified, KeyModifier(key_press->state), 0));

                break;
            }
            case XCB_KEY_RELEASE:
            {
                auto key_release = reinterpret_cast<const xcb_key_release_event_t*>(event);

                vsg::clock::time_point event_time = _first_xcb_time_point + std::chrono::milliseconds(key_release->time-_first_xcb_timestamp);
                vsg::KeySymbol keySymbol = _keyboard->getKeySymbol(key_release->detail, 0);
                vsg::KeySymbol keySymbolModified = _keyboard->getKeySymbol(key_release->detail, key_release->state);
                events.emplace_back(new vsg::KeyReleaseEvent(0, event_time, keySymbol, keySymbolModified, KeyModifier(key_release->state), 0));

                break;
            }
            default:
            {
                std::cout<<"event not handled, response_type = "<<(int)response_type<<std::endl;
                break;
            }
        }
        free(event);
    }
    unsigned numEventsAfter = events.size();
    return numEventsBefore!=numEventsAfter;
}

bool Xcb_Window::resized() const
{
    return false;
}

void Xcb_Window::resize()
{
}
