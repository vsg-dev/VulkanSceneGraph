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

#include <vsg/ui/DropEvent.h>
#include <vsg/app/Window.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>

#include <windows.h>
#include <windowsx.h>

#include <vulkan/vulkan_win32.h>

namespace vsgWin32
{
    class VSG_DECLSPEC KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using VirtualKeyToKeySymbolMap = std::map<uint16_t, vsg::KeySymbol>;

        bool getKeySymbol(WPARAM wParam, LPARAM lParam, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier)
        {
            uint16_t modifierMask = 0;

            // Get scancode
            uint16_t keyFlags = HIWORD(lParam);
            uint16_t scanCode = LOBYTE(lParam);
            bool isExtendedKey = (keyFlags & KF_EXTENDED) == KF_EXTENDED;

            if (isExtendedKey)
            {
                scanCode = MAKEWORD(scanCode, 0xE0);
            }

            // Get virtual key code
            uint32_t virtualKey = static_cast<uint32_t>(wParam);

            // Left and right modifier detection
            if (virtualKey == VK_SHIFT)
            {
                virtualKey = (scanCode == 0x2A) ? VK_RSHIFT : VK_LSHIFT;
            }
            else if (virtualKey == VK_CONTROL)
            {
                virtualKey = isExtendedKey ? VK_RCONTROL : VK_LCONTROL;
            }
            else if (virtualKey == VK_MENU)
            {
                virtualKey = isExtendedKey ? VK_RMENU : VK_LMENU;
            }

            auto itr = _vk2vsg.find(virtualKey);

            if (itr == _vk2vsg.end())
            {
                return false;
            }

            keySymbol = itr->second;

            // Get keys state
            BYTE keyState[256];

            if (!::GetKeyboardState(keyState))
            {
                return false;
            }

            // Modifier's flags setting
            if (keyState[VK_SHIFT] & 0x80) modifierMask |= vsg::KeyModifier::MODKEY_Shift;
            if (keyState[VK_CONTROL] & 0x80) modifierMask |= vsg::KeyModifier::MODKEY_Control;
            if (keyState[VK_MENU] & 0x80) modifierMask |= vsg::KeyModifier::MODKEY_Alt;
            if (keyState[VK_CAPITAL] & 0x01) modifierMask |= vsg::KeyModifier::MODKEY_CapsLock;
            if (keyState[VK_NUMLOCK] & 0x01) modifierMask |= vsg::KeyModifier::MODKEY_NumLock;

            keyModifier = static_cast<vsg::KeyModifier>(modifierMask);

            // Get Unicode symbol
            wchar_t unicodeChar[2] = {0};
            int numChars = ::ToUnicodeEx(virtualKey, scanCode, keyState, unicodeChar, 2, 0, ::GetKeyboardLayout(0));

            if (numChars == 1)
            {
                wchar_t ch = unicodeChar[0];

                modifiedKeySymbol = mapCharToKeySymbol(ch);
            }
            else
            {
                modifiedKeySymbol = keySymbol;
            }

            return true;
        }

    protected:

        vsg::KeySymbol mapCharToKeySymbol(wchar_t ch)
        {
            // Special characters (with and without Shift)
            switch (ch)
            {
            case L'\t': return vsg::KEY_Tab;
            case L'\r': return vsg::KEY_Return;
            }

            return static_cast<vsg::KeySymbol>(ch);
        }

        VirtualKeyToKeySymbolMap _vk2vsg;
    };

    inline vsg::ButtonMask getButtonMask(WPARAM wParam)
    {
        auto mask = (wParam & MK_LBUTTON ? vsg::ButtonMask::BUTTON_MASK_1 : 0) | (wParam & MK_MBUTTON ? vsg::ButtonMask::BUTTON_MASK_2 : 0) | (wParam & MK_RBUTTON ? vsg::ButtonMask::BUTTON_MASK_3 : 0) |
                    (wParam & MK_XBUTTON1 ? vsg::ButtonMask::BUTTON_MASK_4 : 0) | (wParam & MK_XBUTTON2 ? vsg::ButtonMask::BUTTON_MASK_5 : 0);
        return static_cast<vsg::ButtonMask>(mask);
    }

    inline uint32_t getButtonDownEventDetail(UINT buttonMsg, WORD wParamHi)
    {
        switch (buttonMsg)
        {
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN: return 1;
        case WM_MBUTTONDBLCLK:
        case WM_MBUTTONDOWN: return 2;
        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONDOWN: return 3;
        case WM_XBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
            if (wParamHi == XBUTTON1)
                return 4;
            else if (wParamHi == XBUTTON2)
                return 5;
            else
                return 0;
        default:
            return 0;
        }
    }

    inline uint32_t getButtonUpEventDetail(UINT buttonMsg, WORD wParamHi)
    {
        switch (buttonMsg)
        {
        case WM_LBUTTONUP: return 1;
        case WM_MBUTTONUP: return 2;
        case WM_RBUTTONUP: return 3;
        case WM_XBUTTONUP:
            if (wParamHi == XBUTTON1)
                return 4;
            else if (wParamHi == XBUTTON2)
                return 5;
            else
                return 0;
        default:
            return 0;
        }
    }

    /// Win32_Window implements Win32 specific window creation, event handling and vulkan Surface setup.
    class VSG_DECLSPEC Win32_Window : public vsg::Inherit<vsg::Window, Win32_Window>
    {
    public:
        Win32_Window(vsg::ref_ptr<vsg::WindowTraits> traits);
        Win32_Window() = delete;
        Win32_Window(const Win32_Window&) = delete;
        Win32_Window operator=(const Win32_Window&) = delete;

        const char* instanceExtensionSurfaceName() const override { return VK_KHR_WIN32_SURFACE_EXTENSION_NAME; }

        bool valid() const override { return _window; }

        bool visible() const override;

        void releaseWindow() override;

        bool pollEvents(vsg::UIEvents& events) override;

        void resize() override;

        operator HWND() const { return _window; }

        /// handle Win32 event messages, return true if handled.
        virtual bool handleWin32Messages(UINT msg, WPARAM wParam, LPARAM lParam);

    protected:
        virtual ~Win32_Window();

        void _initSurface() override;

        /// Register the window with OLE so that it is offered file drops.
        void _initDrop();
        void _shutdownDrop();

        HWND _window;
        bool _windowMapped = false;

        vsg::ref_ptr<KeyboardMap> _keyboard;

        // Drag and drop. The OLE callbacks only record what happened; the events are emitted from
        // pollEvents() so that nothing reaches the application from inside a window message.
        struct DropTarget;
        DropTarget* _dropTarget = nullptr;
        bool _dropHovering = false;
        bool _dropAccepted = false;
        vsg::Paths _dropPaths;
        int32_t _dropX = 0;
        int32_t _dropY = 0;

        // The hover event emitted last frame, kept so that the accept flag the application set on it
        // can be read back once the frame that handled it has finished.
        vsg::ref_ptr<vsg::DropHoverEvent> _dropHoverEvent;
    };

    /// Use GetLastError() and FormatMessageA(..) to get the error number and error message and store them in a vsg::Exception.
    extern VSG_DECLSPEC vsg::Exception getLastErrorAsException(const std::string_view& prefix = {});

} // namespace vsgWin32

EVSG_type_name(vsgWin32::Win32_Window);
