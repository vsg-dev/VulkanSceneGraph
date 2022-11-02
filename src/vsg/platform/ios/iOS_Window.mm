#include <vsg/platform/ios/iOS_Window.h>

/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */



#include <vsg/core/Exception.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/io/Logger.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/TouchEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/vk/Extensions.h>

#include <time.h>

#include <UIKit/UIKit.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_metal.h>

#include <vsg/app/Viewer.h>
#include <vsg/platform/ios/iOS_ViewController.h>

namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to a iOS_Window
    vsg::ref_ptr<Window> Window::create(vsg::ref_ptr<vsg::WindowTraits> traits)
    {
        return vsgiOS::iOS_Window::create(traits);
    }

} // namespace vsg


#pragma mark -
#pragma mark vsg_iOS_Window


@implementation vsg_iOS_Window
{
    vsg::ref_ptr<vsg::WindowTraits> _traits;
    vsg::ref_ptr<vsg::Viewer>       _vsgViewer;

}

- (instancetype)initWithTraits:(vsg::ref_ptr<vsg::WindowTraits>)traits andVsgViewer:(vsg::ref_ptr<vsg::Viewer>) vsgViewer
{
    _traits = traits;
    _vsgViewer = vsgViewer;
    CGRect frame;
    frame.origin.x = _traits->x;
    frame.origin.y = _traits->y;
    frame.size.width  = _traits->width <= 0 ? 1 : _traits->width;
    frame.size.height = _traits->height <= 0 ? 1 : _traits->height;
    self = [super initWithFrame:frame];
    if (self != nil)
    {
        vsg_iOS_ViewController* vc = [[vsg_iOS_ViewController alloc] initWithTraits: traits andVsgViewer: vsgViewer];
        self.rootViewController = vc;
     }

    return self;
}

- (void)makeKeyAndVisible
{
    [super makeKeyAndVisible];
    
    vsg_iOS_Window* nWindow = self;
    _traits->nativeWindow = nWindow;
    
    vsg::ref_ptr<vsg::Window> w = vsg::Window::create(_traits);
    ((vsg_iOS_ViewController*)self.rootViewController).vsgWindow = w;
    _vsgViewer->addWindow(w);
}

- (vsg::ref_ptr<vsg::Window>)vsgWindow
{
    return ((vsg_iOS_ViewController*)self.rootViewController).vsgWindow;
}

- (BOOL)windowShouldClose:(id)sender
{
    vsgiOS::iOS_Window* w = static_cast<vsgiOS::iOS_Window*>(self.vsgWindow.get());
    vsg::clock::time_point event_time = vsg::clock::now(); // TODO call window getEventTime()??
    w->queueEvent(new vsg::CloseWindowEvent(w, event_time));
    return NO;
}

@end

#pragma mark -
#pragma mark vsg_iOS_View

@implementation vsg_iOS_View
{
    @public vsgiOS::iOS_Window* vsgWindow;
    vsg::ref_ptr<vsg::WindowTraits> _traits;
}

/** Returns a Metal-compatible layer. */
+(Class) layerClass { return [CAMetalLayer class]; }

- (BOOL)isOpaque
{
    return true;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)becomeFirstResponder
{
    return YES;
}

- (void)updateLayer
{

}

- (BOOL)isMultipleTouchEnabled
{
    return YES;
}

template<class T>
vsg::ref_ptr<T> createTouchEvt(vsgiOS::iOS_Window* window, vsg::clock::time_point event_time, UITouch* touch, uint32_t in_id, float devicePixelScale)
{
    auto pos = [touch locationInView:nil];
    uint32_t x = pos.x;
    uint32_t y = pos.y;
    vsg::debug("Touch Id=", in_id, "@[", x, ", ", y, "]");
    return vsg::ref_ptr<T>(new T(window, event_time, x, y, in_id));
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    auto devicePixelScale = _traits->hdpi ? UIScreen.mainScreen.nativeScale : 1.0f;
    vsg::clock::time_point event_time = vsgWindow->getEventTime([event timestamp]);
    uint32_t in_id = static_cast<uint32_t>([event allTouches].count) - 1;
    for (auto touch in [event allTouches])
        vsgWindow->queueEvent(createTouchEvt<vsg::TouchDownEvent>(vsgWindow, event_time, touch, in_id--, devicePixelScale));
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    auto devicePixelScale = _traits->hdpi ? UIScreen.mainScreen.nativeScale : 1.0f;
    vsg::clock::time_point event_time = vsgWindow->getEventTime([event timestamp]);
    uint32_t in_id = static_cast<uint32_t>([event allTouches].count) - 1;
    for (auto touch in [event allTouches])
        vsgWindow->queueEvent(createTouchEvt<vsg::TouchUpEvent>(vsgWindow, event_time, touch, in_id--, devicePixelScale));
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    auto devicePixelScale = _traits->hdpi ? UIScreen.mainScreen.nativeScale : 1.0f;
    vsg::clock::time_point event_time = vsgWindow->getEventTime([event timestamp]);
    uint32_t in_id = static_cast<uint32_t>([event allTouches].count) - 1;
    for (auto touch in [event allTouches])
        vsgWindow->queueEvent(createTouchEvt<vsg::TouchMoveEvent>(vsgWindow, event_time, touch, in_id--, devicePixelScale));
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)pressesChanged:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(UIPressesEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)cursorUpdate:(UIEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)mouseDown:(UIEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)mouseDragged:(UIEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)mouseUp:(UIEvent *)event
{
    vsgWindow->handleUIEvent(event);
}

- (void)mouseMoved:(UIEvent *)event
{
    vsgWindow->handleUIEvent(event);
}
@end


namespace vsgiOS
{
    class iOSSurface : public vsg::Surface
    {
    public:
        iOSSurface(vsg::Instance* instance, CAMetalLayer* caMetalLayer)
            : vsg::Surface(VK_NULL_HANDLE, instance)
        {
            VkMetalSurfaceCreateInfoEXT surfaceCreateInfo{};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
            surfaceCreateInfo.pNext = nullptr;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.pLayer = caMetalLayer;
            
            auto res = vkCreateMetalSurfaceEXT(*instance, &surfaceCreateInfo, _instance->getAllocationCallbacks(), &_surface);
            if (res != VK_SUCCESS || _surface == VK_NULL_HANDLE)
                vsg::error("Failed creating VkSurface");
        }
    };
}

vsgiOS::iOS_Window::iOS_Window(vsg::ref_ptr<vsg::WindowTraits> traits)
    : Inherit(traits)
{
    auto devicePixelScale = _traits->hdpi ?  UIScreen.mainScreen.nativeScale : 1.0f;
    _window = std::any_cast<vsg_iOS_Window*>(traits->nativeWindow);
    _view = (vsg_iOS_View*)( _window.rootViewController.view );
    _view->vsgWindow = this;
    _view->_traits = _traits;
    _keyboard = new KeyboardMap;
    _metalLayer = (CAMetalLayer*) _view.layer;
    
    uint32_t finalwidth = traits->width * devicePixelScale;
    uint32_t finalheight = traits->height * devicePixelScale;
    
    _extent2D.width = finalwidth;
    _extent2D.height = finalheight;
    
    // manually trigger configure here??
    vsg::clock::time_point event_time = vsg::clock::now();
    _bufferedEvents.emplace_back(new vsg::ConfigureWindowEvent(this, event_time, _traits->x, _traits->y, finalwidth, finalheight));
}

vsgiOS::iOS_Window::~iOS_Window()
{
    clear();
}

void vsgiOS::iOS_Window::_initSurface()
{
    if (!_instance) _initInstance();

    _surface = new vsgiOS::iOSSurface(_instance, _metalLayer);
}

bool vsgiOS::iOS_Window::pollEvents(vsg::UIEvents& events)
{
//    for (;;)
//    {
//        NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
//                                            untilDate:[NSDate distantPast]
//                                               inMode:NSDefaultRunLoopMode
//                                              dequeue:YES];
//        if (event == nil)
//            break;
//
//        [NSApp sendEvent:event];
//    }
//
    if (_bufferedEvents.size() > 0)
    {
        events.splice(events.end(), _bufferedEvents);
        _bufferedEvents.clear();
        return true;
    }

    return false;
}

//bool vsgiOS::iOS_Window::resized() const
//{
//    const CGRect contentRect = [_view frame];
//
//    auto devicePixelScale = _traits->hdpi ? UIScreen.mainScreen.nativeScale : 1.0f;
//
//    uint32_t width = contentRect.size.width * devicePixelScale;
//    uint32_t height = contentRect.size.height * devicePixelScale;
//
//    return (width != _extent2D.width || height != _extent2D.height);
//}

void vsgiOS::iOS_Window::resize()
{
    const CGRect contentRect = [_view frame];

    auto devicePixelScale = _traits->hdpi ? UIScreen.mainScreen.nativeScale  : 1.0f;
    [_metalLayer setContentsScale:devicePixelScale];

    _extent2D.width = contentRect.size.width * devicePixelScale;
    _extent2D.height = contentRect.size.height * devicePixelScale;

    buildSwapchain();
}


vsgiOS::KeyboardMap::KeyboardMap()
{
    _keycodeMap =
    {
        { 0xFF, vsg::KEY_Undefined },

//        { kVK_Space, KEY_Space },

//        { kVK_ANSI_0, KEY_0 },
//        { kVK_ANSI_1, KEY_1 },
//        { kVK_ANSI_2, KEY_2 },
//        { kVK_ANSI_3, KEY_3 },
//        { kVK_ANSI_4, KEY_4 },
//        { kVK_ANSI_5, KEY_5 },
//        { kVK_ANSI_6, KEY_6 },
//        { kVK_ANSI_7, KEY_7 },
//        { kVK_ANSI_8, KEY_8 },
//        { kVK_ANSI_9, KEY_9 },

//        { kVK_ANSI_A, KEY_a },
//        { kVK_ANSI_B, KEY_b },
//        { kVK_ANSI_C, KEY_c },
//        { kVK_ANSI_D, KEY_d },
//        { kVK_ANSI_E, KEY_e },
//        { kVK_ANSI_F, KEY_f },
//        { kVK_ANSI_G, KEY_g },
//        { kVK_ANSI_H, KEY_h },
//        { kVK_ANSI_I, KEY_i },
//        { kVK_ANSI_J, KEY_j },
//        { kVK_ANSI_K, KEY_k },
//        { kVK_ANSI_L, KEY_l },
//        { kVK_ANSI_M, KEY_m },
//        { kVK_ANSI_N, KEY_n },
//        { kVK_ANSI_O, KEY_o },
//        { kVK_ANSI_P, KEY_p },
//        { kVK_ANSI_Q, KEY_q },
//        { kVK_ANSI_R, KEY_r },
//        { kVK_ANSI_S, KEY_s },
//        { kVK_ANSI_T, KEY_t },
//        { kVK_ANSI_U, KEY_u },
//        { kVK_ANSI_Z, KEY_v },
//        { kVK_ANSI_W, KEY_w },
//        { kVK_ANSI_X, KEY_x },
//        { kVK_ANSI_Y, KEY_y },
//        { kVK_ANSI_Z, KEY_z },

        { 'A', vsg::KEY_A },
        { 'B', vsg::KEY_B },
        { 'C', vsg::KEY_C },
        { 'D', vsg::KEY_D },
        { 'E', vsg::KEY_E },
        { 'F', vsg::KEY_F },
        { 'G', vsg::KEY_G },
        { 'H', vsg::KEY_H },
        { 'I', vsg::KEY_I },
        { 'J', vsg::KEY_J },
        { 'K', vsg::KEY_K },
        { 'L', vsg::KEY_L },
        { 'M', vsg::KEY_M },
        { 'N', vsg::KEY_N },
        { 'O', vsg::KEY_O },
        { 'P', vsg::KEY_P },
        { 'Q', vsg::KEY_Q },
        { 'R', vsg::KEY_R },
        { 'S', vsg::KEY_S },
        { 'T', vsg::KEY_T },
        { 'U', vsg::KEY_U },
        { 'V', vsg::KEY_V },
        { 'W', vsg::KEY_W },
        { 'X', vsg::KEY_X },
        { 'Y', vsg::KEY_Y },
        { 'Z', vsg::KEY_Z },

        { '!', vsg::KEY_Exclaim },
        { '"', vsg::KEY_Quotedbl },
        { '#', vsg::KEY_Hash },
        { '$', vsg::KEY_Dollar },
        { '&', vsg::KEY_Ampersand },
//        { kVK_ANSI_Quote, KEY_Quote },
        { '(', vsg::KEY_Leftparen },
        { ')', vsg::KEY_Rightparen },
        { '*', vsg::KEY_Asterisk },
        { '+', vsg::KEY_Plus },
//        { kVK_ANSI_Comma, KEY_Comma },
//        { kVK_ANSI_Minus, KEY_Minus },
//        { kVK_ANSI_Period, KEY_Period },
//        { kVK_ANSI_Slash, KEY_Slash },
        { ':', vsg::KEY_Colon },
//        { kVK_ANSI_Semicolon, KEY_Semicolon },
        { '<', vsg::KEY_Less },
//        { kVK_ANSI_Equal, KEY_Equals }, // + isn't an unmodded key, why does windows map is as a virtual??
        { '>', vsg::KEY_Greater },
        { '?', vsg::KEY_Question },
        { '@', vsg::KEY_At},
//        { kVK_ANSI_LeftBracket, KEY_Leftbracket },
//        { kVK_ANSI_Backslash, KEY_Backslash },
//        { kVK_ANSI_RightBracket, KEY_Rightbracket },
        {'|', vsg::KEY_Caret },
        {'_', vsg::KEY_Underscore },
        {'`', vsg::KEY_Backquote },

//        { kVK_Delete, KEY_BackSpace }, /* back space, back char */
//        { kVK_Tab, KEY_Tab },
        //    KEY_Linefeed = 0xFF0A, /* Linefeed, LF */
        //{ AKEYCODE_CLEAR, KEY_Clear },
//        { kVK_Return, KEY_Return }, /* Return, enter */
        //{ AKEYCODE_BREAK, KEY_Pause },  /* Pause, hold */
        //{ AKEYCODE_SCROLL_LOCK, KEY_Scroll_Lock },
        //    KEY_Sys_Req = 0xFF15,
//        { kVK_Escape, KEY_Escape },
//        { kVK_ForwardDelete, KEY_Delete }, /* Delete, rubout */

        /* Cursor control & motion */

//        { kVK_Home, KEY_Home },
//        { kVK_LeftArrow, KEY_Left },          /* Move left, left arrow */
//        { kVK_UpArrow, KEY_Up },              /* Move up, up arrow */
//        { kVK_RightArrow, KEY_Right },        /* Move right, right arrow */
//        { kVK_DownArrow, KEY_Down },          /* Move down, down arrow */
        //{ AKEYCODE_NAVIGATE_PREVIOUS, KEY_Prior }, /* Prior, previous */
//        { kVK_PageUp, KEY_Page_Up },
        //{ AKEYCODE_NAVIGATE_NEXT, KEY_Next }, /* Next */
//        { kVK_PageDown, KEY_Page_Down },
//        { kVK_End, KEY_End }, /* EOL */
        //{ KEY_Begin = 0xFF58, /* BOL */

        /* Misc Functions */

        //{ VK_SELECT, KEY_Select }, /* Select, mark */
        //{ VK_PRINT, KEY_Print },
        //{ VK_EXECUTE, KEY_Execute }, /* Execute, run, do */
        //{ AKEYCODE_INSERT, KEY_Insert }, /* Insert, insert here */
        //{ KEY_Undo = 0xFF65,    /* Undo, oops */
        //KEY_Redo = 0xFF66,    /* redo, again */
        //{ AKEYCODE_MENU, KEY_Menu }, /* On Windows, this is VK_APPS, the context-menu key */
        // KEY_Find = 0xFF68,    /* Find, search */
        //{ VK_CANCEL, KEY_Cancel },  /* Cancel, stop, abort, exit */
//        { kVK_Help, KEY_Help }, /* Help */
        //{ KEY_Break = 0xFF6B,
        //KEY_Mode_switch = 0xFF7E,   /* Character set switch */
        //KEY_Script_switch = 0xFF7E, /* Alias for mode_switch */
        //{ AKEYCODE_NUM_LOCK, KEY_Num_Lock },

        /* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

        //KEY_KP_Space = 0xFF80, /* space */
        //KEY_KP_Tab = 0xFF89,
//        { kVK_ANSI_KeypadEnter, KEY_KP_Enter }, /* enter */
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
//        { kVK_ANSI_KeypadEquals, KEY_KP_Equal }, /* equals */
//        { kVK_ANSI_KeypadMultiply, KEY_KP_Multiply },
//        { kVK_ANSI_KeypadPlus, KEY_KP_Add },
        //{ AKEYCODE_NUMPAD_COMMA, KEY_KP_Separator }, /* separator, often comma */
//        { kVK_ANSI_KeypadMinus, KEY_KP_Subtract },
//        { kVK_ANSI_KeypadDecimal, KEY_KP_Decimal },
//        { kVK_ANSI_KeypadDivide, KEY_KP_Divide },

//        { kVK_ANSI_Keypad0, KEY_KP_0 },
//        { kVK_ANSI_Keypad1, KEY_KP_1 },
//        { kVK_ANSI_Keypad2, KEY_KP_2 },
//        { kVK_ANSI_Keypad3, KEY_KP_3 },
//        { kVK_ANSI_Keypad4, KEY_KP_4 },
//        { kVK_ANSI_Keypad5, KEY_KP_5 },
//        { kVK_ANSI_Keypad6, KEY_KP_6 },
//        { kVK_ANSI_Keypad7, KEY_KP_7 },
//        { kVK_ANSI_Keypad8, KEY_KP_8 },
//        { kVK_ANSI_Keypad9, KEY_KP_9 },

        /*
    * Auxiliary Functions; note the duplicate definitions for left and right
    * function keys;  Sun keyboards and a few other manufactures have such
    * function key groups on the left and/or right sides of the keyboard.
    * We've not found a keyboard with more than 35 function keys total.
    */

//        { kVK_F1, KEY_F1 },
//        { kVK_F2, KEY_F2 },
//        { kVK_F3, KEY_F3 },
//        { kVK_F4, KEY_F4 },
//        { kVK_F5, KEY_F5 },
//        { kVK_F6, KEY_F6 },
//        { kVK_F7, KEY_F7 },
//        { kVK_F8, KEY_F8 },
//        { kVK_F9, KEY_F9 },
//        { kVK_F10, KEY_F10 },
//        { kVK_F11, KEY_F11 },
//        { kVK_F12, KEY_F12 },
//        { kVK_F13, KEY_F13 },
//        { kVK_F14, KEY_F14 },
//        { kVK_F15, KEY_F15 },
//        { kVK_F16, KEY_F16 },
//        { kVK_F17, KEY_F17 },
//        { kVK_F18, KEY_F18 },
//        { kVK_F19, KEY_F19 },
//        { kVK_F20, KEY_F20 },
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

//        { kVK_Shift, KEY_Shift_L },   /* Left shift */
//        { kVK_RightShift, KEY_Shift_R },  /* Right shift */
//        { kVK_Control, KEY_Control_L },  /* Left control */
        //{ AKEYCODE_CTRL_RIGHT, KEY_Control_R }, // no right control on mac
//        { kVK_CapsLock, KEY_Caps_Lock },  /* Caps lock */
        //KEY_Shift_Lock = 0xFFE6, /* Shift lock */

        //{ AKEYCODE_META_LEFT, KEY_Meta_L },  /* Left meta */
        //{ AKEYCODE_META_RIGHT, KEY_Meta_R }, /* Right meta */
//        { kVK_Option, KEY_Alt_L },    /* Left alt */
//        { kVK_RightOption, KEY_Alt_R },   /* Right alt */
//        { kVK_Command, KEY_Super_L }, /* Left super */
        //{ VK_RWIN, KEY_Super_R } /* Right super */
        //KEY_Hyper_L = 0xFFED, /* Left hyper */
        //KEY_Hyper_R = 0xFFEE  /* Right hyper */
    };
}

//bool KeyboardMap::getKeySymbol(NSEvent* anEvent, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier)
//{
//    unsigned short keycode = [anEvent keyCode];
//    NSEventModifierFlags modifierFlags = [anEvent modifierFlags];
//    //NSLog(@"keycode: %d", keycode);
//    // try find the raw keycode
//    auto itr = _keycodeMap.find(keycode);
//    if (itr == _keycodeMap.end())
//    {
//        // if we don't find it, try the unmodified characters
//        NSString* unmodcharacters = [anEvent charactersIgnoringModifiers];
//        if ( [unmodcharacters length] == 0 ) return false; // dead key
//        unsigned short unmodkeychar = [unmodcharacters characterAtIndex:0];
//        itr = _keycodeMap.find(unmodkeychar);
//        if (itr == _keycodeMap.end()) return false;
//    }

//    keySymbol = itr->second;
//    modifiedKeySymbol = keySymbol;

//    uint16_t modifierMask = 0;

//    if (modifierFlags & NSEventModifierFlagOption) modifierMask |= vsg::KeyModifier::MODKEY_Alt;
//    if (modifierFlags & NSEventModifierFlagControl) modifierMask |= vsg::KeyModifier::MODKEY_Control;
//    if (modifierFlags & NSEventModifierFlagShift) modifierMask |= vsg::KeyModifier::MODKEY_Shift;
//    if (modifierFlags & NSEventModifierFlagCapsLock) modifierMask |= vsg::KeyModifier::MODKEY_CapsLock;
//    if (modifierFlags & NSEventModifierFlagNumericPad) modifierMask |= vsg::KeyModifier::MODKEY_NumLock;

//    keyModifier = (vsg::KeyModifier) modifierMask;

//    if(modifierMask == 0) return true;

//    // try find modified by using characters
//    NSString* characters = [anEvent characters];
//    if ( [characters length] == 0 ) return true; // dead key

//    //NSLog(@"characters: %@", characters);

//    if ( [characters length] == 1 )
//    {
//        unsigned short keychar = [characters characterAtIndex:0];
//        itr = _keycodeMap.find(keychar);
//        if (itr == _keycodeMap.end()) return true; // still return true, we just don't have a specific modified character
//        modifiedKeySymbol = itr->second;
//    }

//    return true;
//}


bool vsgiOS::iOS_Window::handleUIEvent(UIEvent* anEvent)
{
    switch([anEvent type])
    {
        
//        // mouse events
//        case UIEventTypeMouseMoved:
//        case UIEventTypeLeftMouseDown:
//        case UIEventTypeLeftMouseUp:
//        case UIEventTypeLeftMouseDragged:
//        case UIEventTypeRightMouseDown:
//        case UIEventTypeRightMouseUp:
//        case UIEventTypeRightMouseDragged:
//        case UIEventTypeOtherMouseDown:
//        case UIEventTypeOtherMouseUp:
//        case UIEventTypeOtherMouseDragged:
//        {
//            CGRect contentRect = [_view frame];
//            CGPoint pos = [anEvent locationInWindow];
//
//            // dpi scale as needed
//            auto devicePixelScale = _traits->hdpi ? [_window backingScaleFactor] : 1.0f;
//            contentRect.size.width = contentRect.size.width * devicePixelScale;
//            contentRect.size.height = contentRect.size.height * devicePixelScale;
//
//            pos.x = pos.x * devicePixelScale;
//            pos.y = pos.y * devicePixelScale;
//
//
//            NSInteger buttonNumber = [anEvent buttonNumber];
//            NSUInteger pressedButtons = [NSEvent pressedMouseButtons];
//
//            vsg::debug("NSEventTypeMouseMoved(etc): ", pos.x, ", ", pos.y);
//
//            auto buttonMask = 0;
//            if(pressedButtons & (1 << 0)) buttonMask |= vsg::BUTTON_MASK_1;
//            if(pressedButtons & (1 << 1)) buttonMask |= vsg::BUTTON_MASK_2;
//            if(pressedButtons & (1 << 2)) buttonMask |= vsg::BUTTON_MASK_3;
//
//            switch([anEvent type])
//            {
//                case NSEventTypeMouseMoved:
//                case NSEventTypeLeftMouseDragged:
//                case NSEventTypeRightMouseDragged:
//                case NSEventTypeOtherMouseDragged:
//                {
//                    _bufferedEvents.emplace_back(new vsg::MoveEvent(this, getEventTime([anEvent timestamp]), pos.x, contentRect.size.height - pos.y, vsg::ButtonMask(buttonMask)));
//                    break;
//                }
//                case NSEventTypeLeftMouseDown:
//                case NSEventTypeRightMouseDown:
//                case NSEventTypeOtherMouseDown:
//                {
//                    _bufferedEvents.emplace_back(new vsg::ButtonPressEvent(this, getEventTime([anEvent timestamp]), pos.x, contentRect.size.height - pos.y, vsg::ButtonMask(buttonMask), buttonNumber));
//                    break;
//                }
//                case NSEventTypeLeftMouseUp:
//                case NSEventTypeRightMouseUp:
//                case NSEventTypeOtherMouseUp:
//                {
//                    _bufferedEvents.emplace_back(new vsg::ButtonReleaseEvent(this, getEventTime([anEvent timestamp]), pos.x, contentRect.size.height - pos.y, vsg::ButtonMask(buttonMask), buttonNumber));
//                    break;
//                }
//                default: break;
//            }
//            return true;
//        }
        // keyboard events
//        case NSEventTypeKeyDown:
//        case NSEventTypeKeyUp:
//        //case NSEventTypeFlagsChanged:
//        {
//            vsg::KeySymbol keySymbol, modifiedKeySymbol;
//            vsg::KeyModifier keyModifier;
//            if (!_keyboard->getKeySymbol(anEvent, keySymbol, modifiedKeySymbol, keyModifier))
//                return false;
//
//            switch([anEvent type])
//            {
//                case NSEventTypeKeyDown:
//                {
//                    _bufferedEvents.emplace_back(new vsg::KeyPressEvent(this, getEventTime([anEvent timestamp]), keySymbol, modifiedKeySymbol, keyModifier));
//                    break;
//                }
//                case NSEventTypeKeyUp:
//                {
//                    _bufferedEvents.emplace_back(new vsg::KeyReleaseEvent(this, getEventTime([anEvent timestamp]), keySymbol, modifiedKeySymbol, keyModifier));
//                    break;
//                }
//                default: break;
//            }
//
//            return true;
//        }
//        // scrollWheel events
//        case NSEventTypeScrollWheel:
//        {
//            _bufferedEvents.emplace_back(new vsg::ScrollWheelEvent(this, getEventTime([anEvent timestamp]), vsg::vec3([anEvent deltaX], [anEvent deltaY], [anEvent deltaZ])));
//            return true;
//        }

        default: break;
    }
    return false;
}

