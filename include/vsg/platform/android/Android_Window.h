#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#define VK_USE_PLATFORM_ANDROID_KHR

#include <vsg/viewer/Window.h>
#include <vsg/ui/KeyEvent.h>

#include <android/input.h>
#include <android/native_window.h>

namespace vsgAndroid
{
    class KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using AKeyCodeToKeySymbolMap = std::map<uint32_t, vsg::KeySymbol>;

        bool getKeySymbol(uint32_t keycode, uint32_t metastate, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier)
        {
            auto itr = _keycodeMap.find(keycode);
            if (itr == _keycodeMap.end()) return false;

            keySymbol = itr->second;
            modifiedKeySymbol = keySymbol;

            uint16_t modifierMask = 0;

            if (metastate & AMETA_ALT_ON) modifierMask |= vsg::KeyModifier::MODKEY_Alt;
            if (metastate & AMETA_CTRL_ON) modifierMask |= vsg::KeyModifier::MODKEY_Control;
            if (metastate & AMETA_SHIFT_ON) modifierMask |= vsg::KeyModifier::MODKEY_Shift;
            if (metastate & AMETA_CAPS_LOCK_ON) modifierMask |= vsg::KeyModifier::MODKEY_CapsLock;
            if (metastate & AMETA_NUM_LOCK_ON) modifierMask |= vsg::KeyModifier::MODKEY_NumLock;

            keyModifier = (vsg::KeyModifier) modifierMask;

            // need to get the modified key somehow but seems we may need to talk to java for that :(

            return true;
        }

    protected:
        AKeyCodeToKeySymbolMap _keycodeMap;
    };


    class Android_Window : public vsg::Inherit<vsg::Window, Android_Window>
    {
    public:

        Android_Window(vsg::ref_ptr<vsg::WindowTraits> traits);
        Android_Window() = delete;
        Android_Window(const Android_Window&) = delete;
        Android_Window operator = (const Android_Window&) = delete;

        const char* instanceExtensionSurfaceName() const override { return "VK_KHR_android_surface"; }

        bool valid() const override { return _window; }

        bool pollEvents(vsg::UIEvents& events) override;

        void resize() override;

        bool handleAndroidInputEvent(AInputEvent* anEvent);

    protected:
        virtual ~Android_Window();

        void _initSurface() override;

        ANativeWindow* _window;

        int64_t _first_android_timestamp = 0;
        vsg::clock::time_point _first_android_time_point;

        vsg::ref_ptr<KeyboardMap> _keyboard;
    };

} // namespace vsgAndroid

EVSG_type_name(vsgAndroid::Android_Window);
