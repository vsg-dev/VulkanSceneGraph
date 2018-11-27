#pragma once

#include <vsg/viewer/Window.h>

#include <xcb/xcb.h>

#include <iostream>

namespace xcb
{

struct Settings
{
    xcb_connection_t* _connection = nullptr;
    xcb_screen_t* _screen = nullptr;
    xcb_window_t _window{};
    xcb_atom_t _wmProtocols{};
    xcb_atom_t _wmDeleteWindow{};
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

    class Xcb_Window : public vsg::Window, protected Settings
    {
    public:

        Xcb_Window() = delete;
        Xcb_Window(const Xcb_Window&) = delete;
        Xcb_Window& operator = (const Xcb_Window&) = delete;

        static Result create(const Traits& traits, bool debugLayer=false, bool apiDumpLayer=false, vsg::AllocationCallbacks* allocator=nullptr);

        bool valid() const override;

        bool pollEvents() override;

        bool resized() const override;

        void resize() override;

    protected:
        Xcb_Window(Settings& settings , vsg::Instance* instance, vsg::Surface* surface, vsg::PhysicalDevice* physicalDevice, vsg::Device* device, vsg::RenderPass* renderPass, bool debugLayersEnabled);

        ~Xcb_Window();
    };


}

