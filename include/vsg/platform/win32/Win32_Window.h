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

#include <vulkan/vulkan_win32.h>

namespace vsgWin32
{
    class KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using VirtualKeyToKeySymbolMap = std::map<uint16_t, vsg::KeySymbol>;

        bool getKeySymbol(WPARAM wParam, LPARAM lParam, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier)
        {
            uint16_t modifierMask = 0;
            uint32_t virtualKey = ::MapVirtualKeyEx((lParam >> 16) & 0xff, MAPVK_VSC_TO_VK_EX, ::GetKeyboardLayout(0));
            auto itr = _vk2vsg.find(virtualKey);

            if (itr == _vk2vsg.end())
            {
                // What ever the code was in lParam should translate to a Virtual Key that we know of in _vk2vsg
                // If we cannot find it, we simply return.
                return false;
            }

            // This is the base-key that was pressed. (i.e., the VSG enum of the physical key pressed).
            keySymbol = itr->second;

            // Look for any modifiers that may be active.
            BYTE keyState[256];
            if (virtualKey == 0 || !::GetKeyboardState(keyState))
            {
                // if virtualKey was undefined or we could not get the keyboard state, simply return.
                return false;
            }

            // If any of the specific left/right modifier keys are active
            // add the side-independent vsg modifier to the modifier Mask
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

            // Check if caps lock or numlock is toggled.
            if (keyState[VK_CAPITAL] & 0x01) modifierMask |= vsg::KeyModifier::MODKEY_CapsLock;
            if (keyState[VK_NUMLOCK] & 0x01) modifierMask |= vsg::KeyModifier::MODKEY_NumLock;

            // This is the final keyModifier
            keyModifier = static_cast<vsg::KeyModifier>(modifierMask);

            // The actual keystroke is what we get after the ::ToAscii call
            char asciiKey[2];
            int32_t numChars = ::ToAscii(static_cast<UINT>(wParam), (lParam >> 16) & 0xff, keyState, reinterpret_cast<WORD*>(asciiKey), 0);
            if (numChars == 1)
            {
                // it is indeed an ascii character. 0-127
                modifiedKeySymbol = static_cast<vsg::KeySymbol>(asciiKey[0]);
            }
            else
            {
                // otherwise treat the modifiedKeySymbol as the same as the keySymbol.
                modifiedKeySymbol = keySymbol;
            }

            return true;
        }

    protected:
        VirtualKeyToKeySymbolMap _vk2vsg;
    };

    inline vsg::ButtonMask getButtonMask(WPARAM wParam)
    {
        auto mask = (wParam & MK_LBUTTON ? vsg::ButtonMask::BUTTON_MASK_1 : 0) | (wParam & MK_MBUTTON ? vsg::ButtonMask::BUTTON_MASK_2 : 0) | (wParam & MK_RBUTTON ? vsg::ButtonMask::BUTTON_MASK_3 : 0) |
                    (wParam & MK_XBUTTON1 ? vsg::ButtonMask::BUTTON_MASK_4 : 0) | (wParam & MK_XBUTTON2 ? vsg::ButtonMask::BUTTON_MASK_5 : 0);
        return static_cast<vsg::ButtonMask>(mask);
    }

    inline uint32_t getButtonDownEventDetail(UINT buttonMsg)
    {
        return buttonMsg == WM_LBUTTONDOWN ? 1 : (buttonMsg == WM_MBUTTONDOWN ? 2 : buttonMsg == WM_RBUTTONDOWN ? 3
                                                                                                                : (buttonMsg == WM_XBUTTONDOWN ? 4 : 0)); // need to determine x1, x2
    }

    inline uint32_t getButtonUpEventDetail(UINT buttonMsg)
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
