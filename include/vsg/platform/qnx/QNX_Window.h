#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield
Copyright (C) 2026 BlackBerry Limited

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#define VK_USE_PLATFORM_QNX

#include <vsg/app/Window.h>
#include <vsg/ui/KeyEvent.h>

#include <screen/screen.h>
#include <sys/keycodes.h>

namespace vsgQNX
{

/// KeyboardMap maps screen keyboard events to vsg::KeySymbol
class KeyboardMap : public vsg::Object
{
public:
    KeyboardMap();

    using KeyCodeToKeySymbolMap = std::map<int, vsg::KeySymbol>;
    using KeyModToKeyModifierMap = std::map<int, vsg::KeyModifier>;

    bool getKeySymbol(int keycap, int keysym, int keymod, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier);

protected:
    KeyCodeToKeySymbolMap _keycodeMap;
    KeyModToKeyModifierMap _keymodMap;
};

class QNX_Surface : public vsg::Surface
{
public:
    QNX_Surface(vsg::Instance* instance, screen_context_t context, screen_window_t window);
};

class QNX_Window : public vsg::Inherit<vsg::Window, QNX_Window>
{
public:
    QNX_Window(vsg::ref_ptr<vsg::WindowTraits> traits);
    QNX_Window() = delete;
    QNX_Window(const QNX_Window&) = delete;
    QNX_Window operator=(const QNX_Window&) = delete;

    const char* instanceExtensionSurfaceName() const override { return "VK_QNX_screen_surface"; }

    bool valid() const override;

    bool visible() const override;

    void releaseWindow() override;
    void releaseConnection() override;

    bool pollEvents(vsg::UIEvents& events) override;

    void resize() override;

protected:
    virtual ~QNX_Window();

    void _initSurface() override;

    screen_context_t _context = nullptr;
    screen_event_t _event = nullptr;
    screen_window_t _window = nullptr;
    // A value of null is valid and indicates the default display.
    screen_display_t _display = reinterpret_cast<screen_display_t>(-1);

    bool _windowVisible = false;
    bool _windowInFocus = true;

    int _pointerPosPrev[2];
    int _buttonsPressedPrev;

    int64_t _first_qnx_timestamp = 0;
    vsg::clock::time_point _first_qnx_time_point;

    vsg::ref_ptr<KeyboardMap> _keyboard;
};

} // namespace vsgQNX

EVSG_type_name(vsgQNX::QNX_Window);
