#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#define VK_USE_PLATFORM_WIN32_KHR

#ifndef NOMINMAX
#    define NOMINMAX
#endif

#include <vsg/app/Window.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include <windows.h>
#include <windowsx.h>

namespace vsgWin32
{
    class KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using VirtualKeyToKeySymbolMap = std::map<uint16_t, vsg::KeySymbol>;
        using ASCIIToVSGMap = std::map<uint16_t, vsg::KeySymbol>;

        bool getKeySymbol(WPARAM wParam, LPARAM lParam, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier)
        {
            uint16_t modifierMask = 0;

            // In this method we do the following
            // scan_code -> windows virtual key -> vsg key symbol
            
            //// We first use MapVirtualKeyEx to the Windows Virtual Key for the key that was pressed.
            uint32_t scan_code = (lParam >> 16) & 0xff;
            uint32_t virtualKey = ::MapVirtualKeyEx(scan_code, MAPVK_VSC_TO_VK_EX, ::GetKeyboardLayout(0));
            auto itr = _keycodeMap.find(virtualKey);
            if (itr == _keycodeMap.end())
            {
                // since we could not find the vsg equivalent of the windows virtual key
                // we return false. If indeed it should be recognized by vsg, ensure that it is in _keycodeMap.
                // so that we do not return false.
                return false;
            }

            keySymbol = itr->second; // unmodified key symbol.
            //// Now we figure out the "actual" semantic key value  
            // We start with assuming that the modifiedKeySymbol is the same as keySymbol
            // If it i
            // and figure out if there were any modifiers pressed at the same time.
            modifiedKeySymbol = itr->second;

            BYTE keyState[256];
            if (virtualKey == 0 || !::GetKeyboardState(keyState))
            {
                return false;
            }

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

            default:
                virtualKey = static_cast<int>(wParam);
                break;
            }

            if (keyState[VK_CAPITAL] & 0x01) modifierMask |= vsg::KeyModifier::MODKEY_CapsLock;
            if (keyState[VK_NUMLOCK] & 0x01) modifierMask |= vsg::KeyModifier::MODKEY_NumLock;

            keyModifier = static_cast<vsg::KeyModifier>(modifierMask);

            // our actual keystroke is what we get after the ::ToAscii call
            char asciiKey[2];
            int32_t numChars = ::ToAscii(static_cast<UINT>(wParam), (lParam >> 16) & 0xff, keyState, reinterpret_cast<WORD*>(asciiKey), 0);

            if (numChars > 0)
            {
                // its ascii so lets look for it in our ascii translation map
                ASCIIToVSGMap::iterator itr = _ascii2vsgmap.find(asciiKey[0]);
                if (itr != _ascii2vsgmap.end())
                {
                    modifiedKeySymbol = itr->second;
                }
                else
                {
                    // its ascii but is missing from our _ascii2vsgmap
                    // we should really add this key to the _ascii2vsgmap
                    modifiedKeySymbol = vsg::KEY_Undefined;
                }
            }
            return true;
        }

    protected:
        VirtualKeyToKeySymbolMap _keycodeMap;
        ASCIIToVSGMap _ascii2vsgmap;
    };

    vsg::ButtonMask getButtonMask(WPARAM wParam)
    {
        auto mask = (wParam & MK_LBUTTON ? vsg::ButtonMask::BUTTON_MASK_1 : 0) | (wParam & MK_MBUTTON ? vsg::ButtonMask::BUTTON_MASK_2 : 0) | (wParam & MK_RBUTTON ? vsg::ButtonMask::BUTTON_MASK_3 : 0) |
                    (wParam & MK_XBUTTON1 ? vsg::ButtonMask::BUTTON_MASK_4 : 0) | (wParam & MK_XBUTTON2 ? vsg::ButtonMask::BUTTON_MASK_5 : 0);
        return static_cast<vsg::ButtonMask>(mask);
    }

    uint32_t getButtonDownEventDetail(UINT buttonMsg)
    {
        return buttonMsg == WM_LBUTTONDOWN ? 1 : (buttonMsg == WM_MBUTTONDOWN ? 2 : buttonMsg == WM_RBUTTONDOWN ? 3
                                                                                                                : (buttonMsg == WM_XBUTTONDOWN ? 4 : 0)); // need to determine x1, x2
    }

    uint32_t getButtonUpEventDetail(UINT buttonMsg)
    {
        return buttonMsg == WM_LBUTTONUP ? 1 : (buttonMsg == WM_MBUTTONUP ? 2 : buttonMsg == WM_RBUTTONUP ? 3
                                                                                                          : (buttonMsg == WM_XBUTTONUP ? 4 : 0));
    }

    /// Win32_Window implements Win32 specific window creation, event handling and vulkan Surface setup.
    class Win32_Window : public vsg::Inherit<vsg::Window, Win32_Window>
    {
    public:
        Win32_Window(vsg::ref_ptr<vsg::WindowTraits> traits);
        Win32_Window() = delete;
        Win32_Window(const Win32_Window&) = delete;
        Win32_Window operator=(const Win32_Window&) = delete;

        const char* instanceExtensionSurfaceName() const override { return VK_KHR_WIN32_SURFACE_EXTENSION_NAME; }

        bool valid() const override { return _window; }

        bool visible() const override;

        bool pollEvents(vsg::UIEvents& events) override;

        void resize() override;

        operator HWND() { return _window; }
        operator const HWND() const { return _window; }

        LRESULT handleWin32Messages(UINT msg, WPARAM wParam, LPARAM lParam);

    protected:
        virtual ~Win32_Window();

        void _initSurface() override;

        HWND _window;
        bool _windowMapped = false;

        vsg::ref_ptr<KeyboardMap> _keyboard;
    };

} // namespace vsgWin32

EVSG_type_name(vsgWin32::Win32_Window);
