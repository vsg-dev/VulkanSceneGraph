/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#pragma once

#include <vsg/viewer/Window.h>
#include <vsg/ui/KeyEvent.h>

#include <xcb/xcb.h>

#include <iostream>

namespace xcb
{

    class KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using KeycodeModifier = std::pair<uint16_t, uint16_t>;
        using KeycodeMap = std::map<KeycodeModifier, vsg::KeySymbol>;

        void add(uint16_t keycode, uint16_t modifier, vsg::KeySymbol key);

        void add(uint16_t keycode, std::initializer_list<std::pair<uint16_t, vsg::KeySymbol> > combinations);

        vsg::KeySymbol getKeySymbol(uint16_t keycode, uint16_t modifier)
        {
            auto itr = _keycodeMap.find(KeycodeModifier(keycode, 0));
            if (itr==_keycodeMap.end()) return vsg::KEY_Undefined;

            vsg::KeySymbol baseKeySymbol = itr->second;
            if (modifier==0) return baseKeySymbol;

            bool shift = (modifier & vsg::MODKEY_Shift)!=0;
            uint16_t index = 0;

            if (baseKeySymbol>=vsg::KEY_KP_Space && baseKeySymbol<=vsg::KEY_KP_Divide)
            {
                // numeric keypad values
                bool numLock = ((modifier & vsg::MODKEY_NumLock)!=0);
                index = (numLock && !shift) ? 1 : 0;
            }
            else
            {
                bool capsLock = (modifier & vsg::MODKEY_CapsLock)!=0;
                index = (capsLock ? !shift : shift) ? 1 : 0;
            }
            if (index==0) return baseKeySymbol;

            if (itr = _keycodeMap.find(KeycodeModifier(keycode, index)); itr!=_keycodeMap.end()) return itr->second;
            return vsg::KEY_Undefined;
        }

    protected:
        KeycodeMap _keycodeMap;
        uint16_t   _modifierMask;
    };


    // window decoation
    struct MotifHints
    {
        enum Flags : uint32_t
        {
            FLAGS_FUNCTIONS   = 1<<0,
            FLAGS_DECORATIONS = 1<<1,
            FLAGS_INPUT_MODE  = 1<<2,
            FLAGS_STATUS      = 1<<3,
        };

        enum Functions : uint32_t
        {
            FUNC_ALL = 1<<0,
            FUNC_RESIZE = 1<<1,
            FUNC_MOVE = 1<<2,
            FUNC_MINIMUMSIZE = 1<<3,
            FUNC_MAXIMUMSIZE = 1<<4,
            FUNC_CLOSE = 1<<5
        };

        enum Decorations : uint32_t
        {
            DECOR_ALL = 1<<0,
            DECOR_BORDER = 1<<1,
            DECOR_RESIZE = 1<<2,
            DECOR_TITLE = 1<<3,
            DECOR_MENU = 1<<4,
            DECOR_MINIMUMSIZE = 1<<5,
            DECOR_MAXIMUMSIZE = 1<<6
        };

        static MotifHints borderless()
        {
            MotifHints hints;
            hints.flags = FLAGS_DECORATIONS;
            return hints;
        }

        static MotifHints window(bool resize=true, bool move=true, bool close=true)
        {
            MotifHints hints;
            hints.flags = FLAGS_DECORATIONS | FLAGS_FUNCTIONS;
            hints.functions = 0;
            if (resize) hints.functions |= FUNC_RESIZE;
            if (move) hints.functions |= FUNC_MOVE;
            if (close) hints.functions |= FUNC_CLOSE;
            hints.decorations = DECOR_ALL;
            return hints;
        }


        uint32_t flags{};
        uint32_t functions{};
        uint32_t decorations{};
        int32_t  input_mode{};
        uint32_t status{};
    };

    class AtomRequest
    {
    public:
        AtomRequest(xcb_connection_t* connection, const char* atom_name):
            _connection(connection)
        {
            _cookie = xcb_intern_atom(connection, false, strlen(atom_name), atom_name);
        }

        operator xcb_atom_t ()
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
            width  = geometry_reply->width;
            height = geometry_reply->height;

            if (tree_reply)
            {
                xcb_translate_coordinates_cookie_t trans_cookie = xcb_translate_coordinates(connection, window, tree_reply->parent, x, y);
                xcb_translate_coordinates_reply_t* trans_reply = xcb_translate_coordinates_reply(connection, trans_cookie, nullptr);
                if (trans_reply)
                {
                    std::cout<<"trans->dst_x = "<<trans_reply->dst_x<<", trans->dst_y = "<<trans_reply->dst_y<<std::endl;
                    free(trans_reply);
                }

                trans_cookie = xcb_translate_coordinates(connection, window, geometry_reply->root, x, y);
                trans_reply = xcb_translate_coordinates_reply(connection, trans_cookie, nullptr);
                if (trans_reply)
                {
                    std::cout<<"2nd trans->dst_x = "<<trans_reply->dst_x<<", trans->dst_y = "<<trans_reply->dst_y<<std::endl;
                    free(trans_reply);
                }

                free(tree_reply);
            }

            free(geometry_reply);
            return true;
        }
        return false;
    }

    class Xcb_Surface : public vsg::Surface
    {
    public:
        Xcb_Surface(vsg::Instance* instance, xcb_connection_t* connection, xcb_window_t window, vsg::AllocationCallbacks* allocator = nullptr);
    };

    class Xcb_Window : public vsg::Window
    {
    public:

        Xcb_Window() = delete;
        Xcb_Window(const Xcb_Window&) = delete;
        Xcb_Window& operator = (const Xcb_Window&) = delete;

        static Result create(const Traits& traits, bool debugLayer=false, bool apiDumpLayer=false, vsg::AllocationCallbacks* allocator=nullptr);

        bool valid() const override;

        bool pollEvents(vsg::Events& events) override;

        bool resized() const override;

        void resize() override;

    protected:
        Xcb_Window(const Traits& traits, bool debugLayer, bool apiDumpLayer, vsg::AllocationCallbacks* allocator);

        ~Xcb_Window();

        xcb_connection_t* _connection = nullptr;
        xcb_screen_t* _screen = nullptr;
        xcb_window_t _window{};
        xcb_atom_t _wmProtocols{};
        xcb_atom_t _wmDeleteWindow{};

        bool _closeEventRecieved = false;

        xcb_timestamp_t _first_xcb_timestamp = 0;
        vsg::clock::time_point _first_xcb_time_point;

        vsg::ref_ptr<KeyboardMap> _keyboard;
    };


}

