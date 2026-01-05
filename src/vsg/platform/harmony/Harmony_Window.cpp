/* <editor-fold desc="MIT License">

Copyright(c) 2025 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/platform/ohos/Harmony_Window.h>

#include <vsg/core/Exception.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/io/Logger.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/TouchEvent.h>

#include <time.h>

#include <arkui/native_key_event.h>
#include <arkui/native_node.h>
#include <arkui/native_type.h>
#include <native_window/external_window.h>

using namespace vsg;
using namespace vsgHarmony;

namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to an Harmony_Window
    ref_ptr<Window> Window::create(vsg::ref_ptr<WindowTraits> traits)
    {
        return vsgHarmony::Harmony_Window::create(traits);
    }

} // namespace vsg

namespace vsgHarmony
{
    class OHOSsurface : public vsg::Surface
    {
    public:
        OHOSsurface(vsg::Instance* instance, OHNativeWindow* window) :
            vsg::Surface(VK_NULL_HANDLE, instance)
        {
            VkSurfaceCreateInfoOHOS surfaceCreateInfo{};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_SURFACE_CREATE_INFO_OHOS;
            surfaceCreateInfo.pNext = nullptr;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.window = window;

            auto result = vkCreateSurfaceOHOS(*instance, &surfaceCreateInfo, _instance->getAllocationCallbacks(), &_surface);
            if (result != VK_SUCCESS)
            {
                throw Exception{"Failed to create OHOSsurface.", result};
            }
        }
    };

    static int64_t now_ms(void)
    {
        struct timespec res;
        clock_gettime(CLOCK_MONOTONIC, &res);
        return 1000 * res.tv_sec + res.tv_nsec / 1e6;
    }

    // OHOS event handling will be implemented when NDK provides the APIs
    // Current placeholder implementation

} // namespace vsgHarmony

KeyboardMap::KeyboardMap()
{
    // OpenHarmony official keycode mappings using ARKUI_KEYCODE macros
    // Reference: https://developer.huawei.com/consumer/cn/doc/harmonyos-references/capi-native-key-event-h
    _keycodeMap =
        {
            {ARKUI_KEYCODE_UNKNOWN, KEY_Undefined},

            // Numbers (ARKUI_KEYCODE_0-9)
            {ARKUI_KEYCODE_0, KEY_0},
            {ARKUI_KEYCODE_1, KEY_1},
            {ARKUI_KEYCODE_2, KEY_2},
            {ARKUI_KEYCODE_3, KEY_3},
            {ARKUI_KEYCODE_4, KEY_4},
            {ARKUI_KEYCODE_5, KEY_5},
            {ARKUI_KEYCODE_6, KEY_6},
            {ARKUI_KEYCODE_7, KEY_7},
            {ARKUI_KEYCODE_8, KEY_8},
            {ARKUI_KEYCODE_9, KEY_9},

            // Letters (ARKUI_KEYCODE_A-Z)
            {ARKUI_KEYCODE_A, KEY_A},
            {ARKUI_KEYCODE_B, KEY_B},
            {ARKUI_KEYCODE_C, KEY_C},
            {ARKUI_KEYCODE_D, KEY_D},
            {ARKUI_KEYCODE_E, KEY_E},
            {ARKUI_KEYCODE_F, KEY_F},
            {ARKUI_KEYCODE_G, KEY_G},
            {ARKUI_KEYCODE_H, KEY_H},
            {ARKUI_KEYCODE_I, KEY_I},
            {ARKUI_KEYCODE_J, KEY_J},
            {ARKUI_KEYCODE_K, KEY_K},
            {ARKUI_KEYCODE_L, KEY_L},
            {ARKUI_KEYCODE_M, KEY_M},
            {ARKUI_KEYCODE_N, KEY_N},
            {ARKUI_KEYCODE_O, KEY_O},
            {ARKUI_KEYCODE_P, KEY_P},
            {ARKUI_KEYCODE_Q, KEY_Q},
            {ARKUI_KEYCODE_R, KEY_R},
            {ARKUI_KEYCODE_S, KEY_S},
            {ARKUI_KEYCODE_T, KEY_T},
            {ARKUI_KEYCODE_U, KEY_U},
            {ARKUI_KEYCODE_V, KEY_V},
            {ARKUI_KEYCODE_W, KEY_W},
            {ARKUI_KEYCODE_X, KEY_X},
            {ARKUI_KEYCODE_Y, KEY_Y},
            {ARKUI_KEYCODE_Z, KEY_Z},

            // Special characters
            {ARKUI_KEYCODE_COMMA, KEY_Comma},
            {ARKUI_KEYCODE_PERIOD, KEY_Period},
            {ARKUI_KEYCODE_GRAVE, KEY_Backquote},
            {ARKUI_KEYCODE_MINUS, KEY_Minus},
            {ARKUI_KEYCODE_EQUALS, KEY_Equals},
            {ARKUI_KEYCODE_LEFT_BRACKET, KEY_Leftbracket},
            {ARKUI_KEYCODE_RIGHT_BRACKET, KEY_Rightbracket},
            {ARKUI_KEYCODE_BACKSLASH, KEY_Backslash},
            {ARKUI_KEYCODE_SEMICOLON, KEY_Semicolon},
            {ARKUI_KEYCODE_APOSTROPHE, KEY_Quote},
            {ARKUI_KEYCODE_SLASH, KEY_Slash},
            {ARKUI_KEYCODE_AT, KEY_At},
            {ARKUI_KEYCODE_PLUS, KEY_Plus},
            {ARKUI_KEYCODE_STAR, KEY_Asterisk},
            {ARKUI_KEYCODE_POUND, KEY_Hash},

            // Control keys
            {ARKUI_KEYCODE_DEL, KEY_BackSpace},
            {ARKUI_KEYCODE_TAB, KEY_Tab},
            {ARKUI_KEYCODE_ENTER, KEY_Return},
            {ARKUI_KEYCODE_ESCAPE, KEY_Escape},
            {ARKUI_KEYCODE_FORWARD_DEL, KEY_Delete},
            {ARKUI_KEYCODE_SPACE, KEY_Space},

            // Cursor control & motion
            {ARKUI_KEYCODE_MOVE_HOME, KEY_Home},
            {ARKUI_KEYCODE_MOVE_END, KEY_End},
            {ARKUI_KEYCODE_DPAD_LEFT, KEY_Left},
            {ARKUI_KEYCODE_DPAD_UP, KEY_Up},
            {ARKUI_KEYCODE_DPAD_RIGHT, KEY_Right},
            {ARKUI_KEYCODE_DPAD_DOWN, KEY_Down},
            {ARKUI_KEYCODE_PAGE_UP, KEY_Page_Up},
            {ARKUI_KEYCODE_PAGE_DOWN, KEY_Page_Down},

            // Misc Functions
            {ARKUI_KEYCODE_INSERT, KEY_Insert},
            {ARKUI_KEYCODE_MENU, KEY_Menu},
            //{ARKUI_KEYCODE_HELP, KEY_Help},
            {ARKUI_KEYCODE_NUM_LOCK, KEY_Num_Lock},
            {ARKUI_KEYCODE_BREAK, KEY_Pause},
            {ARKUI_KEYCODE_SCROLL_LOCK, KEY_Scroll_Lock},

            // Keypad Functions
            {ARKUI_KEYCODE_NUMPAD_ENTER, KEY_KP_Enter},
            {ARKUI_KEYCODE_NUMPAD_EQUALS, KEY_KP_Equal},
            {ARKUI_KEYCODE_NUMPAD_MULTIPLY, KEY_KP_Multiply},
            {ARKUI_KEYCODE_NUMPAD_ADD, KEY_KP_Add},
            {ARKUI_KEYCODE_NUMPAD_COMMA, KEY_KP_Separator},
            {ARKUI_KEYCODE_NUMPAD_SUBTRACT, KEY_KP_Subtract},
            {ARKUI_KEYCODE_NUMPAD_DOT, KEY_KP_Decimal},
            {ARKUI_KEYCODE_NUMPAD_DIVIDE, KEY_KP_Divide},

            {ARKUI_KEYCODE_NUMPAD_0, KEY_KP_0},
            {ARKUI_KEYCODE_NUMPAD_1, KEY_KP_1},
            {ARKUI_KEYCODE_NUMPAD_2, KEY_KP_2},
            {ARKUI_KEYCODE_NUMPAD_3, KEY_KP_3},
            {ARKUI_KEYCODE_NUMPAD_4, KEY_KP_4},
            {ARKUI_KEYCODE_NUMPAD_5, KEY_KP_5},
            {ARKUI_KEYCODE_NUMPAD_6, KEY_KP_6},
            {ARKUI_KEYCODE_NUMPAD_7, KEY_KP_7},
            {ARKUI_KEYCODE_NUMPAD_8, KEY_KP_8},
            {ARKUI_KEYCODE_NUMPAD_9, KEY_KP_9},

            // Function keys
            {ARKUI_KEYCODE_F1, KEY_F1},
            {ARKUI_KEYCODE_F2, KEY_F2},
            {ARKUI_KEYCODE_F3, KEY_F3},
            {ARKUI_KEYCODE_F4, KEY_F4},
            {ARKUI_KEYCODE_F5, KEY_F5},
            {ARKUI_KEYCODE_F6, KEY_F6},
            {ARKUI_KEYCODE_F7, KEY_F7},
            {ARKUI_KEYCODE_F8, KEY_F8},
            {ARKUI_KEYCODE_F9, KEY_F9},
            {ARKUI_KEYCODE_F10, KEY_F10},
            {ARKUI_KEYCODE_F11, KEY_F11},
            {ARKUI_KEYCODE_F12, KEY_F12},

            // Modifier keys
            {ARKUI_KEYCODE_SHIFT_LEFT, KEY_Shift_L},
            {ARKUI_KEYCODE_SHIFT_RIGHT, KEY_Shift_R},
            {ARKUI_KEYCODE_CTRL_LEFT, KEY_Control_L},
            {ARKUI_KEYCODE_CTRL_RIGHT, KEY_Control_R},
            {ARKUI_KEYCODE_CAPS_LOCK, KEY_Caps_Lock},
            {ARKUI_KEYCODE_META_LEFT, KEY_Meta_L},
            {ARKUI_KEYCODE_META_RIGHT, KEY_Meta_R},
            {ARKUI_KEYCODE_ALT_LEFT, KEY_Alt_L},
            {ARKUI_KEYCODE_ALT_RIGHT, KEY_Alt_R}};
}
bool KeyboardMap::getKeySymbol(uint32_t keycode, KeyMetaState* KeyMetaState, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier)
{
    auto itr = _keycodeMap.find(keycode);
    if (itr == _keycodeMap.end()) return false;

    keySymbol = itr->second;
    modifiedKeySymbol = keySymbol;

    uint16_t modifierMask = 0;
    // Map OpenHarmony modifier flags to VSG modifiers
    // These values should match the OHOS NDK definitions
    if (KeyMetaState->altPressed) modifierMask |= vsg::KeyModifier::MODKEY_Alt;      // OH_META_ALT_ON
    if (KeyMetaState->ctrlPressed) modifierMask |= vsg::KeyModifier::MODKEY_Control; // OH_META_CTRL_ON
    if (KeyMetaState->shiftPressed) modifierMask |= vsg::KeyModifier::MODKEY_Shift;  // OH_META_SHIFT_ON
    if (KeyMetaState->capsLock) modifierMask |= vsg::KeyModifier::MODKEY_CapsLock;   // OH_META_CAPS_LOCK_ON
    if (KeyMetaState->numLock) modifierMask |= vsg::KeyModifier::MODKEY_NumLock;     // OH_META_NUM_LOCK_ON

    keyModifier = (vsg::KeyModifier)modifierMask;

    return true;
}

Harmony_Window::Harmony_Window(vsg::ref_ptr<WindowTraits> traits) :
    Inherit(traits)
{
    _keyboard = new KeyboardMap;

    if (!traits->getValue("nativeWindow", _window))
    {
        vsg::log(vsg::Logger::LOGGER_WARN, "Value 'nativeWindow' not provided on WindowTraits object. Falling back to std::any_cast<OHNativeWindow*> for OpenHarmony window setup.");
    }

    if (_window == nullptr)
    {
        OHNativeWindow* nativeWindow = std::any_cast<OHNativeWindow*>(traits->nativeWindow);

        if (nativeWindow == nullptr)
        {
            throw Exception{"Error: vsg::Harmony_Window::Harmony_Window(...) failed to create Window, traits->nativeHandle is not a valid OHNativeWindow.", VK_ERROR_INVALID_EXTERNAL_HANDLE};
        }

        _window = nativeWindow;
    }

    // we could get the width and height from the window?
    uint32_t finalWidth = traits->width;
    uint32_t finalHeight = traits->height;

    if (traits->device)
    {
        share(traits->device);
    }

    _extent2D.width = finalWidth;
    _extent2D.height = finalHeight;

    _first_ohos_timestamp = now_ms();
    _first_ohos_time_point = vsg::clock::now();
}

Harmony_Window::~Harmony_Window()
{
    clear();
}

void Harmony_Window::_initSurface()
{
    if (!_instance) _initInstance();

    _surface = new vsgHarmony::OHOSsurface(_instance, _window);
}

bool Harmony_Window::pollEvents(vsg::UIEvents& events)
{
    return Window::pollEvents(events);
}

void Harmony_Window::resize()
{
    // Get window dimensions using NativeWindowHandleOpt
    int32_t code = 0;
    int32_t width = 0;
    int32_t height = 0;

    code = OH_NativeWindow_NativeWindowHandleOpt(_window, GET_BUFFER_GEOMETRY, &height, &width);
    if (code == 0)
    {
        _extent2D.width = static_cast<uint32_t>(width);
        _extent2D.height = static_cast<uint32_t>(height);
        vsg::debug("resize event - width: ", _extent2D.width, ", height: ", _extent2D.height);
    }
    else
    {
        vsg::warn("Failed to get window dimensions, code: ", code);
    }

    buildSwapchain();
}
void updateMetaState(KeyMetaState* state, const ArkUI_UIInputEvent* event, int32_t action, int32_t keyCode)
{
    OH_ArkUI_KeyEvent_IsCapsLockOn(event, &state->capsLock);
    OH_ArkUI_KeyEvent_IsNumLockOn(event, &state->numLock);

    if (action == ARKUI_KEY_EVENT_DOWN)
    {
        if (keyCode == ARKUI_KEYCODE_SHIFT_LEFT || keyCode == ARKUI_KEYCODE_SHIFT_RIGHT)
            state->shiftPressed = true;
        if (keyCode == ARKUI_KEYCODE_CTRL_LEFT || keyCode == ARKUI_KEYCODE_CTRL_RIGHT)
            state->ctrlPressed = true;
        if (keyCode == ARKUI_KEYCODE_ALT_LEFT || keyCode == ARKUI_KEYCODE_ALT_RIGHT)
            state->altPressed = true;
    }
    else if (action == ARKUI_KEY_EVENT_UP)
    {
        if (keyCode == ARKUI_KEYCODE_SHIFT_LEFT || keyCode == ARKUI_KEYCODE_SHIFT_RIGHT)
            state->shiftPressed = false;
        if (keyCode == ARKUI_KEYCODE_CTRL_LEFT || keyCode == ARKUI_KEYCODE_CTRL_RIGHT)
            state->ctrlPressed = false;
        if (keyCode == ARKUI_KEYCODE_ALT_LEFT || keyCode == ARKUI_KEYCODE_ALT_RIGHT)
            state->altPressed = false;
    }
}
bool Harmony_Window::handleOHOSInputEvent(ArkUI_NodeEvent* event)
{
    if (!event) return false;

    // Get input event from ArkUI node event
    ArkUI_UIInputEvent* inputEvent = OH_ArkUI_NodeEvent_GetInputEvent(event);
    if (!inputEvent) return false;

    // Get event type
    int32_t eventType = OH_ArkUI_UIInputEvent_GetType(inputEvent);

    // Handle touch/pointer events
    if (eventType == ArkUI_UIInputEvent_Type::ARKUI_UIINPUTEVENT_TYPE_TOUCH)
    {
        // Get action type
        int32_t action = OH_ArkUI_UIInputEvent_GetAction(inputEvent);

        // Get event time (in nanoseconds)
        int64_t eventTimeNs = OH_ArkUI_UIInputEvent_GetEventTime(inputEvent);
        vsg::clock::time_point event_time = _first_ohos_time_point +
                                            std::chrono::milliseconds((eventTimeNs / 1000000) - _first_ohos_timestamp);

        // Process current event

        // Map OHOS action to VSG events
        switch (action)
        {
        case UI_TOUCH_EVENT_ACTION_DOWN: { // ACTION_DOWN
            uint32_t pointerIndex = 0;
            int32_t ret = OH_ArkUI_PointerEvent_GetChangedPointerId(inputEvent, &pointerIndex);
            if (ARKUI_ERROR_CODE_NO_ERROR != ret)
                return false;
            int32_t pointerId = OH_ArkUI_PointerEvent_GetPointerId(inputEvent, pointerIndex);
            // Get current coordinates
            float x = OH_ArkUI_PointerEvent_GetXByIndex(inputEvent, pointerIndex);
            float y = OH_ArkUI_PointerEvent_GetYByIndex(inputEvent, pointerIndex);
            vsg::debug("touch down event - id: ", pointerId, " xy: ", x, ", ", y);
            bufferedEvents.emplace_back(vsg::TouchDownEvent::create(this, event_time, x, y, pointerId));
        }
        break;

        case UI_TOUCH_EVENT_ACTION_MOVE: { // ACTION_MOVE
            // Get pointer count
            uint32_t pointerCount = OH_ArkUI_PointerEvent_GetPointerCount(inputEvent);
            for (uint32_t p = 0; p < pointerCount; p++)
            {
                // Get pointer ID
                int32_t pointerId = OH_ArkUI_PointerEvent_GetPointerId(inputEvent, p);

                // Get current coordinates
                float x = OH_ArkUI_PointerEvent_GetXByIndex(inputEvent, p);
                float y = OH_ArkUI_PointerEvent_GetYByIndex(inputEvent, p);

                vsg::debug("touch move event - id: ", pointerId, " xy: ", x, ", ", y);
                bufferedEvents.emplace_back(vsg::TouchMoveEvent::create(this, event_time, x, y, pointerId));
            }
        }
        break;
        case UI_TOUCH_EVENT_ACTION_UP: { // ACTION_UP
            uint32_t pointerIndex = 0;
            int32_t ret = OH_ArkUI_PointerEvent_GetChangedPointerId(inputEvent, &pointerIndex);
            if (ARKUI_ERROR_CODE_NO_ERROR != ret)
                return false;
            int32_t pointerId = OH_ArkUI_PointerEvent_GetPointerId(inputEvent, pointerIndex);
            // Get current coordinates
            float x = OH_ArkUI_PointerEvent_GetXByIndex(inputEvent, pointerIndex);
            float y = OH_ArkUI_PointerEvent_GetYByIndex(inputEvent, pointerIndex);
            vsg::debug("touch up event - id: ", pointerId, " xy: ", x, ", ", y);
            bufferedEvents.emplace_back(vsg::TouchUpEvent::create(this, event_time, x, y, pointerId));
        }
        break;
        case UI_TOUCH_EVENT_ACTION_CANCEL: // ACTION_CANCEL
            vsg::debug("touch cancel");
            bufferedEvents.emplace_back(vsg::TouchUpEvent::create(this, event_time, 0, 0, -1));
            break;
        default:
            break;
        }
        //}
        return true;
    }
    // Handle keyboard events
    else if (eventType == ArkUI_UIInputEvent_Type::ARKUI_UIINPUTEVENT_TYPE_KEY)
    {
        // Get action type
        int32_t action = OH_ArkUI_KeyEvent_GetType(inputEvent);

        // Get event time
        int64_t eventTimeNs = OH_ArkUI_UIInputEvent_GetEventTime(inputEvent);
        vsg::clock::time_point event_time = _first_ohos_time_point +
                                            std::chrono::milliseconds((eventTimeNs / 1000000) - _first_ohos_timestamp);

        // Get keycode from UIInputEvent
        int32_t keyCode = OH_ArkUI_KeyEvent_GetKeyCode(inputEvent);

        updateMetaState(&_keyMetaState, inputEvent, action, keyCode);

        vsg::KeySymbol keySymbol, modifiedKeySymbol;
        vsg::KeyModifier keyModifier;
        if (!_keyboard->getKeySymbol(static_cast<uint32_t>(keyCode), &_keyMetaState, keySymbol, modifiedKeySymbol, keyModifier))
            return false;

        // OHOS key actions: ACTION_DOWN = 0, ACTION_UP = 1
        switch (action)
        {
        case ARKUI_KEY_EVENT_DOWN: // ACTION_DOWN
            vsg::debug("key down event - keycode: ", keyCode);
            bufferedEvents.emplace_back(vsg::KeyPressEvent::create(this, event_time, keySymbol, modifiedKeySymbol, keyModifier));
            break;
        case ARKUI_KEY_EVENT_UP: // ACTION_UP
            vsg::debug("key up event - keycode: ", keyCode);
            bufferedEvents.emplace_back(vsg::KeyReleaseEvent::create(this, event_time, keySymbol, modifiedKeySymbol, keyModifier));
            break;
        default:
            break;
        }
        return true;
    }

    return false;
}
