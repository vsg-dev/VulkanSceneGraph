#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#define VK_USE_PLATFORM_MACOS_MVK

#include <vsg/viewer/Window.h>
#include <vsg/ui/KeyEvent.h>

@class NSEvent;
@class CAMetalLayer;
@class vsg_MacOS_NSView;
@class vsg_MacOS_NSWindow;

namespace vsgMacOS
{
    extern vsg::Names getInstanceExtensions();

    class KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using kVKKeyCodeToKeySymbolMap = std::map<unsigned short, vsg::KeySymbol>;

        bool getKeySymbol(NSEvent* anEvent, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier);

    protected:
        kVKKeyCodeToKeySymbolMap _keycodeMap;
    };


    class MacOS_Window : public vsg::Window
    {
    public:
        MacOS_Window() = delete;
        MacOS_Window(const MacOS_Window&) = delete;
        MacOS_Window operator = (const MacOS_Window&) = delete;

        using Result = vsg::Result<vsg::Window, VkResult, VK_SUCCESS>;
        static Result create(vsg::ref_ptr<Window::Traits> traits, bool debugLayer=false, bool apiDumpLayer=false, vsg::AllocationCallbacks* allocator=nullptr);

        virtual bool valid() const { return _window && !_shouldClose; }

        virtual bool pollEvents(vsg::Events& events);

        virtual bool resized() const;

        virtual void resize();

        bool handleNSEvent(NSEvent* anEvent);

        // native objects
        vsg_MacOS_NSWindow* window() { return _window; };
        vsg_MacOS_NSView* view() { return _view; };
        CAMetalLayer* layer() { return _metalLayer; };

        vsg::clock::time_point getEventTime(double eventTime)
        {
            long elapsedmilli = long(double(eventTime - _first_macos_timestamp) * 1000.0f);
            return _first_macos_time_point + std::chrono::milliseconds(elapsedmilli);
        }

        void shouldClose(bool shouldClose) { _shouldClose = shouldClose; }

    protected:
        virtual ~MacOS_Window();

        MacOS_Window(vsg::ref_ptr<vsg::Window::Traits> traits, bool debugLayer, bool apiDumpLayer, vsg::AllocationCallbacks* allocator);

        vsg_MacOS_NSWindow* _window;
        vsg_MacOS_NSView* _view;
        CAMetalLayer* _metalLayer;

        bool _shouldClose;

        double _first_macos_timestamp = 0;
        vsg::clock::time_point _first_macos_time_point;

        vsg::Events _bufferedEvents;
        vsg::ref_ptr<KeyboardMap> _keyboard;
    };

} // namespace vsg

