/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/platform/win32/Win32_Window.h>
#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>
#include <vsg/io/Options.h>
#include <vsg/platform/win32/Win32_Window.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/vk/Extensions.h>

using namespace vsg;
using namespace vsgWin32;

namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to a Win32_Window
    ref_ptr<Window> Window::create(vsg::ref_ptr<WindowTraits> traits)
    {
        return vsgWin32::Win32_Window::create(traits);
    }

} // namespace vsg

namespace vsgWin32
{
    class VSG_DECLSPEC Win32Surface : public vsg::Surface
    {
    public:
        Win32Surface(vsg::Instance* instance, HWND window) :
            vsg::Surface(VK_NULL_HANDLE, instance)
        {
            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.hinstance = ::GetModuleHandle(NULL);
            surfaceCreateInfo.hwnd = window;
            surfaceCreateInfo.pNext = nullptr;

            auto result = vkCreateWin32SurfaceKHR(*instance, &surfaceCreateInfo, _instance->getAllocationCallbacks(), &_surface);
        }
    };

    // our windows events callback
    LRESULT CALLBACK Win32WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        Win32_Window* win = reinterpret_cast<Win32_Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (win != nullptr) return win->handleWin32Messages(msg, wParam, lParam);
        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }

} // namespace vsgWin32

KeyboardMap::KeyboardMap()
{

    //| Dec | VSG                       | Hex | Oct | Char | Description                      |
    //|---- |---------------------------|-----|-----|------|----------------------------------|
    _ascii2vsgmap = {
        {0, KEY_Undefined},           //| 00  | 000 | ^@    | Null (NUL)                      |
        {1, KEY_Undefined},           //| 01  | 001 | ^A    | Start of heading (SOH)          |
        {2, KEY_Undefined},           //| 02  | 002 | ^B    | Start of text (STX)             |
        {3, KEY_Undefined},           //| 03  | 003 | ^C    | End of text (ETX)               |
        {4, KEY_Undefined},           //| 04  | 004 | ^D    | End of transmission (EOT)       |
        {5, KEY_Undefined},           //| 05  | 005 | ^E    | Enquiry (ENQ)                   |
        {6, KEY_Undefined},           //| 06  | 006 | ^F    | Acknowledge (ACK)               |
        {7, KEY_Undefined},           //| 07  | 007 | ^G    | Bell (BEL)                      |
        {8, KEY_Undefined},           //| 08  | 010 | ^H    | Backspace (BS)                  |
        {9, KEY_Undefined},           //| 09  | 011 | ^I    | Horizontal tab (HT)             |
        {10, KEY_Undefined},          //| 0A  | 012 | ^J    | Line feed (LF)                  |
        {11, KEY_Undefined},          //| 0B  | 013 | ^K    | Vertical tab (VT)               |
        {12, KEY_Undefined},          //| 0C  | 014 | ^L    | New page/form feed (FF)         |
        {13, KEY_Undefined},          //| 0D  | 015 | ^M    | Carriage return (CR)            |
        {14, KEY_Undefined},          //| 0E  | 016 | ^N    | Shift out (SO)                  |
        {15, KEY_Undefined},          //| 0F  | 017 | ^O    | Shift in (SI)                   |
        {16, KEY_Undefined},          //| 10  | 020 | ^P    | Data link escape (DLE)          |
        {17, KEY_Undefined},          //| 11  | 021 | ^Q    | Device control 1 (DC1)          |
        {18, KEY_Undefined},          //| 12  | 022 | ^R    | Device control 2 (DC2)          |
        {19, KEY_Undefined},          //| 13  | 023 | ^S    | Device control 3 (DC3)          |
        {20, KEY_Undefined},          //| 14  | 024 | ^T    | Device control 4 (DC4)          |
        {21, KEY_Undefined},          //| 15  | 025 | ^U    | Negative acknowledge (NAK)      |
        {22, KEY_Undefined},          //| 16  | 026 | ^V    | Synchronous idle (SYN)          |
        {23, KEY_Undefined},          //| 17  | 027 | ^W    | End of transmission block (ETB) |
        {24, KEY_Undefined},          //| 18  | 030 | ^X    | Cancel (CAN)                    |
        {25, KEY_Undefined},          //| 19  | 031 | ^Y    | End of medium (EM)              |
        {26, KEY_Undefined},          //| 1A  | 032 | ^Z    | Substitute (SUB)                |
        {27, KEY_Undefined},          //| 1B  | 033 | ^[    | Escape (ESC)                    |
        {28, KEY_Undefined},          //| 1C  | 034 | ^\    | File separator (FS)             |
        {29, KEY_Undefined},          //| 1D  | 035 | ^]    | Group separator (GS)            |
        {30, KEY_Undefined},          //| 1E  | 036 | ^^    | Record separator (RS)           |
        {31, KEY_Undefined},          //| 1F  | 037 | ^_    | Unit separator (US)             |
        {32, KEY_Undefined},          //| 20  | 040 | Space |                                 |
        {33, KEY_Exclaim},            //| 21  | 041 | !     | Exclamation mark                |
        {34, KEY_Exclaim},            //| 22  | 042 | "     | Quotation mark/Double quote     |
        {35, KEY_Hash},               //| 23  | 043 | #     | Number sign                     |
        {36, KEY_Dollar},             //| 24  | 044 | $     | Dollar sign                     |
        {37, KEY_Percent},            //| 25  | 045 | %     | Percent sign                    |
        {38, KEY_Ampersand},          //| 26  | 046 | &amp; | Ampersand                       |
        {39, KEY_Quote},              //| 27  | 047 | '     | Apostrophe/Single quote         |
        {40, KEY_Leftparen},          //| 28  | 050 | (     | Left parenthesis                |
        {41, KEY_Rightparen},         //| 29  | 051 | )     | Right parenthesis               |
        {42, KEY_Asterisk},           //| 2A  | 052 | *     | Asterisk                        |
        {43, KEY_Plus},               //| 2B  | 053 | +     | Plus sign                       |
        {44, KEY_Comma},              //| 2C  | 054 | ,     | Comma                           |
        {45, KEY_Minus},              //| 2D  | 055 | -     | Hyphen/Minus                    |
        {46, KEY_Period},             //| 2E  | 056 | .     | Full stop/Period                |
        {47, KEY_Slash},              //| 2F  | 057 | /     | Solidus/Slash                   |
        {48, KEY_0},                  //| 30  | 060 | 0     | Digit zero                      |
        {49, KEY_1},                  //| 31  | 061 | 1     | Digit one                       |
        {50, KEY_2},                  //| 32  | 062 | 2     | Digit two                       |
        {51, KEY_3},                  //| 33  | 063 | 3     | Digit three                     |
        {52, KEY_4},                  //| 34  | 064 | 4     | Digit four                      |
        {53, KEY_5},                  //| 35  | 065 | 5     | Digit five                      |
        {54, KEY_6},                  //| 36  | 066 | 6     | Digit six                       |
        {55, KEY_7},                  //| 37  | 067 | 7     | Digit seven                     |
        {56, KEY_8},                  //| 38  | 070 | 8     | Digit eight                     |
        {57, KEY_9},                  //| 39  | 071 | 9     | Digit nine                      |
        {58, KEY_Colon},              //| 3A  | 072 | :     | Colon                           |
        {59, KEY_Semicolon},          //| 3B  | 073 | ;     | Semicolon                       |
        {60, KEY_Less},               //| 3C  | 074 | <     | Less-than sign                  |
        {61, KEY_Equals},             //| 3D  | 075 | =     | Equal/Equality sign             |
        {62, KEY_Greater},            //| 3E  | 076 | >     | Greater-than sign               |
        {63, KEY_Question},           //| 3F  | 077 | ?     | Question mark                   |
        {64, KEY_At},                 //| 40  | 100 | @     | Commercial at/At sign           |
        {65, KEY_A},                  //| 41  | 101 | A     | Latin capital letter A          |
        {66, KEY_B},                  //| 42  | 102 | B     | Latin capital letter B          |
        {67, KEY_C},                  //| 43  | 103 | C     | Latin capital letter C          |
        {68, KEY_D},                  //| 44  | 104 | D     | Latin capital letter D          |
        {69, KEY_E},                  //| 45  | 105 | E     | Latin capital letter E          |
        {70, KEY_F},                  //| 46  | 106 | F     | Latin capital letter F          |
        {71, KEY_G},                  //| 47  | 107 | G     | Latin capital letter G          |
        {72, KEY_H},                  //| 48  | 110 | H     | Latin capital letter H          |
        {73, KEY_I},                  //| 49  | 111 | I     | Latin capital letter I          |
        {74, KEY_J},                  //| 4A  | 112 | J     | Latin capital letter J          |
        {75, KEY_K},                  //| 4B  | 113 | K     | Latin capital letter K          |
        {76, KEY_L},                  //| 4C  | 114 | L     | Latin capital letter L          |
        {77, KEY_M},                  //| 4D  | 115 | M     | Latin capital letter M          |
        {78, KEY_N},                  //| 4E  | 116 | N     | Latin capital letter N          |
        {79, KEY_O},                  //| 4F  | 117 | O     | Latin capital letter O          |
        {80, KEY_P},                  //| 50  | 120 | P     | Latin capital letter P          |
        {81, KEY_Q},                  //| 51  | 121 | Q     | Latin capital letter Q          |
        {82, KEY_R},                  //| 52  | 122 | R     | Latin capital letter R          |
        {83, KEY_S},                  //| 53  | 123 | S     | Latin capital letter S          |
        {84, KEY_T},                  //| 54  | 124 | T     | Latin capital letter T          |
        {85, KEY_U},                  //| 55  | 125 | U     | Latin capital letter U          |
        {86, KEY_V},                  //| 56  | 126 | V     | Latin capital letter V          |
        {87, KEY_W},                  //| 57  | 127 | W     | Latin capital letter W          |
        {88, KEY_X},                  //| 58  | 130 | X     | Latin capital letter X          |
        {89, KEY_Y},                  //| 59  | 131 | Y     | Latin capital letter Y          |
        {90, KEY_Z},                  //| 5A  | 132 | Z     | Latin capital letter Z          |
        {91, KEY_Leftbracket},        //| 5B  | 133 | [     | Left square bracket             |
        {92, KEY_Backslash},          //| 5C  | 134 | \     | Reverse solidus/Backslash       |
        {93, KEY_Rightbracket},       //| 5D  | 135 | ]     | Right square bracket            |
        {94, KEY_Caret},              //| 5E  | 136 | ^     | Circumflex accent/Caret         |
        {95, KEY_Underscore},         //| 5F  | 137 | _     | Underscore/Low line             |
        {96, KEY_Backquote},          //| 60  | 140 | `     | Grave accent                    |
        {97, KEY_a},                  //| 61  | 141 | a     | Latin small letter a            |
        {98, KEY_b},                  //| 62  | 142 | b     | Latin small letter b            |
        {99, KEY_c},                  //| 63  | 143 | c     | Latin small letter c            |
        {100, KEY_d},                 //| 64  | 144 | d     | Latin small letter d            |
        {101, KEY_e},                 //| 65  | 145 | e     | Latin small letter e            |
        {102, KEY_f},                 //| 66  | 146 | f     | Latin small letter f            |
        {103, KEY_g},                 //| 67  | 147 | g     | Latin small letter g            |
        {104, KEY_h},                 //| 68  | 150 | h     | Latin small letter h            |
        {105, KEY_i},                 //| 69  | 151 | i     | Latin small letter i            |
        {106, KEY_j},                 //| 6A  | 152 | j     | Latin small letter j            |
        {107, KEY_k},                 //| 6B  | 153 | k     | Latin small letter k            |
        {108, KEY_l},                 //| 6C  | 154 | l     | Latin small letter l            |
        {109, KEY_m},                 //| 6D  | 155 | m     | Latin small letter m            |
        {110, KEY_n},                 //| 6E  | 156 | n     | Latin small letter n            |
        {111, KEY_o},                 //| 6F  | 157 | o     | Latin small letter o            |
        {112, KEY_p},                 //| 70  | 160 | p     | Latin small letter p            |
        {113, KEY_q},                 //| 71  | 161 | q     | Latin small letter q            |
        {114, KEY_r},                 //| 72  | 162 | r     | Latin small letter r            |
        {115, KEY_s},                 //| 73  | 163 | s     | Latin small letter s            |
        {116, KEY_t},                 //| 74  | 164 | t     | Latin small letter t            |
        {117, KEY_u},                 //| 75  | 165 | u     | Latin small letter u            |
        {118, KEY_v},                 //| 76  | 166 | v     | Latin small letter v            |
        {119, KEY_w},                 //| 77  | 167 | w     | Latin small letter w            |
        {120, KEY_x},                 //| 78  | 170 | x     | Latin small letter x            |
        {121, KEY_y},                 //| 79  | 171 | y     | Latin small letter y            |
        {122, KEY_z},                 //| 7A  | 172 | z     | Latin small letter z            |
        {123, KEY_Leftcurlybracket},  //| 7B  | 173 | {     | Left curly bracket              |
        {124, KEY_Verticalslash},     //| 7C  | 174 | |     | Vertical line/Vertical bar      |
        {125, KEY_Rightcurlybracket}, //| 7D  | 175 | }     | Right curly bracket             |
        {126, KEY_Tilde},             //| 7E  | 176 | ~     | Tilde                           |
        {127, KEY_Undefined},         //| 7F  | 177 | DEL   | Delete (DEL)                    |
    };

    _keycodeMap =
        {
            {0x0, KEY_Undefined},

            {VK_SPACE, KEY_Space},

            {'0', KEY_0},
            {'1', KEY_1},
            {'2', KEY_2},
            {'3', KEY_3},
            {'4', KEY_4},
            {'5', KEY_5},
            {'6', KEY_6},
            {'7', KEY_7},
            {'8', KEY_8},
            {'9', KEY_9},

            // In this map it is incorrect to have map-keys that are not
            // not explicitly listed or mentioned in Windows 10 SDKs WinUser.h
            //
            // Here is an example that illustrates the issue.
            // Lower case 'p' generates a keycode dec(112), i.e., hex(0x70)
            // while VK_F1 also has value dec(112) i.e., hex(0x70)
            // This means the initial mapping of
            // {'p',   KEY_p} is overwritten by the later mapping
            // {VK_F1, KEY_F1},
            // BottomLine: The map is <WindowsVirtualKey> -> <VSG KeyCode>
            // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
            //
            //{'a', KEY_a},
            //{'b', KEY_b},
            //{'c', KEY_c},
            //{'d', KEY_d},
            //{'e', KEY_e},
            //{'f', KEY_f},
            //{'g', KEY_g},
            //{'h', KEY_h},
            //{'i', KEY_i},
            //{'j', KEY_j},
            //{'k', KEY_k},
            //{'l', KEY_l},
            //{'m', KEY_m},
            //{'n', KEY_n},
            //{'o', KEY_o},
            //{'p', KEY_p},
            //{'q', KEY_q},
            //{'r', KEY_r},
            //{'s', KEY_s},
            //{'t', KEY_t},
            //{'u', KEY_u},
            //{'z', KEY_v},
            //{'w', KEY_w},
            //{'x', KEY_x},
            //{'y', KEY_y},
            //{'z', KEY_z},

            {'A', KEY_A},
            {'B', KEY_B},
            {'C', KEY_C},
            {'D', KEY_D},
            {'E', KEY_E},
            {'F', KEY_F},
            {'G', KEY_G},
            {'H', KEY_H},
            {'I', KEY_I},
            {'J', KEY_J},
            {'K', KEY_K},
            {'L', KEY_L},
            {'M', KEY_M},
            {'N', KEY_N},
            {'O', KEY_O},
            {'P', KEY_P},
            {'Q', KEY_Q},
            {'R', KEY_R},
            {'S', KEY_S},
            {'T', KEY_T},
            {'U', KEY_U},
            {'V', KEY_V},
            {'W', KEY_W},
            {'X', KEY_X},
            {'Y', KEY_Y},
            {'Z', KEY_Z},

            /* Cursor control & motion */

            {VK_HOME, KEY_Home},
            {VK_LEFT, KEY_Left},   /* Move left, left arrow */
            {VK_UP, KEY_Up},       /* Move up, up arrow */
            {VK_RIGHT, KEY_Right}, /* Move right, right arrow */
            {VK_DOWN, KEY_Down},   /* Move down, down arrow */
            {VK_PRIOR, KEY_Prior}, /* Prior, previous */
            //{ VK_, KEY_Page_Up = 0xFF55,
            {VK_NEXT, KEY_Next}, /* Next */
            //KEY_Page_Down = 0xFF56,
            {VK_END, KEY_End}, /* EOL */
            //{ KEY_Begin = 0xFF58, /* BOL */

            //            {'!', KEY_Exclaim},
            //            {'"', KEY_Quotedbl},
            //            {'#', KEY_Hash},
            //            {'$', KEY_Dollar},
            //            {'&', KEY_Ampersand},
            {VK_OEM_7, KEY_Quote},
            //            {'(', KEY_Leftparen},
            //            {')', KEY_Rightparen},
            //            {'*', KEY_Asterisk},
            //            {'+', KEY_Plus},
            {VK_OEM_COMMA, KEY_Comma},
            {VK_OEM_MINUS, KEY_Minus},
            {VK_OEM_PERIOD, KEY_Period},
            {VK_OEM_2, KEY_Slash},
            //            {':', KEY_Colon},
            {VK_OEM_1, KEY_Semicolon},
            //            {'<', KEY_Less},
            {VK_OEM_PLUS, KEY_Equals}, // + isn't an unmodded key, why does windows map is as a virtual??
                                       //            {'>', KEY_Greater},
                                       //            {'?', KEY_Question},
                                       //            {'@', KEY_At},
            {VK_OEM_4, KEY_Leftbracket},
            {VK_OEM_5, KEY_Backslash},
            {VK_OEM_6, KEY_Rightbracket},
            //            {'|', KEY_Caret},
            //            {'_', KEY_Underscore},
            //            {0xc0, KEY_Backquote},

            {VK_BACK, KEY_BackSpace}, /* back space, back char */
            {VK_TAB, KEY_Tab},
            //    KEY_Linefeed = 0xFF0A, /* Linefeed, LF */
            {VK_CLEAR, KEY_Clear},
            {VK_RETURN, KEY_Return}, /* Return, enter */
            {VK_PAUSE, KEY_Pause},   /* Pause, hold */
            {VK_SCROLL, KEY_Scroll_Lock},
            //    KEY_Sys_Req = 0xFF15,
            {VK_ESCAPE, KEY_Escape},
            {VK_DELETE, KEY_Delete}, /* Delete, rubout */

            /* Misc Functions */

            {VK_SELECT, KEY_Select}, /* Select, mark */
            {VK_PRINT, KEY_Print},
            {VK_EXECUTE, KEY_Execute}, /* Execute, run, do */
            {VK_INSERT, KEY_Insert},   /* Insert, insert here */
            //{ KEY_Undo = 0xFF65,    /* Undo, oops */
            //KEY_Redo = 0xFF66,    /* redo, again */
            {VK_APPS, KEY_Menu}, /* On Windows, this is VK_APPS, the context-menu key */
            // KEY_Find = 0xFF68,    /* Find, search */
            {VK_CANCEL, KEY_Cancel}, /* Cancel, stop, abort, exit */
            {VK_HELP, KEY_Help},     /* Help */
            //{ KEY_Break = 0xFF6B,
            //KEY_Mode_switch = 0xFF7E,   /* Character set switch */
            //KEY_Script_switch = 0xFF7E, /* Alias for mode_switch */
            {VK_NUMLOCK, KEY_Num_Lock},

            /* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

            //KEY_KP_Space = 0xFF80, /* space */
            //KEY_KP_Tab = 0xFF89,
            //KEY_KP_Enter = 0xFF8D, /* enter */
            //KEY_KP_F1 = 0xFF91,    /* PF1, KP_A, ... */
            //KEY_KP_F2 = 0xFF92,
            //KEY_KP_F3 = 0xFF93,
            //KEY_KP_F4 = 0xFF94,
            //KEY_KP_Home = 0xFF95,
            //KEY_KP_Left = 0xFF96,
            //KEY_KP_Up = 0xFF97,
            //KEY_KP_Right = 0xFF98,
            //KEY_KP_Down = 0xFF99,
            //KEY_KP_Prior = 0xFF9A,
            //KEY_KP_Page_Up = 0xFF9A,
            //KEY_KP_Next = 0xFF9B,
            //KEY_KP_Page_Down = 0xFF9B,
            //KEY_KP_End = 0xFF9C,
            //KEY_KP_Begin = 0xFF9D,
            //KEY_KP_Insert = 0xFF9E,
            //KEY_KP_Delete = 0xFF9F,
            //KEY_KP_Equal = 0xFFBD, /* equals */
            //KEY_KP_Multiply = 0xFFAA,
            //KEY_KP_Add = 0xFFAB,
            //KEY_KP_Separator = 0xFFAC, /* separator, often comma */
            //KEY_KP_Subtract = 0xFFAD,
            //KEY_KP_Decimal = 0xFFAE,
            //KEY_KP_Divide = 0xFFAF,

            {VK_NUMPAD0, KEY_KP_0},
            {VK_NUMPAD1, KEY_KP_1},
            {VK_NUMPAD2, KEY_KP_2},
            {VK_NUMPAD3, KEY_KP_3},
            {VK_NUMPAD4, KEY_KP_4},
            {VK_NUMPAD5, KEY_KP_5},
            {VK_NUMPAD6, KEY_KP_6},
            {VK_NUMPAD7, KEY_KP_7},
            {VK_NUMPAD8, KEY_KP_8},
            {VK_NUMPAD9, KEY_KP_9},

            /*
        * Auxiliary Functions; note the duplicate definitions for left and right
        * function keys;  Sun keyboards and a few other manufactures have such
        * function key groups on the left and/or right sides of the keyboard.
        * We've not found a keyboard with more than 35 function keys total.
        */

            {VK_F1, KEY_F1},
            {VK_F2, KEY_F2},
            {VK_F3, KEY_F3},
            {VK_F4, KEY_F4},
            {VK_F5, KEY_F5},
            {VK_F6, KEY_F6},
            {VK_F7, KEY_F7},
            {VK_F8, KEY_F8},
            {VK_F9, KEY_F9},
            {VK_F10, KEY_F10},
            {VK_F11, KEY_F11},
            {VK_F12, KEY_F12},
            {VK_F13, KEY_F13},
            {VK_F14, KEY_F14},
            {VK_F15, KEY_F15},
            {VK_F16, KEY_F16},
            {VK_F17, KEY_F17},
            {VK_F18, KEY_F18},
            {VK_F19, KEY_F19},
            {VK_F20, KEY_F20},
            {VK_F21, KEY_F21},
            {VK_F22, KEY_F22},
            {VK_F23, KEY_F23},
            {VK_F24, KEY_F24},

            //KEY_F25 = 0xFFD6,
            //KEY_F26 = 0xFFD7,
            //KEY_F27 = 0xFFD8,
            //KEY_F28 = 0xFFD9,
            //KEY_F29 = 0xFFDA,
            //KEY_F30 = 0xFFDB,
            //KEY_F31 = 0xFFDC,
            //KEY_F32 = 0xFFDD,
            //KEY_F33 = 0xFFDE,
            //KEY_F34 = 0xFFDF,
            //KEY_F35 = 0xFFE0,

            /* Modifiers */

            {VK_LSHIFT, KEY_Shift_L},     /* Left shift */
            {VK_RSHIFT, KEY_Shift_R},     /* Right shift */
            {VK_LCONTROL, KEY_Control_L}, /* Left control */
            {VK_RCONTROL, KEY_Control_R}, /* Right control */
            {VK_CAPITAL, KEY_Caps_Lock},  /* Caps lock */
            //KEY_Shift_Lock = 0xFFE6, /* Shift lock */

            //KEY_Meta_L = 0xFFE7,  /* Left meta */
            //KEY_Meta_R = 0xFFE8,  /* Right meta */
            {VK_LMENU, KEY_Alt_L},  /* Left alt */
            {VK_RMENU, KEY_Alt_R},  /* Right alt */
            {VK_LWIN, KEY_Super_L}, /* Left super */
            {VK_RWIN, KEY_Super_R}  /* Right super */
            //KEY_Hyper_L = 0xFFED, /* Left hyper */
            //KEY_Hyper_R = 0xFFEE  /* Right hyper */
        };
}

Win32_Window::Win32_Window(vsg::ref_ptr<WindowTraits> traits) :
    Inherit(traits),
    _window(nullptr)
{
    _keyboard = new KeyboardMap;

#ifdef UNICODE
    std::wstring windowClass;
    convert_utf(traits->windowClass, windowClass);
    std::wstring windowTitle;
    convert_utf(traits->windowTitle, windowTitle);
#else
    const auto& windowClass = traits->windowClass;
    const auto& windowTitle = traits->windowTitle;
#endif

    bool createWindow = true;

    if (traits->nativeWindow.has_value())
    {
        auto nativeWindow = std::any_cast<HWND>(traits->nativeWindow);
        if (nativeWindow)
        {
            _window = nativeWindow;
            createWindow = false;
        }
    }

    if (createWindow)
    {
        // register window class
        WNDCLASSEX wc;
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        wc.lpfnWndProc = Win32WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = ::GetModuleHandle(NULL);
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = 0;
        wc.lpszMenuName = 0;
        wc.lpszClassName = windowClass.c_str();
        wc.hIconSm = 0;

        if (::RegisterClassEx(&wc) == 0)
        {
            auto lastError = ::GetLastError();
            if (lastError != ERROR_CLASS_ALREADY_EXISTS) throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, could not register window class.", VK_ERROR_INITIALIZATION_FAILED};
        }

        // fetch screen display information

        std::vector<DISPLAY_DEVICE> displayDevices;
        DISPLAY_DEVICE displayDevice;
        displayDevice.cb = sizeof(displayDevice);

        for (uint32_t deviceNum = 0; EnumDisplayDevices(nullptr, deviceNum, &displayDevice, 0); ++deviceNum)
        {
            if (displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) continue;
            if (!(displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) continue;

            displayDevices.push_back(displayDevice);
        }

        // assume a traits->screenNum of < 0 will default to screen 0
        int32_t screenNum = traits->screenNum < 0 ? 0 : traits->screenNum;
        if (screenNum >= displayDevices.size()) throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, screenNum is out of range.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

        DEVMODE deviceMode;
        deviceMode.dmSize = sizeof(deviceMode);
        deviceMode.dmDriverExtra = 0;

        if (!::EnumDisplaySettings(displayDevices[screenNum].DeviceName, ENUM_CURRENT_SETTINGS, &deviceMode)) throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, EnumDisplaySettings failed to fetch display settings.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

        // setup window rect and style
        int32_t screenx = 0;
        int32_t screeny = 0;
        RECT windowRect;

        uint32_t windowStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        uint32_t extendedStyle = 0;

        if (!traits->fullscreen)
        {
            screenx = deviceMode.dmPosition.x + traits->x;
            screeny = deviceMode.dmPosition.y + traits->y;

            windowRect.left = screenx;
            windowRect.top = screeny;
            windowRect.right = windowRect.left + traits->width;
            windowRect.bottom = windowRect.top + traits->height;

            if (traits->decoration)
            {
                windowStyle |= WS_OVERLAPPEDWINDOW;

                extendedStyle |= WS_EX_WINDOWEDGE |
                                 WS_EX_APPWINDOW |
                                 WS_EX_OVERLAPPEDWINDOW |
                                 WS_EX_ACCEPTFILES |
                                 WS_EX_LTRREADING;

                // if decorated call adjust to account for borders etc
                if (!::AdjustWindowRectEx(&windowRect, windowStyle, FALSE, extendedStyle)) throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, AdjustWindowRectEx failed.", VK_ERROR_INVALID_EXTERNAL_HANDLE};
            }
        }
        else
        {
            screenx = deviceMode.dmPosition.x;
            screeny = deviceMode.dmPosition.y;

            windowRect.left = screenx;
            windowRect.top = screeny;
            windowRect.right = windowRect.left + deviceMode.dmPelsWidth;
            windowRect.bottom = windowRect.top + deviceMode.dmPelsHeight;
        }

        // create the window
        _window = ::CreateWindowEx(extendedStyle, windowClass.c_str(), windowTitle.c_str(), windowStyle,
                                   windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
                                   NULL, NULL, ::GetModuleHandle(NULL), NULL);

        if (_window == nullptr) throw Exception{"Error: vsg::Win32_Window::Win32_Window(...) failed to create Window, CreateWindowEx did not return a valid window handle.", VK_ERROR_INVALID_EXTERNAL_HANDLE};

        // set window handle user data pointer to hold ref to this so we can retrieve in WindowsProc
        SetWindowLongPtr(_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        // reposition once the window has been created to account for borders etc
        ::SetWindowPos(_window, nullptr, screenx, screeny, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 0);

        traits->x = windowRect.left;
        traits->y = windowRect.top;
        traits->systemConnection = wc.hInstance;

        ShowWindow(_window, SW_SHOW);
        SetForegroundWindow(_window);
        SetFocus(_window);
    }

    // get client rect to find final width height of the view
    RECT clientRect;
    ::GetClientRect(_window, &clientRect);

    uint32_t finalWidth = clientRect.right - clientRect.left;
    uint32_t finalHeight = clientRect.bottom - clientRect.top;

    if (traits->shareWindow)
    {
        // share the _instance, _physicalDevice and _device;
        share(*traits->shareWindow);
    }

    _extent2D.width = finalWidth;
    _extent2D.height = finalHeight;

    // assign dimensions
    traits->width = finalWidth;
    traits->height = finalHeight;

    traits->nativeWindow = _window;

    _windowMapped = true;
}

Win32_Window::~Win32_Window()
{
    clear();

    if (_window != nullptr)
    {
        vsg::debug("Calling DestroyWindow(_window);");

        TCHAR className[MAX_PATH];
        GetClassName(_window, className, MAX_PATH);

        ::DestroyWindow(_window);
        _window = nullptr;

        // when should we unregister??
        ::UnregisterClass(className, ::GetModuleHandle(NULL));
    }
}

void Win32_Window::_initSurface()
{
    if (!_instance) _initInstance();

    _surface = new vsgWin32::Win32Surface(_instance, _window);
}

bool Win32_Window::visible() const
{
    return _window != 0 && _windowMapped;
}

bool Win32_Window::pollEvents(vsg::UIEvents& events)
{
    vsg::clock::time_point event_time = vsg::clock::now();

    MSG msg;

    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            // somehow close all windows
            events.emplace_back(vsg::CloseWindowEvent::create(this, event_time));
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return Window::pollEvents(events);
}

void Win32_Window::resize()
{
    RECT windowRect;
    GetClientRect(_window, &windowRect);

    _extent2D.width = windowRect.right - windowRect.left;
    _extent2D.height = windowRect.bottom - windowRect.top;

    buildSwapchain();
}

LRESULT Win32_Window::handleWin32Messages(UINT msg, WPARAM wParam, LPARAM lParam)
{
    vsg::clock::time_point event_time = vsg::clock::now();

    // get the current window rect
    RECT windowRect;
    GetClientRect(_window, &windowRect);

    int32_t winx = windowRect.left;
    int32_t winy = windowRect.top;
    int32_t winw = windowRect.right - windowRect.left;
    int32_t winh = windowRect.bottom - windowRect.top;

    switch (msg)
    {
    case WM_CLOSE:
        vsg::debug("close window");
        bufferedEvents.emplace_back(vsg::CloseWindowEvent::create(this, event_time));
        break;
    case WM_SHOWWINDOW:
        bufferedEvents.emplace_back(vsg::ExposeWindowEvent::create(this, event_time, winx, winy, winw, winh));
        break;
    case WM_DESTROY:
        _windowMapped = false;
        break;
    case WM_PAINT:
        ValidateRect(_window, NULL);
        break;
    case WM_MOUSEMOVE: {
        int32_t mx = GET_X_LPARAM(lParam);
        int32_t my = GET_Y_LPARAM(lParam);

        bufferedEvents.emplace_back(vsg::MoveEvent::create(this, event_time, mx, my, getButtonMask(wParam)));
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN: {
        int32_t mx = GET_X_LPARAM(lParam);
        int32_t my = GET_Y_LPARAM(lParam);

        bufferedEvents.emplace_back(vsg::ButtonPressEvent::create(this, event_time, mx, my, getButtonMask(wParam), getButtonDownEventDetail(msg)));

        //::SetCapture(_window);
    }
    break;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP: {
        int32_t mx = GET_X_LPARAM(lParam);
        int32_t my = GET_Y_LPARAM(lParam);

        bufferedEvents.emplace_back(vsg::ButtonReleaseEvent::create(this, event_time, mx, my, getButtonMask(wParam), getButtonUpEventDetail(msg)));

        //::ReleaseCapture(); // should only release once all mouse buttons are released ??
        break;
    }
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK: {
        //::SetCapture(_window);
    }
    break;
    case WM_MOUSEWHEEL: {
        bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(this, event_time, GET_WHEEL_DELTA_WPARAM(wParam) < 0 ? vec3(0.0f, -1.0f, 0.0f) : vec3(0.0f, 1.0f, 0.0f)));
        break;
    }
    case WM_MOVE: {
        bufferedEvents.emplace_back(vsg::ConfigureWindowEvent::create(this, event_time, winx, winy, winw, winh));
        break;
    }
    case WM_SIZE: {
        if (wParam == SIZE_MINIMIZED || wParam == SIZE_MAXHIDE || winw == 0 || winh == 0)
        {
            _windowMapped = false;
        }
        else
        {
            _windowMapped = true;
            bufferedEvents.emplace_back(vsg::ConfigureWindowEvent::create(this, event_time, winx, winy, winw, winh));
        }
        break;
    }
    case WM_EXITSIZEMOVE:
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        vsg::KeySymbol keySymbol, modifiedKeySymbol;
        vsg::KeyModifier keyModifier;
        if (_keyboard->getKeySymbol(wParam, lParam, keySymbol, modifiedKeySymbol, keyModifier))
        {
            int32_t repeatCount = (lParam & 0xffff);
            bufferedEvents.emplace_back(vsg::KeyPressEvent::create(this, event_time, keySymbol, modifiedKeySymbol, keyModifier, repeatCount));
        }
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        vsg::KeySymbol keySymbol, modifiedKeySymbol;
        vsg::KeyModifier keyModifier;
        if (_keyboard->getKeySymbol(wParam, lParam, keySymbol, modifiedKeySymbol, keyModifier))
        {
            bufferedEvents.emplace_back(vsg::KeyReleaseEvent::create(this, event_time, keySymbol, modifiedKeySymbol, keyModifier, 0));
        }

        break;
    }
    default:
        break;
    }
    return ::DefWindowProc(_window, msg, wParam, lParam);
}
