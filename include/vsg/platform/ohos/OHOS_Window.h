#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#define VK_USE_PLATFORM_OHOS

#include <vsg/app/Window.h>
#include <vsg/ui/KeyEvent.h>

// Forward declaration for OHOS native window type
struct NativeWindow;
typedef struct NativeWindow OHNativeWindow;

// Forward declaration for OHOS ArkUI event type
struct ArkUI_NodeEvent;

namespace vsgOHOS
{
    typedef struct {
        bool capsLock;
        bool numLock;
        bool shiftPressed;
        bool ctrlPressed;
        bool altPressed;
    } KeyMetaState;

    /// KeyboardMap maps OpenHarmony keyboard events to vsg::KeySymbol
    class KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using OHOSKeyCodeToKeySymbolMap = std::map<uint32_t, vsg::KeySymbol>;

        bool getKeySymbol(uint32_t keycode, KeyMetaState* KeyMetaState, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier);
    
    protected:
        OHOSKeyCodeToKeySymbolMap _keycodeMap;
    };

    /// OHOS_Window implements OpenHarmony specific window creation, event handling and vulkan Surface setup.
    ///
    /// In order to initialise the window, an OHNativeWindow* handle must be provided through WindowTraits,
    /// provided via the value "nativeWindow".
    ///
    /// ```
    /// // void OH_main(struct OH_NativeActivity* activity)
    /// auto traits = vsg::WindowTraits::create();
    /// traits->setValue("nativeWindow", activity->window);
    /// auto window = vsg::Window::create(traits);
    /// ```
    ///
    /// This window handle may also be provided through WindowTraits::nativeWindow however due to
    /// the way OpenHarmony loads shared libraries this is likely to encounter duplicate typeinfo for
    /// OHNativeWindow, and as a result can throw a std::bad_any_cast on later SDK versions and some
    /// system architectures.
    class OHOS_Window : public vsg::Inherit<vsg::Window, OHOS_Window>
    {
    public:
        OHOS_Window(vsg::ref_ptr<vsg::WindowTraits> traits);
        OHOS_Window() = delete;
        OHOS_Window(const OHOS_Window&) = delete;
        OHOS_Window operator=(const OHOS_Window&) = delete;

        const char* instanceExtensionSurfaceName() const override { return "VK_OHOS_surface"; }

        bool valid() const override { return _window; }

        bool pollEvents(vsg::UIEvents& events) override;

        void resize() override;

        bool handleOHOSInputEvent(ArkUI_NodeEvent* event);

    protected:
        virtual ~OHOS_Window();

        void _initSurface() override;

        OHNativeWindow* _window = nullptr;

        int64_t _first_ohos_timestamp = 0;
        vsg::clock::time_point _first_ohos_time_point;

        vsg::ref_ptr<KeyboardMap> _keyboard;
        KeyMetaState _keyMetaState;
    };

} // namespace vsgOHOS

EVSG_type_name(vsgOHOS::OHOS_Window);