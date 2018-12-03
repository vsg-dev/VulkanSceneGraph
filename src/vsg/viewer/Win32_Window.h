#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#define VK_USE_PLATFORM_WIN32_KHR

#include <vsg/viewer/Window.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include <windows.h>
#include <windowsx.h>

#include <iostream>

namespace vsgWin32
{
    extern vsg::Names getInstanceExtensions();

    class KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using VirtualKeyToKeySymbolMap = std::map<uint16_t, vsg::KeySymbol>;

        vsg::KeySymbol getKeySymbol(WPARAM wParam, LPARAM lParam, bool applyModifier)
        {
            uint32_t modifierMask = 0;

            bool rightSide = (lParam & 0x01000000) != 0;
            int virtualKey = ::MapVirtualKeyEx((lParam >> 16) & 0xff, 3, ::GetKeyboardLayout(0));

            std::cout << "MapVirtualKeyEx 0x" << std::hex << virtualKey << std::endl;

            auto itr = _keycodeMap.find(virtualKey);
            if (itr == _keycodeMap.end()) return vsg::KEY_Undefined;


            switch (virtualKey)
            {
                case VK_LSHIFT:
                case VK_RSHIFT:
                    modifierMask |= vsg::KeyModifier::MODKEY_Shift;
                    break;

                case VK_LCONTROL:
                case VK_RCONTROL:
                    modifierMask |= vsg::KeyModifier::MODKEY_Control;
                    break;

                case VK_LMENU:
                case VK_RMENU:
                    modifierMask |= vsg::KeyModifier::MODKEY_Alt;
                    break;

                //////////////////
                default          :
                //////////////////

                    virtualKey = wParam;
                    break;
            }

            if (keyState[VK_CAPITAL] & 0x01) modifierMask |= osgGA::GUIEventAdapter::MODKEY_CAPS_LOCK;
            if (keyState[VK_NUMLOCK] & 0x01) modifierMask |= osgGA::GUIEventAdapter::MODKEY_NUM_LOCK;


            vsg::KeySymbol baseKeySymbol = itr->second;

            BYTE keyState[256];
            if (virtualKey==0 || !::GetKeyboardState(keyState))
            {
                return vsg::KEY_Undefined;
            }

            char asciiKey[2];
            int numChars = ::ToAscii(wParam, (lParam>>16)&0xff, keyState, reinterpret_cast<WORD*>(asciiKey), 0);
            if (numChars>0) baseKeySymbol = (vsg::KeySymbol)asciiKey[0];

            std::cout << "moded ascii: " << asciiKey << std::endl;

            return baseKeySymbol;

            /*auto itr = _keycodeMap.find(KeycodeModifier(keycode, 0));
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
            return vsg::KEY_Undefined;*/
        }

    protected:
        VirtualKeyToKeySymbolMap _keycodeMap;
    };


    vsg::ButtonMask getButtonMask(WPARAM wParam)
    {
        auto mask = (wParam & MK_LBUTTON ? vsg::ButtonMask::BUTTON_MASK_1 : 0) | (wParam & MK_RBUTTON ? vsg::ButtonMask::BUTTON_MASK_2 : 0) | (wParam & MK_MBUTTON ? vsg::ButtonMask::BUTTON_MASK_3 : 0) |
                    (wParam & MK_XBUTTON1 ? vsg::ButtonMask::BUTTON_MASK_4 : 0) | (wParam & MK_XBUTTON2 ? vsg::ButtonMask::BUTTON_MASK_5 : 0);
        return (vsg::ButtonMask)mask;
    }

    int getButtonEventDetail(UINT buttonMsg)
    {
        return buttonMsg == WM_LBUTTONDOWN ? 1 : (buttonMsg == WM_RBUTTONDOWN ? 2 : buttonMsg == WM_MBUTTONDOWN ? 3 : (buttonMsg == WM_XBUTTONDOWN ? 4 : 0)); // need to determine x1, x2
    }

    class Win32_Window : public vsg::Window
    {
    public:
        Win32_Window() = delete;
        Win32_Window(const Win32_Window&) = delete;
        Win32_Window operator=(const Win32_Window&) = delete;

        static Result create(const Traits& traits, bool debugLayer = false, bool apiDumpLayer = false, vsg::AllocationCallbacks* allocator = nullptr);

        bool valid() const override { return _window && !_shouldClose; }

        bool pollEvents(vsg::Events& events) override;

        bool resized() const override;

        void resize() override;

        operator HWND() { return _window; }
        operator const HWND() const { return _window; }

        LRESULT handleWin32Messages(UINT msg, WPARAM wParam, LPARAM lParam);

    protected:
        virtual ~Win32_Window();

        Win32_Window(const Window::Traits& traits, bool debugLayer = false, bool apiDumpLayer = false, vsg::AllocationCallbacks* allocator = nullptr);

        HWND _window;
        bool _shouldClose;

        vsg::Events _bufferedEvents;
        vsg::ref_ptr<KeyboardMap> _keyboard;
    };

} // namespace vsgWin32
