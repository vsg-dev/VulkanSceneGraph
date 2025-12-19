/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/platform/android/Android_Window.h>

#include <vsg/core/Exception.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/io/Logger.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/TouchEvent.h>

#include <cassert>
#include <ctime>

using namespace vsg;
using namespace vsgAndroid;

namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to an Android_Window
    ref_ptr<Window> Window::create(vsg::ref_ptr<WindowTraits> traits)
    {
        return vsgAndroid::Android_Window::create(traits);
    }

} // namespace vsg

namespace vsgAndroid
{
    class AndroidSurface : public vsg::Surface
    {
    public:
        AndroidSurface(vsg::Instance* instance, ANativeWindow* window) :
            vsg::Surface(VK_NULL_HANDLE, instance)
        {
            VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo{};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.pNext = nullptr;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.window = window;

            auto result = vkCreateAndroidSurfaceKHR(*instance, &surfaceCreateInfo, _instance->getAllocationCallbacks(), &_surface);
            if (result != VK_SUCCESS)
            {
                throw Exception{"Failed to create AndroidSurface.", result};
            }
        }
    };

    static int64_t getUptimeMs()
    {
        struct timespec res;
        clock_gettime(CLOCK_MONOTONIC, &res);
        return 1000 * res.tv_sec + res.tv_nsec / 1000'000;
    }

} // namespace vsgAndroid

KeyboardMap::KeyboardMap()
{
    _keycodeMap =
        {
            {0x0, KEY_Undefined},

            {AKEYCODE_SPACE, KEY_Space},

            {AKEYCODE_0, KEY_0},
            {AKEYCODE_1, KEY_1},
            {AKEYCODE_2, KEY_2},
            {AKEYCODE_3, KEY_3},
            {AKEYCODE_4, KEY_4},
            {AKEYCODE_5, KEY_5},
            {AKEYCODE_6, KEY_6},
            {AKEYCODE_7, KEY_7},
            {AKEYCODE_8, KEY_8},
            {AKEYCODE_9, KEY_9},

            // Note that virtual key 'A' etc. correspond to the unmodified character 'a', hence the map below assigns capital letters to their corresponding lowercase ones.
            {AKEYCODE_A, KEY_a},
            {AKEYCODE_B, KEY_b},
            {AKEYCODE_C, KEY_c},
            {AKEYCODE_D, KEY_d},
            {AKEYCODE_E, KEY_e},
            {AKEYCODE_F, KEY_f},
            {AKEYCODE_G, KEY_g},
            {AKEYCODE_H, KEY_h},
            {AKEYCODE_I, KEY_i},
            {AKEYCODE_J, KEY_j},
            {AKEYCODE_K, KEY_k},
            {AKEYCODE_L, KEY_l},
            {AKEYCODE_M, KEY_m},
            {AKEYCODE_N, KEY_n},
            {AKEYCODE_O, KEY_o},
            {AKEYCODE_P, KEY_p},
            {AKEYCODE_Q, KEY_q},
            {AKEYCODE_R, KEY_r},
            {AKEYCODE_S, KEY_s},
            {AKEYCODE_T, KEY_t},
            {AKEYCODE_U, KEY_u},
            {AKEYCODE_Z, KEY_v},
            {AKEYCODE_W, KEY_w},
            {AKEYCODE_X, KEY_x},
            {AKEYCODE_Y, KEY_y},
            {AKEYCODE_Z, KEY_z},

            {'!', KEY_Exclaim},
            {'"', KEY_Quotedbl},
            {'#', KEY_Hash},
            {'$', KEY_Dollar},
            {'&', KEY_Ampersand},
            {AKEYCODE_APOSTROPHE, KEY_Quote},
            {'(', KEY_Leftparen},
            {')', KEY_Rightparen},
            {AKEYCODE_STAR, KEY_Asterisk},
            {'+', KEY_Plus},
            {AKEYCODE_COMMA, KEY_Comma},
            {AKEYCODE_MINUS, KEY_Minus},
            {AKEYCODE_PERIOD, KEY_Period},
            {AKEYCODE_SLASH, KEY_Slash},
            {':', KEY_Colon},
            {AKEYCODE_SEMICOLON, KEY_Semicolon},
            {'<', KEY_Less},
            {AKEYCODE_EQUALS, KEY_Equals}, // + isn't an unmodded key, why does windows map is as a virtual??
            {'>', KEY_Greater},
            {'?', KEY_Question},
            {AKEYCODE_AT, KEY_At},
            {AKEYCODE_LEFT_BRACKET, KEY_Leftbracket},
            {AKEYCODE_BACKSLASH, KEY_Backslash},
            {AKEYCODE_RIGHT_BRACKET, KEY_Rightbracket},
            {'|', KEY_Caret},
            {'_', KEY_Underscore},
            {'`', KEY_Backquote},

            {AKEYCODE_DEL, KEY_BackSpace}, /* back space, back char */
            {AKEYCODE_TAB, KEY_Tab},
            //    KEY_Linefeed = 0xFF0A, /* Linefeed, LF */
            {AKEYCODE_CLEAR, KEY_Clear},
            {AKEYCODE_ENTER, KEY_Return}, /* Return, enter */
            {AKEYCODE_BREAK, KEY_Pause},  /* Pause, hold */
            {AKEYCODE_SCROLL_LOCK, KEY_Scroll_Lock},
            //    KEY_Sys_Req = 0xFF15,
            {AKEYCODE_ESCAPE, KEY_Escape},
            {AKEYCODE_FORWARD_DEL, KEY_Delete}, /* Delete, rubout */

            /* Cursor control & motion */

            {AKEYCODE_MOVE_HOME, KEY_Home},
            {AKEYCODE_DPAD_LEFT, KEY_Left},          /* Move left, left arrow */
            {AKEYCODE_DPAD_UP, KEY_Up},              /* Move up, up arrow */
            {AKEYCODE_DPAD_RIGHT, KEY_Right},        /* Move right, right arrow */
            {AKEYCODE_DPAD_DOWN, KEY_Down},          /* Move down, down arrow */
            {AKEYCODE_NAVIGATE_PREVIOUS, KEY_Prior}, /* Prior, previous */
            {AKEYCODE_PAGE_UP, KEY_Page_Up},
            {AKEYCODE_NAVIGATE_NEXT, KEY_Next}, /* Next */
            {AKEYCODE_PAGE_DOWN, KEY_Page_Down},
            {AKEYCODE_MOVE_END, KEY_End}, /* EOL */
            //{ KEY_Begin = 0xFF58, /* BOL */

            /* Misc Functions */

            //{ VK_SELECT, KEY_Select }, /* Select, mark */
            //{ VK_PRINT, KEY_Print },
            //{ VK_EXECUTE, KEY_Execute }, /* Execute, run, do */
            {AKEYCODE_INSERT, KEY_Insert}, /* Insert, insert here */
            //{ KEY_Undo = 0xFF65,    /* Undo, oops */
            //KEY_Redo = 0xFF66,    /* redo, again */
            {AKEYCODE_MENU, KEY_Menu}, /* On Windows, this is VK_APPS, the context-menu key */
            // KEY_Find = 0xFF68,    /* Find, search */
            //{ VK_CANCEL, KEY_Cancel },  /* Cancel, stop, abort, exit */
            {AKEYCODE_HELP, KEY_Help}, /* Help */
            //{ KEY_Break = 0xFF6B,
            //KEY_Mode_switch = 0xFF7E,   /* Character set switch */
            //KEY_Script_switch = 0xFF7E, /* Alias for mode_switch */
            {AKEYCODE_NUM_LOCK, KEY_Num_Lock},

            /* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

            //KEY_KP_Space = 0xFF80, /* space */
            //KEY_KP_Tab = 0xFF89,
            {AKEYCODE_NUMPAD_ENTER, KEY_KP_Enter}, /* enter */
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
            {AKEYCODE_NUMPAD_EQUALS, KEY_KP_Equal}, /* equals */
            {AKEYCODE_NUMPAD_MULTIPLY, KEY_KP_Multiply},
            {AKEYCODE_NUMPAD_ADD, KEY_KP_Add},
            {AKEYCODE_NUMPAD_COMMA, KEY_KP_Separator}, /* separator, often comma */
            {AKEYCODE_NUMPAD_SUBTRACT, KEY_KP_Subtract},
            {AKEYCODE_NUMPAD_DOT, KEY_KP_Decimal},
            {AKEYCODE_NUMPAD_DIVIDE, KEY_KP_Divide},

            {AKEYCODE_NUMPAD_0, KEY_KP_0},
            {AKEYCODE_NUMPAD_1, KEY_KP_1},
            {AKEYCODE_NUMPAD_2, KEY_KP_2},
            {AKEYCODE_NUMPAD_3, KEY_KP_3},
            {AKEYCODE_NUMPAD_4, KEY_KP_4},
            {AKEYCODE_NUMPAD_5, KEY_KP_5},
            {AKEYCODE_NUMPAD_6, KEY_KP_6},
            {AKEYCODE_NUMPAD_7, KEY_KP_7},
            {AKEYCODE_NUMPAD_8, KEY_KP_8},
            {AKEYCODE_NUMPAD_9, KEY_KP_9},

            /*
        * Auxiliary Functions; note the duplicate definitions for left and right
        * function keys;  Sun keyboards and a few other manufacturers have such
        * function key groups on the left and/or right sides of the keyboard.
        * We've not found a keyboard with more than 35 function keys total.
        */

            {AKEYCODE_F1, KEY_F1},
            {AKEYCODE_F2, KEY_F2},
            {AKEYCODE_F3, KEY_F3},
            {AKEYCODE_F4, KEY_F4},
            {AKEYCODE_F5, KEY_F5},
            {AKEYCODE_F6, KEY_F6},
            {AKEYCODE_F7, KEY_F7},
            {AKEYCODE_F8, KEY_F8},
            {AKEYCODE_F9, KEY_F9},
            {AKEYCODE_F10, KEY_F10},
            {AKEYCODE_F11, KEY_F11},
            {AKEYCODE_F12, KEY_F12},
            //{ VK_F13, KEY_F13 },
            //{ VK_F14, KEY_F14 },
            //{ VK_F15, KEY_F15 },
            //{ VK_F16, KEY_F16 },
            //{ VK_F17, KEY_F17 },
            //{ VK_F18, KEY_F18 },
            //{ VK_F19, KEY_F19 },
            //{ VK_F20, KEY_F20 },
            //{ VK_F21, KEY_F21 },
            //{ VK_F22, KEY_F22 },
            //{ VK_F23, KEY_F23 },
            //{ VK_F24, KEY_F24 },

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

            {AKEYCODE_SHIFT_LEFT, KEY_Shift_L},   /* Left shift */
            {AKEYCODE_SHIFT_RIGHT, KEY_Shift_R},  /* Right shift */
            {AKEYCODE_CTRL_LEFT, KEY_Control_L},  /* Left control */
            {AKEYCODE_CTRL_RIGHT, KEY_Control_R}, /* Right control */
            {AKEYCODE_CAPS_LOCK, KEY_Caps_Lock},  /* Caps lock */
            //KEY_Shift_Lock = 0xFFE6, /* Shift lock */

            {AKEYCODE_META_LEFT, KEY_Meta_L},  /* Left meta */
            {AKEYCODE_META_RIGHT, KEY_Meta_R}, /* Right meta */
            {AKEYCODE_ALT_LEFT, KEY_Alt_L},    /* Left alt */
            {AKEYCODE_ALT_RIGHT, KEY_Alt_R},   /* Right alt */
            //{ VK_LWIN, KEY_Super_L }, /* Left super */
            //{ VK_RWIN, KEY_Super_R } /* Right super */
            //KEY_Hyper_L = 0xFFED, /* Left hyper */
            //KEY_Hyper_R = 0xFFEE  /* Right hyper */
        };
}

static JavaVM* javaVM = nullptr;
static jclass classKeyCharacterMap = nullptr;
static jmethodID methodLoad = nullptr;
static jmethodID methodGet = nullptr;

static JNIEnv* getJNIEnvFromJavaVM(JavaVM* vm)
{
    assert(vm);
    JNIEnv* env = nullptr;
    jint ret = vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if(ret == JNI_EDETACHED)
    {
        JavaVMAttachArgs args = {JNI_VERSION_1_6, "NativeActivityThread", nullptr};
        ret = vm->AttachCurrentThread(&env, &args);
        if (ret != JNI_OK)
        {
            vsg::error("Failed to attach thread：ret=", ret);
            return nullptr;
        }
    }
    else if (ret != JNI_OK)
    {
        vsg::error("Failed to get JNIEnv： ret=", ret);
        return nullptr;
    }
    return env;
}

void KeyboardMap::initializeKeyCharacterMap(void* vm)
{
    javaVM = static_cast<JavaVM*>(vm);
    JNIEnv* env = getJNIEnvFromJavaVM(javaVM);
    if (!env)
        return;

    jclass KeyCharacterMap = env->FindClass("android/view/KeyCharacterMap");  // Since API level 1
    assert(KeyCharacterMap);

    classKeyCharacterMap = (jclass)env->NewGlobalRef(KeyCharacterMap);
    env->DeleteLocalRef(KeyCharacterMap);

    methodLoad = env->GetStaticMethodID(classKeyCharacterMap, "load", "(I)Landroid/view/KeyCharacterMap;");
    methodGet = env->GetMethodID(classKeyCharacterMap, "get", "(II)I");
    assert(methodLoad && methodGet);
}

void KeyboardMap::releaseKeyCharacterMap()
{
    if (classKeyCharacterMap)
    {
        JNIEnv* env = getJNIEnvFromJavaVM(javaVM);
        if(env)
        {
            env->DeleteGlobalRef(classKeyCharacterMap);
            classKeyCharacterMap = nullptr;
        }

        javaVM->DetachCurrentThread();
    }
}

int32_t KeyboardMap::getUnicodeChar(int32_t keyCode, int32_t metaState)
{
    if(!javaVM || !classKeyCharacterMap || !methodLoad || !methodGet)
    {
        vsg::warn("JNI not initialized");
        return 0;
    }

    JNIEnv* env = getJNIEnvFromJavaVM(javaVM);
    if(!env)
        return 0;

    jobject kcmInstance = env->CallStaticObjectMethod(classKeyCharacterMap, methodLoad, 0);
    if (env->ExceptionCheck() || !kcmInstance)
    {
        env->ExceptionClear();
        return 0;
    }

    int32_t unicode = env->CallIntMethod(kcmInstance, methodGet, keyCode, metaState);
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
        unicode = 0;
    }

    env->DeleteLocalRef(kcmInstance);
    return unicode;
}

Android_Window::Android_Window(vsg::ref_ptr<WindowTraits> traits) :
    Inherit(traits)
{
    _keyboard = new KeyboardMap;

    if (!traits->getValue("nativeWindow", _window))
    {
        vsg::log(vsg::Logger::LOGGER_WARN, "Value 'nativeWindow' not provided on WindowTraits object. Falling back to std::any_cast<ANativeWindow*> for Android window setup.");
    }

    if (_window == nullptr)
    {
        ANativeWindow* nativeWindow = std::any_cast<ANativeWindow*>(traits->nativeWindow);

        if (nativeWindow == nullptr)
        {
            throw Exception{"Error: vsg::Android_Window::Android_Window(...) failed to create Window, traits->nativeHandle is not a valid ANativeWindow.", VK_ERROR_INVALID_EXTERNAL_HANDLE};
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

    _uptimeMs = getUptimeMs();
    _first_android_time_point = vsg::clock::now();
}

Android_Window::~Android_Window()
{
    clear();
}

void Android_Window::_initSurface()
{
    if (!_instance) _initInstance();

    _surface = new vsgAndroid::AndroidSurface(_instance, _window);
}

bool Android_Window::pollEvents(vsg::UIEvents& events)
{
    return Window::pollEvents(events);
}

void Android_Window::resize()
{
    _extent2D.width = ANativeWindow_getWidth(_window);
    _extent2D.height = ANativeWindow_getHeight(_window);

    vsg::debug("resize event = wh: ", _extent2D.width, ", ", _extent2D.height);

    buildSwapchain();
}

bool Android_Window::handleAndroidInputEvent(AInputEvent* anEvent)
{
    int32_t eventType = AInputEvent_getType(anEvent);
    if(eventType == AINPUT_EVENT_TYPE_MOTION)
    {
        int32_t action = AMotionEvent_getAction(anEvent);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        int32_t actionIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        size_t pointerCount = AMotionEvent_getPointerCount(anEvent);
        int64_t timeNs = AMotionEvent_getEventTime(anEvent);
        int64_t timeMs = timeNs / 1000'000;
        vsg::clock::time_point time = _first_android_time_point + std::chrono::milliseconds(timeMs - _uptimeMs);

        switch(actionMasked)
        {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
            {
                float x = AMotionEvent_getX(anEvent, actionIndex);
                float y = AMotionEvent_getY(anEvent, actionIndex);
                uint32_t id = AMotionEvent_getPointerId(anEvent, actionIndex);
                bufferedEvents.emplace_back(vsg::TouchDownEvent::create(this, time, x, y, id));
                break;
            }
            case AMOTION_EVENT_ACTION_MOVE:
                for(size_t p = 0; p < pointerCount; ++p)
                {
                    float x = AMotionEvent_getX(anEvent, p);
                    float y = AMotionEvent_getY(anEvent, p);
                    uint32_t id = AMotionEvent_getPointerId(anEvent, p);
                    bufferedEvents.emplace_back(vsg::TouchMoveEvent::create(this, time, x, y, id));
                }
                break;
            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
            {
                float x = AMotionEvent_getX(anEvent, actionIndex);
                float y = AMotionEvent_getY(anEvent, actionIndex);
                uint32_t id = AMotionEvent_getPointerId(anEvent, actionIndex);
                bufferedEvents.emplace_back(vsg::TouchUpEvent::create(this, time, x, y, id));
                break;
            }
            case AMOTION_EVENT_ACTION_CANCEL:
                for(size_t p = 0; p < pointerCount; ++p)
                {
                    float x = AMotionEvent_getX(anEvent, p);
                    float y = AMotionEvent_getY(anEvent, p);
                    uint32_t id = AMotionEvent_getPointerId(anEvent, p);
                    bufferedEvents.emplace_back(vsg::TouchUpEvent::create(this, time, x, y, id));
                }
                break;
            default:
                //vsg::info("unhandled touch action: %d", actionMasked);
                break;

        }
        return true;
    }
    else if (eventType == AINPUT_EVENT_TYPE_KEY)
    {
        int32_t action = AKeyEvent_getAction(anEvent);
        int32_t keyCode = AKeyEvent_getKeyCode(anEvent);
        int32_t metaState = AKeyEvent_getMetaState(anEvent);

        int64_t timeNs = AKeyEvent_getEventTime(anEvent);
        int64_t timeMs = timeNs / 1000'000;
        vsg::clock::time_point time = _first_android_time_point + std::chrono::milliseconds(timeMs - _uptimeMs);

        vsg::KeySymbol keySymbol, modifiedKeySymbol;
        vsg::KeyModifier keyModifier;
        if (!_keyboard->getKeySymbol(keyCode, metaState, keySymbol, modifiedKeySymbol, keyModifier))
            return false;

        switch (action)
        {
        case AKEY_EVENT_ACTION_DOWN:
            bufferedEvents.emplace_back(vsg::KeyPressEvent::create(this, time, keySymbol, modifiedKeySymbol, keyModifier));
            break;
        case AKEY_EVENT_ACTION_UP:
            bufferedEvents.emplace_back(vsg::KeyReleaseEvent::create(this, time, keySymbol, modifiedKeySymbol, keyModifier));
            break;
            //case AKEY_EVENT_ACTION_MULTIPLE:
            //   bufferedEvents.emplace_back(new vsg::KeyPressEvent(this, time, keySymbol, modifiedKeySymbol, keyModifier);
            //   break;
        default: break;
        }
        return true;
    }
    return false;
}
