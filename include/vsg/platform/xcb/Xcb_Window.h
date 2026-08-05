/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#pragma once

#include <vsg/app/Window.h>
#include <vsg/ui/DropEvent.h>
#include <vsg/ui/KeyEvent.h>

#include <xcb/xcb.h>

#include <vulkan/vulkan_xcb.h>

namespace vsgXcb
{

    /// KeyboardMap maps Xcb keyboard events to vsg::KeySymbol
    class KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using KeycodeModifier = std::pair<uint16_t, uint16_t>;
        using KeycodeMap = std::map<KeycodeModifier, vsg::KeySymbol>;

        void add(uint16_t keycode, uint16_t modifier, vsg::KeySymbol key);

        void add(uint16_t keycode, std::initializer_list<std::pair<uint16_t, vsg::KeySymbol>> combinations);

        vsg::KeySymbol getKeySymbol(uint16_t keycode, uint16_t modifier);
        vsg::KeyModifier getKeyModifier(vsg::KeySymbol keySym, uint16_t modifier, bool pressed);

    protected:
        KeycodeMap _keycodeMap;
        uint16_t _modifierMask;
    };

    ///  Xcb_Surface implements XcbSurface creation.
    class Xcb_Surface : public vsg::Surface
    {
    public:
        Xcb_Surface(vsg::Instance* instance, xcb_connection_t* connection, xcb_window_t window);
    };

    /// Xcb_Window implements Xcb specific window creation, event handling and vulkan Surface setup.
    class Xcb_Window : public vsg::Inherit<vsg::Window, Xcb_Window>
    {
    public:
        Xcb_Window(vsg::ref_ptr<vsg::WindowTraits> traits);
        Xcb_Window() = delete;
        Xcb_Window(const Xcb_Window&) = delete;
        Xcb_Window& operator=(const Xcb_Window&) = delete;

        const char* instanceExtensionSurfaceName() const override { return VK_KHR_XCB_SURFACE_EXTENSION_NAME; }

        bool valid() const override;

        bool visible() const override;

        void releaseWindow() override;
        void releaseConnection() override;

        bool pollEvents(vsg::UIEvents& events) override;

        void resize() override;

    protected:
        ~Xcb_Window();

        void _initSurface() override;

        /// Set up the XdndAware property so that the window is offered file drops.
        void _initXdnd();

        /// Handle one XDND ClientMessage or the SelectionNotify that carries the dropped file names,
        /// appending any resulting Drop events to bufferedEvents. Returns false if the message was
        /// not part of a drag and drop exchange.
        bool _handleXdndEvent(const xcb_generic_event_t* event);

        /// Send the XdndStatus reply that tells the source whether a drop would be accepted. Sent
        /// once per frame while a drag is in progress, so a handler that changes its mind while the
        /// cursor is stationary is still able to update the cursor.
        void _sendXdndStatus();

        xcb_connection_t* _connection = nullptr;
        xcb_screen_t* _screen = nullptr;
        xcb_window_t _window{};
        xcb_atom_t _wmProtocols{};
        xcb_atom_t _wmDeleteWindow{};

        // Drag and drop. The source only hands over the file names once the button is released, so
        // the drop is delivered in two steps: XdndDrop asks for the XdndSelection selection, and the
        // DropFilesEvent is emitted when the resulting SelectionNotify arrives.
        struct XdndAtoms
        {
            xcb_atom_t aware{};
            xcb_atom_t enter{};
            xcb_atom_t position{};
            xcb_atom_t status{};
            xcb_atom_t leave{};
            xcb_atom_t drop{};
            xcb_atom_t finished{};
            xcb_atom_t selection{};
            xcb_atom_t typeList{};
            xcb_atom_t actionCopy{};
            xcb_atom_t uriList{};
            xcb_atom_t property{};
        };

        XdndAtoms _xdnd;
        xcb_window_t _xdndSource = 0;
        bool _xdndHovering = false;
        bool _xdndDropping = false;
        bool _xdndAccepted = false;
        bool _xdndAcceptedSent = false;
        bool _xdndStatusPending = false;
        int32_t _xdndX = 0;
        int32_t _xdndY = 0;
        vsg::clock::time_point _xdndDropStarted;

        // The hover event emitted last frame, kept so that the accept flag the application set on it
        // can be read back once the frame that handled it has finished.
        vsg::ref_ptr<vsg::DropHoverEvent> _xdndHoverEvent;

        bool _windowMapped = false;

        xcb_timestamp_t _first_xcb_timestamp = 0;
        vsg::clock::time_point _first_xcb_time_point;

        vsg::ref_ptr<KeyboardMap> _keyboard;
    };

} // namespace vsgXcb

EVSG_type_name(vsgXcb::Xcb_Window);
