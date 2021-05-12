#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/core/observer_ptr.h>
#include <vsg/ui/WindowEvent.h>
#include <vsg/viewer/Window.h>

namespace vsg
{

    enum KeySymbol : uint16_t
    {
        KEY_Undefined = 0x0,

        KEY_Space = 0x20,

        KEY_0 = '0',
        KEY_1 = '1',
        KEY_2 = '2',
        KEY_3 = '3',
        KEY_4 = '4',
        KEY_5 = '5',
        KEY_6 = '6',
        KEY_7 = '7',
        KEY_8 = '8',
        KEY_9 = '9',

        KEY_a = 'a',
        KEY_b = 'b',
        KEY_c = 'c',
        KEY_d = 'd',
        KEY_e = 'e',
        KEY_f = 'f',
        KEY_g = 'g',
        KEY_h = 'h',
        KEY_i = 'i',
        KEY_j = 'j',
        KEY_k = 'k',
        KEY_l = 'l',
        KEY_m = 'm',
        KEY_n = 'n',
        KEY_o = 'o',
        KEY_p = 'p',
        KEY_q = 'q',
        KEY_r = 'r',
        KEY_s = 's',
        KEY_t = 't',
        KEY_u = 'u',
        KEY_v = 'v',
        KEY_w = 'w',
        KEY_x = 'x',
        KEY_y = 'y',
        KEY_z = 'z',

        KEY_A = 'A',
        KEY_B = 'B',
        KEY_C = 'C',
        KEY_D = 'D',
        KEY_E = 'E',
        KEY_F = 'F',
        KEY_G = 'G',
        KEY_H = 'H',
        KEY_I = 'I',
        KEY_J = 'J',
        KEY_K = 'K',
        KEY_L = 'L',
        KEY_M = 'M',
        KEY_N = 'N',
        KEY_O = 'O',
        KEY_P = 'P',
        KEY_Q = 'Q',
        KEY_R = 'R',
        KEY_S = 'S',
        KEY_T = 'T',
        KEY_U = 'U',
        KEY_V = 'V',
        KEY_W = 'W',
        KEY_X = 'X',
        KEY_Y = 'Y',
        KEY_Z = 'Z',

        KEY_Exclaim = 0x21,
        KEY_Quotedbl = 0x22,
        KEY_Hash = 0x23,
        KEY_Dollar = 0x24,
        KEY_Percent = 0x25,
        KEY_Ampersand = 0x26,
        KEY_Quote = 0x27,
        KEY_Leftparen = 0x28,
        KEY_Rightparen = 0x29,
        KEY_Asterisk = 0x2A,
        KEY_Plus = 0x2B,
        KEY_Comma = 0x2C,
        KEY_Minus = 0x2D,
        KEY_Period = 0x2E,
        KEY_Slash = 0x2F,
        KEY_Colon = 0x3A,
        KEY_Semicolon = 0x3B,
        KEY_Less = 0x3C,
        KEY_Equals = 0x3D,
        KEY_Greater = 0x3E,
        KEY_Question = 0x3F,
        KEY_At = 0x40,
        KEY_Leftbracket = 0x5B,
        KEY_Backslash = 0x5C,
        KEY_Rightbracket = 0x5D,
        KEY_Caret = 0x5E,
        KEY_Underscore = 0x5F,
        KEY_Backquote = 0x60,
        KEY_Leftcurlybracket = 0x7B,
        KEY_Verticalslash = 0x7C,
        KEY_Rightcurlybracket = 0x7D,
        KEY_Tilde = 0x7E,

        KEY_BackSpace = 0xFF08, /* back space, back char */
        KEY_Tab = 0xFF09,
        KEY_Linefeed = 0xFF0A, /* Linefeed, LF */
        KEY_Clear = 0xFF0B,
        KEY_Return = 0xFF0D, /* Return, enter */
        KEY_Pause = 0xFF13,  /* Pause, hold */
        KEY_Scroll_Lock = 0xFF14,
        KEY_Sys_Req = 0xFF15,
        KEY_Escape = 0xFF1B,
        KEY_Delete = 0xFFFF, /* Delete, rubout */

        /* Cursor control & motion */

        KEY_Home = 0xFF50,
        KEY_Left = 0xFF51,  /* Move left, left arrow */
        KEY_Up = 0xFF52,    /* Move up, up arrow */
        KEY_Right = 0xFF53, /* Move right, right arrow */
        KEY_Down = 0xFF54,  /* Move down, down arrow */
        KEY_Prior = 0xFF55, /* Prior, previous */
        KEY_Page_Up = 0xFF55,
        KEY_Next = 0xFF56, /* Next */
        KEY_Page_Down = 0xFF56,
        KEY_End = 0xFF57,   /* EOL */
        KEY_Begin = 0xFF58, /* BOL */

        /* Misc Functions */

        KEY_Select = 0xFF60, /* Select, mark */
        KEY_Print = 0xFF61,
        KEY_Execute = 0xFF62, /* Execute, run, do */
        KEY_Insert = 0xFF63,  /* Insert, insert here */
        KEY_Undo = 0xFF65,    /* Undo, oops */
        KEY_Redo = 0xFF66,    /* redo, again */
        KEY_Menu = 0xFF67,    /* On Windows, this is VK_APPS, the context-menu key */
        KEY_Find = 0xFF68,    /* Find, search */
        KEY_Cancel = 0xFF69,  /* Cancel, stop, abort, exit */
        KEY_Help = 0xFF6A,    /* Help */
        KEY_Break = 0xFF6B,
        KEY_Mode_switch = 0xFF7E,   /* Character set switch */
        KEY_Script_switch = 0xFF7E, /* Alias for mode_switch */
        KEY_Num_Lock = 0xFF7F,

        /* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

        KEY_KP_Space = 0xFF80, /* space */
        KEY_KP_Tab = 0xFF89,
        KEY_KP_Enter = 0xFF8D, /* enter */
        KEY_KP_F1 = 0xFF91,    /* PF1, KP_A, ... */
        KEY_KP_F2 = 0xFF92,
        KEY_KP_F3 = 0xFF93,
        KEY_KP_F4 = 0xFF94,
        KEY_KP_Home = 0xFF95,
        KEY_KP_Left = 0xFF96,
        KEY_KP_Up = 0xFF97,
        KEY_KP_Right = 0xFF98,
        KEY_KP_Down = 0xFF99,
        KEY_KP_Prior = 0xFF9A,
        KEY_KP_Page_Up = 0xFF9A,
        KEY_KP_Next = 0xFF9B,
        KEY_KP_Page_Down = 0xFF9B,
        KEY_KP_End = 0xFF9C,
        KEY_KP_Begin = 0xFF9D,
        KEY_KP_Insert = 0xFF9E,
        KEY_KP_Delete = 0xFF9F,
        KEY_KP_Equal = 0xFFBD, /* equals */
        KEY_KP_Multiply = 0xFFAA,
        KEY_KP_Add = 0xFFAB,
        KEY_KP_Separator = 0xFFAC, /* separator, often comma */
        KEY_KP_Subtract = 0xFFAD,
        KEY_KP_Decimal = 0xFFAE,
        KEY_KP_Divide = 0xFFAF,

        KEY_KP_0 = 0xFFB0,
        KEY_KP_1 = 0xFFB1,
        KEY_KP_2 = 0xFFB2,
        KEY_KP_3 = 0xFFB3,
        KEY_KP_4 = 0xFFB4,
        KEY_KP_5 = 0xFFB5,
        KEY_KP_6 = 0xFFB6,
        KEY_KP_7 = 0xFFB7,
        KEY_KP_8 = 0xFFB8,
        KEY_KP_9 = 0xFFB9,

        /*
            * Auxiliary Functions; note the duplicate definitions for left and right
            * function keys;  Sun keyboards and a few other manufactures have such
            * function key groups on the left and/or right sides of the keyboard.
            * We've not found a keyboard with more than 35 function keys total.
            */

        KEY_F1 = 0xFFBE,
        KEY_F2 = 0xFFBF,
        KEY_F3 = 0xFFC0,
        KEY_F4 = 0xFFC1,
        KEY_F5 = 0xFFC2,
        KEY_F6 = 0xFFC3,
        KEY_F7 = 0xFFC4,
        KEY_F8 = 0xFFC5,
        KEY_F9 = 0xFFC6,
        KEY_F10 = 0xFFC7,
        KEY_F11 = 0xFFC8,
        KEY_F12 = 0xFFC9,
        KEY_F13 = 0xFFCA,
        KEY_F14 = 0xFFCB,
        KEY_F15 = 0xFFCC,
        KEY_F16 = 0xFFCD,
        KEY_F17 = 0xFFCE,
        KEY_F18 = 0xFFCF,
        KEY_F19 = 0xFFD0,
        KEY_F20 = 0xFFD1,
        KEY_F21 = 0xFFD2,
        KEY_F22 = 0xFFD3,
        KEY_F23 = 0xFFD4,
        KEY_F24 = 0xFFD5,
        KEY_F25 = 0xFFD6,
        KEY_F26 = 0xFFD7,
        KEY_F27 = 0xFFD8,
        KEY_F28 = 0xFFD9,
        KEY_F29 = 0xFFDA,
        KEY_F30 = 0xFFDB,
        KEY_F31 = 0xFFDC,
        KEY_F32 = 0xFFDD,
        KEY_F33 = 0xFFDE,
        KEY_F34 = 0xFFDF,
        KEY_F35 = 0xFFE0,

        /* Modifiers */

        KEY_Shift_L = 0xFFE1,    /* Left shift */
        KEY_Shift_R = 0xFFE2,    /* Right shift */
        KEY_Control_L = 0xFFE3,  /* Left control */
        KEY_Control_R = 0xFFE4,  /* Right control */
        KEY_Caps_Lock = 0xFFE5,  /* Caps lock */
        KEY_Shift_Lock = 0xFFE6, /* Shift lock */

        KEY_Meta_L = 0xFFE7,  /* Left meta */
        KEY_Meta_R = 0xFFE8,  /* Right meta */
        KEY_Alt_L = 0xFFE9,   /* Left alt */
        KEY_Alt_R = 0xFFEA,   /* Right alt */
        KEY_Super_L = 0xFFEB, /* Left super */
        KEY_Super_R = 0xFFEC, /* Right super */
        KEY_Hyper_L = 0xFFED, /* Left hyper */
        KEY_Hyper_R = 0xFFEE  /* Right hyper */
    };

    enum KeyModifier : uint16_t
    {
        MODKEY_Shift = 1,
        MODKEY_CapsLock = 2,
        MODKEY_Control = 4,
        MODKEY_Alt = 8,
        MODKEY_NumLock = 16,
        MODKEY_Meta = 128
    };

    VSG_type_name(vsg::KeyEvent);
    class VSG_DECLSPEC KeyEvent : public Inherit<WindowEvent, KeyEvent>
    {
    public:
        KeyEvent() {}

        KeyEvent(Window* in_window, time_point in_time, KeySymbol in_keyBase, KeySymbol in_keyModified, KeyModifier in_modifier, uint32_t in_repeatCount = 0) :
            Inherit(in_window, in_time),
            keyBase(in_keyBase),
            keyModified(in_keyModified),
            keyModifier(in_modifier),
            repeatCount(in_repeatCount) {}

        KeySymbol keyBase = {};
        KeySymbol keyModified = {};
        KeyModifier keyModifier = {};
        uint32_t repeatCount = 0;

        void read(Input& input) override;
        void write(Output& output) const override;
    };

    VSG_type_name(vsg::KeyPressEvent);
    class KeyPressEvent : public Inherit<KeyEvent, KeyPressEvent>
    {
    public:
        KeyPressEvent() {}

        KeyPressEvent(Window* in_window, time_point in_time, KeySymbol in_keyBase, KeySymbol in_keyModified, KeyModifier in_modifier, uint32_t in_repeatCount = 0) :
            Inherit(in_window, in_time, in_keyBase, in_keyModified, in_modifier, in_repeatCount) {}
    };

    VSG_type_name(vsg::KeyReleaseEvent);
    class KeyReleaseEvent : public Inherit<KeyEvent, KeyReleaseEvent>
    {
    public:
        KeyReleaseEvent() {}

        KeyReleaseEvent(Window* in_window, time_point in_time, KeySymbol in_keyBase, KeySymbol in_keyModified, KeyModifier in_modifier, uint32_t in_repeatCount = 0) :
            Inherit(in_window, in_time, in_keyBase, in_keyModified, in_modifier, in_repeatCount) {}
    };

} // namespace vsg
