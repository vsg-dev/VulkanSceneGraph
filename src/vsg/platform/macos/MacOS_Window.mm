/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/platform/macos/MacOS_Window.h>

#include <vsg/core/Exception.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/TouchEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/vk/Extensions.h>

#include <iostream>
#include <time.h>

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

// this a carbon header, we should try to avoid
#import <Carbon/Carbon.h>

using namespace vsg;
using namespace vsgMacOS;

namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to a MacOS_Window
    ref_ptr<Window> Window::create(vsg::ref_ptr<vsg::WindowTraits> traits)
    {
        return vsgMacOS::MacOS_Window::create(traits);
    }

} // namespace vsg


//------------------------------------------------------------------------
// Application delegate
//------------------------------------------------------------------------

@interface vsg_MacOS_ApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@implementation vsg_MacOS_ApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    return NSTerminateCancel;
}

- (void)applicationDidChangeScreenParameters:(NSNotification *) notification
{
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    [NSApp stop:nil];
}

- (void)applicationDidHide:(NSNotification *)notification
{
}

@end


//------------------------------------------------------------------------
// NSWindow implmentation
//------------------------------------------------------------------------

@interface vsg_MacOS_NSWindow : NSWindow {}
@end

@implementation vsg_MacOS_NSWindow

- (BOOL)canBecomeKeyWindow
{
    //std::cout << "canBecomeKeyWindow" << std::endl;
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end

//------------------------------------------------------------------------
// NSWindow delegate
//------------------------------------------------------------------------

@interface vsg_MacOS_NSWindowDelegate : NSObject <NSWindowDelegate>
{
    vsgMacOS::MacOS_Window* window;
    vsg::ref_ptr<vsg::WindowTraits> _traits;
}

- (instancetype)initWithVsgWindow:(vsgMacOS::MacOS_Window*)initWindow andTraits:(vsg::ref_ptr<vsg::WindowTraits>)traits;

@end

@implementation vsg_MacOS_NSWindowDelegate

- (instancetype)initWithVsgWindow:(vsgMacOS::MacOS_Window*)initWindow andTraits:(vsg::ref_ptr<vsg::WindowTraits>)traits
{
    self = [super init];
    if (self != nil)
    {
        window = initWindow;
        _traits = traits;
    }

    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    vsg::clock::time_point event_time = vsg::clock::now();
    window->queueEvent(vsg::CloseWindowEvent::create(window, event_time));
    return NO;
}

- (void) handleFrameSizeChange
{
    vsg::clock::time_point event_time = vsg::clock::now();
    
    const NSRect contentRect = [window-> view() frame];

    auto devicePixelScale = _traits->hdpi ? [window->window() backingScaleFactor] : 1.0f;

    uint32_t width = contentRect.size.width * devicePixelScale;
    uint32_t height = contentRect.size.height * devicePixelScale;
    
    //std::cout << "handleFrameSizeChange: " << width << ", " << height << std::endl;
    
    window->queueEvent(vsg::ConfigureWindowEvent::create(window, event_time, contentRect.origin.x, contentRect.origin.y, width, height));
}

- (void)windowDidResize:(NSNotification *)notification
{
    [self handleFrameSizeChange];
}

- (void)windowDidMove:(NSNotification *)notification
{
    [self handleFrameSizeChange];
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    [self handleFrameSizeChange];
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    [self handleFrameSizeChange];
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    //std::cout << "windowDidBecomeKey" << std::endl;
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    //std::cout << "windowDidResignKey" << std::endl;
}

@end

//------------------------------------------------------------------------
// NSView implmentation
//------------------------------------------------------------------------

@interface vsg_MacOS_NSView : NSView
{
    vsgMacOS::MacOS_Window* window;
    NSTrackingArea* trackingArea;
}

- (instancetype)initWithVsgWindow:(vsgMacOS::MacOS_Window*)initWindow;

@end

@implementation vsg_MacOS_NSView

- (instancetype)initWithVsgWindow:(vsgMacOS::MacOS_Window*)initWindow
{
    self = [super init];
    if (self != nil)
    {
        window = initWindow;
        [self updateTrackingAreas];
    }

    return self;
}

- (void)dealloc
{
    [trackingArea release];
    [super dealloc];
}

- (BOOL)isOpaque
{
    return true;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)becomeFirstResponder
{
    return YES;
}

- (BOOL)resignFirstResponder
{
    return YES;
}

- (BOOL)wantsUpdateLayer
{
    return YES;
}

- (void)updateLayer
{

}

- (id)makeBackingLayer
{
    if (window->layer())
        return window->layer();

    return [super makeBackingLayer];
}

- (void)cursorUpdate:(NSEvent *)event
{
}

- (void)mouseDown:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)mouseDragged:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)mouseUp:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)mouseMoved:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)rightMouseDown:(NSEvent *)event
{
    window->handleNSEvent(event);
    [super rightMouseDown:event]; // docs state we should call super for right mouse down
}

- (void)rightMouseDragged:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)rightMouseUp:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)otherMouseDown:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)otherMouseDragged:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)otherMouseUp:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)mouseExited:(NSEvent *)event
{
    [window->window() setAcceptsMouseMovedEvents:NO];
}

- (void)mouseEntered:(NSEvent *)event
{
    [window->window() setAcceptsMouseMovedEvents:YES];
}

- (void)viewDidChangeBackingProperties
{
    /*const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];
    */
}

- (void)drawRect:(NSRect)rect
{
}

- (void)keyDown:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)flagsChanged:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)keyUp:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)scrollWheel:(NSEvent *)event
{
    window->handleNSEvent(event);
}

- (void)updateTrackingAreas
{
    if (trackingArea != nil)
    {
        [self removeTrackingArea:trackingArea];
        [trackingArea release];
    }

    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                          NSTrackingActiveInKeyWindow |
                                          NSTrackingEnabledDuringMouseDrag |
                                          NSTrackingCursorUpdate |
                                          NSTrackingInVisibleRect |
                                          NSTrackingAssumeInside;

    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                options:options
                                                  owner:self
                                               userInfo:nil];

    [self addTrackingArea:trackingArea];
    [super updateTrackingAreas];
}

@end



//-----------------

namespace vsgMacOS
{
    class MacOSSurface : public vsg::Surface
    {
    public:
        MacOSSurface(vsg::Instance* instance, NSView* window) :
            vsg::Surface(VK_NULL_HANDLE, instance)
        {
            VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo{};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
            surfaceCreateInfo.pNext = nullptr;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.pView = window;

            /*auto result = */ vkCreateMacOSSurfaceMVK(*instance, &surfaceCreateInfo, _instance->getAllocationCallbacks(), &_surface);
        }
    };

    void createApplicationMenus(void)
    {
        NSString *appName;
        NSString *title;
        NSMenu *appleMenu;
        NSMenuItem *menuItem;

        /* Create the main menu bar */
        [NSApp setMainMenu:[[NSMenu alloc] init]];

        /* Create the application menu */
        appName = @"App Title";
        appleMenu = [[NSMenu alloc] initWithTitle:@""];

        /* Add menu items */
        title = [@"About " stringByAppendingString:appName];
        [appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

        [appleMenu addItem:[NSMenuItem separatorItem]];

        NSMenu* service_menu = [[NSMenu alloc] init];
        NSMenuItem* service_menu_item = [[NSMenuItem alloc] initWithTitle:@"Services" action:nil keyEquivalent:@""];
        [service_menu_item setSubmenu: service_menu];
        [appleMenu addItem: service_menu_item];
        [NSApp setServicesMenu: service_menu];

        [appleMenu addItem:[NSMenuItem separatorItem]];

        title = [@"Hide " stringByAppendingString:appName];
        [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@/*"h"*/"h"];

        menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@/*"h"*/""];
        [menuItem setKeyEquivalentModifierMask:(NSEventModifierFlagOption|NSEventModifierFlagCommand)];

        [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

        [appleMenu addItem:[NSMenuItem separatorItem]];

        title = [@"Quit " stringByAppendingString:appName];
        [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@/*"q"*/"q"];

        /* Put menu into the menubar */
        menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
        [menuItem setSubmenu:appleMenu];
        [[NSApp mainMenu] addItem:menuItem];
        [menuItem release];

        /* Tell the application object that this is now the application menu */
        //[NSApp setServicesMenu:appleMenu];
        //[NSApp setAppleMenu:appleMenu];
        [appleMenu release];
    }

} // namespace vsgMacOS

KeyboardMap::KeyboardMap()
{
    _keycodeMap =
    {
        { 0xFF, KeySymbol::KEY_Undefined },

        { kVK_Space, KeySymbol::KEY_Space },

        { kVK_ANSI_0, KeySymbol::KEY_0 },
        { kVK_ANSI_1, KeySymbol::KEY_1 },
        { kVK_ANSI_2, KeySymbol::KEY_2 },
        { kVK_ANSI_3, KeySymbol::KEY_3 },
        { kVK_ANSI_4, KeySymbol::KEY_4 },
        { kVK_ANSI_5, KeySymbol::KEY_5 },
        { kVK_ANSI_6, KeySymbol::KEY_6 },
        { kVK_ANSI_7, KeySymbol::KEY_7 },
        { kVK_ANSI_8, KeySymbol::KEY_8 },
        { kVK_ANSI_9, KeySymbol::KEY_9 },

        { kVK_ANSI_A, KeySymbol::KEY_a },
        { kVK_ANSI_B, KeySymbol::KEY_b },
        { kVK_ANSI_C, KeySymbol::KEY_c },
        { kVK_ANSI_D, KeySymbol::KEY_d },
        { kVK_ANSI_E, KeySymbol::KEY_e },
        { kVK_ANSI_F, KeySymbol::KEY_f },
        { kVK_ANSI_G, KeySymbol::KEY_g },
        { kVK_ANSI_H, KeySymbol::KEY_h },
        { kVK_ANSI_I, KeySymbol::KEY_i },
        { kVK_ANSI_J, KeySymbol::KEY_j },
        { kVK_ANSI_K, KeySymbol::KEY_k },
        { kVK_ANSI_L, KeySymbol::KEY_l },
        { kVK_ANSI_M, KeySymbol::KEY_m },
        { kVK_ANSI_N, KeySymbol::KEY_n },
        { kVK_ANSI_O, KeySymbol::KEY_o },
        { kVK_ANSI_P, KeySymbol::KEY_p },
        { kVK_ANSI_Q, KeySymbol::KEY_q },
        { kVK_ANSI_R, KeySymbol::KEY_r },
        { kVK_ANSI_S, KeySymbol::KEY_s },
        { kVK_ANSI_T, KeySymbol::KEY_t },
        { kVK_ANSI_U, KeySymbol::KEY_u },
        { kVK_ANSI_Z, KeySymbol::KEY_v },
        { kVK_ANSI_W, KeySymbol::KEY_w },
        { kVK_ANSI_X, KeySymbol::KEY_x },
        { kVK_ANSI_Y, KeySymbol::KEY_y },
        { kVK_ANSI_Z, KeySymbol::KEY_z },

        { 'A', KeySymbol::KEY_A },
        { 'B', KeySymbol::KEY_B },
        { 'C', KeySymbol::KEY_C },
        { 'D', KeySymbol::KEY_D },
        { 'E', KeySymbol::KEY_E },
        { 'F', KeySymbol::KEY_F },
        { 'G', KeySymbol::KEY_G },
        { 'H', KeySymbol::KEY_H },
        { 'I', KeySymbol::KEY_I },
        { 'J', KeySymbol::KEY_J },
        { 'K', KeySymbol::KEY_K },
        { 'L', KeySymbol::KEY_L },
        { 'M', KeySymbol::KEY_M },
        { 'N', KeySymbol::KEY_N },
        { 'O', KeySymbol::KEY_O },
        { 'P', KeySymbol::KEY_P },
        { 'Q', KeySymbol::KEY_Q },
        { 'R', KeySymbol::KEY_R },
        { 'S', KeySymbol::KEY_S },
        { 'T', KeySymbol::KEY_T },
        { 'U', KeySymbol::KEY_U },
        { 'V', KeySymbol::KEY_V },
        { 'W', KeySymbol::KEY_W },
        { 'X', KeySymbol::KEY_X },
        { 'Y', KeySymbol::KEY_Y },
        { 'Z', KeySymbol::KEY_Z },

        { '!', KeySymbol::KEY_Exclaim },
        { '"', KeySymbol::KEY_Quotedbl },
        { '#', KeySymbol::KEY_Hash },
        { '$', KeySymbol::KEY_Dollar },
        { '&', KeySymbol::KEY_Ampersand },
        { kVK_ANSI_Quote, KeySymbol::KEY_Quote },
        { '(', KeySymbol::KEY_Leftparen },
        { ')', KeySymbol::KEY_Rightparen },
        { '*', KeySymbol::KEY_Asterisk },
        { '+', KeySymbol::KEY_Plus },
        { kVK_ANSI_Comma, KeySymbol::KEY_Comma },
        { kVK_ANSI_Minus, KeySymbol::KEY_Minus },
        { kVK_ANSI_Period, KeySymbol::KEY_Period },
        { kVK_ANSI_Slash, KeySymbol::KEY_Slash },
        { ':', KeySymbol::KEY_Colon },
        { kVK_ANSI_Semicolon, KeySymbol::KEY_Semicolon },
        { '<', KeySymbol::KEY_Less },
        { kVK_ANSI_Equal, KeySymbol::KEY_Equals }, // + isnt an unmodded key, why does windows map is as a virtual??
        { '>', KeySymbol::KEY_Greater },
        { '?', KeySymbol::KEY_Question },
        { '@', KeySymbol::KEY_At},
        { kVK_ANSI_LeftBracket, KeySymbol::KEY_Leftbracket },
        { kVK_ANSI_Backslash, KeySymbol::KEY_Backslash },
        { kVK_ANSI_RightBracket, KeySymbol::KEY_Rightbracket },
        {'|', KeySymbol::KEY_Caret },
        {'_', KeySymbol::KEY_Underscore },
        {'`', KeySymbol::KEY_Backquote },

        { kVK_Delete, KeySymbol::KEY_BackSpace }, /* back space, back char */
        { kVK_Tab, KeySymbol::KEY_Tab },
        //    KEY_Linefeed = 0xFF0A, /* Linefeed, LF */
        //{ AKEYCODE_CLEAR, KEY_Clear },
        { kVK_Return, KeySymbol::KEY_Return }, /* Return, enter */
        //{ AKEYCODE_BREAK, KEY_Pause },  /* Pause, hold */
        //{ AKEYCODE_SCROLL_LOCK, KEY_Scroll_Lock },
        //    KEY_Sys_Req = 0xFF15,
        { kVK_Escape, KeySymbol::KEY_Escape },
        { kVK_ForwardDelete, KeySymbol::KEY_Delete }, /* Delete, rubout */

        /* Cursor control & motion */

        { kVK_Home, KeySymbol::KEY_Home },
        { kVK_LeftArrow, KeySymbol::KEY_Left },          /* Move left, left arrow */
        { kVK_UpArrow, KeySymbol::KEY_Up },              /* Move up, up arrow */
        { kVK_RightArrow, KeySymbol::KEY_Right },        /* Move right, right arrow */
        { kVK_DownArrow, KeySymbol::KEY_Down },          /* Move down, down arrow */
        //{ AKEYCODE_NAVIGATE_PREVIOUS, KEY_Prior }, /* Prior, previous */
        { kVK_PageUp, KeySymbol::KEY_Page_Up },
        //{ AKEYCODE_NAVIGATE_NEXT, KEY_Next }, /* Next */
        { kVK_PageDown, KeySymbol::KEY_Page_Down },
        { kVK_End, KeySymbol::KEY_End }, /* EOL */
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
        { kVK_Help, KeySymbol::KEY_Help }, /* Help */
        //{ KEY_Break = 0xFF6B,
        //KEY_Mode_switch = 0xFF7E,   /* Character set switch */
        //KEY_Script_switch = 0xFF7E, /* Alias for mode_switch */
        //{ AKEYCODE_NUM_LOCK, KEY_Num_Lock },

        /* Keypad Functions, keypad numbers cleverly chosen to map to ascii */

        //KEY_KP_Space = 0xFF80, /* space */
        //KEY_KP_Tab = 0xFF89,
        { kVK_ANSI_KeypadEnter, KeySymbol::KEY_KP_Enter }, /* enter */
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
        { kVK_ANSI_KeypadEquals, KeySymbol::KEY_KP_Equal }, /* equals */
        { kVK_ANSI_KeypadMultiply, KeySymbol::KEY_KP_Multiply },
        { kVK_ANSI_KeypadPlus, KeySymbol::KEY_KP_Add },
        //{ AKEYCODE_NUMPAD_COMMA, KEY_KP_Separator }, /* separator, often comma */
        { kVK_ANSI_KeypadMinus, KeySymbol::KEY_KP_Subtract },
        { kVK_ANSI_KeypadDecimal, KeySymbol::KEY_KP_Decimal },
        { kVK_ANSI_KeypadDivide, KeySymbol::KEY_KP_Divide },

        { kVK_ANSI_Keypad0, KeySymbol::KEY_KP_0 },
        { kVK_ANSI_Keypad1, KeySymbol::KEY_KP_1 },
        { kVK_ANSI_Keypad2, KeySymbol::KEY_KP_2 },
        { kVK_ANSI_Keypad3, KeySymbol::KEY_KP_3 },
        { kVK_ANSI_Keypad4, KeySymbol::KEY_KP_4 },
        { kVK_ANSI_Keypad5, KeySymbol::KEY_KP_5 },
        { kVK_ANSI_Keypad6, KeySymbol::KEY_KP_6 },
        { kVK_ANSI_Keypad7, KeySymbol::KEY_KP_7 },
        { kVK_ANSI_Keypad8, KeySymbol::KEY_KP_8 },
        { kVK_ANSI_Keypad9, KeySymbol::KEY_KP_9 },

        /*
    * Auxiliary Functions; note the duplicate definitions for left and right
    * function keys;  Sun keyboards and a few other manufactures have such
    * function key groups on the left and/or right sides of the keyboard.
    * We've not found a keyboard with more than 35 function keys total.
    */

        { kVK_F1, KeySymbol::KEY_F1 },
        { kVK_F2, KeySymbol::KEY_F2 },
        { kVK_F3, KeySymbol::KEY_F3 },
        { kVK_F4, KeySymbol::KEY_F4 },
        { kVK_F5, KeySymbol::KEY_F5 },
        { kVK_F6, KeySymbol::KEY_F6 },
        { kVK_F7, KeySymbol::KEY_F7 },
        { kVK_F8, KeySymbol::KEY_F8 },
        { kVK_F9, KeySymbol::KEY_F9 },
        { kVK_F10, KeySymbol::KEY_F10 },
        { kVK_F11, KeySymbol::KEY_F11 },
        { kVK_F12, KeySymbol::KEY_F12 },
        { kVK_F13, KeySymbol::KEY_F13 },
        { kVK_F14, KeySymbol::KEY_F14 },
        { kVK_F15, KeySymbol::KEY_F15 },
        { kVK_F16, KeySymbol::KEY_F16 },
        { kVK_F17, KeySymbol::KEY_F17 },
        { kVK_F18, KeySymbol::KEY_F18 },
        { kVK_F19, KeySymbol::KEY_F19 },
        { kVK_F20, KeySymbol::KEY_F20 },
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

        { kVK_Shift, KeySymbol::KEY_Shift_L },   /* Left shift */
        { kVK_RightShift, KeySymbol::KEY_Shift_R },  /* Right shift */
        { kVK_Control, KeySymbol::KEY_Control_L },  /* Left control */
        //{ AKEYCODE_CTRL_RIGHT, KEY_Control_R }, // no right control on mac
        { kVK_CapsLock, KeySymbol::KEY_Caps_Lock },  /* Caps lock */
        //KEY_Shift_Lock = 0xFFE6, /* Shift lock */

        //{ AKEYCODE_META_LEFT, KEY_Meta_L },  /* Left meta */
        //{ AKEYCODE_META_RIGHT, KEY_Meta_R }, /* Right meta */
        { kVK_Option, KeySymbol::KEY_Alt_L },    /* Left alt */
        { kVK_RightOption, KeySymbol::KEY_Alt_R },   /* Right alt */
        { kVK_Command, KeySymbol::KEY_Super_L }, /* Left super */
        //{ VK_RWIN, KEY_Super_R } /* Right super */
        //KEY_Hyper_L = 0xFFED, /* Left hyper */
        //KEY_Hyper_R = 0xFFEE  /* Right hyper */
    };
}

bool KeyboardMap::getKeySymbol(NSEvent* anEvent, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier)
{
    unsigned short keycode = [anEvent keyCode];
    NSEventModifierFlags modifierFlags = [anEvent modifierFlags];
    //NSLog(@"keycode: %d", keycode);
    // try find the raw keycode
    auto itr = _keycodeMap.find(keycode);
    if (itr == _keycodeMap.end())
    {
        // if we don't find it, try the unmodified characters
        NSString* unmodcharacters = [anEvent charactersIgnoringModifiers];
        if ( [unmodcharacters length] == 0 ) return false; // dead key
        unsigned short unmodkeychar = [unmodcharacters characterAtIndex:0];
        itr = _keycodeMap.find(unmodkeychar);
        if (itr == _keycodeMap.end()) return false;
    }

    keySymbol = itr->second;
    modifiedKeySymbol = keySymbol;

    uint16_t modifierMask = 0;

    if (modifierFlags & NSEventModifierFlagOption) modifierMask |= static_cast<uint16_t>(vsg::KeyModifier::MODKEY_Alt);
    if (modifierFlags & NSEventModifierFlagControl) modifierMask |= static_cast<uint16_t>(vsg::KeyModifier::MODKEY_Control);
    if (modifierFlags & NSEventModifierFlagShift) modifierMask |= static_cast<uint16_t>(vsg::KeyModifier::MODKEY_Shift);
    if (modifierFlags & NSEventModifierFlagCapsLock) modifierMask |= static_cast<uint16_t>(vsg::KeyModifier::MODKEY_CapsLock);
    if (modifierFlags & NSEventModifierFlagNumericPad) modifierMask |= static_cast<uint16_t>(vsg::KeyModifier::MODKEY_NumLock);

    keyModifier = static_cast<vsg::KeyModifier>(modifierMask);

    if(modifierMask == 0) return true;

    // try find modified by using characters
    NSString* characters = [anEvent characters];
    if ( [characters length] == 0 ) return true; // dead key

    //NSLog(@"characters: %@", characters);

    if ( [characters length] == 1 )
    {
        unsigned short keychar = [characters characterAtIndex:0];
        itr = _keycodeMap.find(keychar);
        if (itr == _keycodeMap.end()) return true; // still return true, we just don't have a specific modified character
        modifiedKeySymbol = itr->second;
    }

    return true;
}

MacOS_Window::MacOS_Window(vsg::ref_ptr<vsg::WindowTraits> traits) :
    Inherit(traits)
{
    _keyboard = new KeyboardMap;

    NSRect contentRect = NSMakeRect(0, 0, traits->width, traits->height);

    NSWindowStyleMask styleMask = 0;
    if(!traits->decoration)
    {
        styleMask |= NSWindowStyleMaskBorderless;
    }
    else
    {
        styleMask |= NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    }

    // create app
    [NSApplication sharedApplication];
    vsg_MacOS_ApplicationDelegate* appDelegate = [[vsg_MacOS_ApplicationDelegate alloc] init];
    [NSApp setDelegate:appDelegate];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    //[NSApp run];

    // create metal layer
    _metalLayer = [[CAMetalLayer alloc] init];
    if (!_metalLayer)
    {
        throw Exception{"Error: vsg::MacOS_Window::MacOS_Window(...) failed to create Window, unable to create Metal layer for view.", VK_ERROR_INVALID_EXTERNAL_HANDLE};
    }

    // create window
    _window = [[vsg_MacOS_NSWindow alloc] initWithContentRect:contentRect
                                                              styleMask:styleMask
                                                              backing:NSBackingStoreBuffered
                                                              defer:NO];

    vsg_MacOS_NSWindowDelegate* windowDelegate = [[vsg_MacOS_NSWindowDelegate alloc] initWithVsgWindow:this andTraits:traits];
    [_window setDelegate:windowDelegate];

    [_window setTitle:[NSString stringWithUTF8String:traits->windowTitle.c_str()]];
    [_window setAcceptsMouseMovedEvents:NO];
    [_window setRestorable:NO];
    [_window setOpaque:YES];
    [_window setBackgroundColor:[NSColor whiteColor]];

    // create view
    _view = [[vsg_MacOS_NSView alloc] initWithVsgWindow:this];
    [_view setWantsBestResolutionOpenGLSurface:_traits->hdpi];
    [_view setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable) ];
    [_view setWantsLayer:YES];

    // attach view to window
    [_window setContentView:_view];
    _window.initialFirstResponder = _view;
    [_window makeFirstResponder:_view];

    auto devicePixelScale = _traits->hdpi ? [_window backingScaleFactor] : 1.0f;
    [_metalLayer setContentsScale:devicePixelScale];

    // we could get the width height from the window?
    uint32_t finalwidth = traits->width * devicePixelScale;
    uint32_t finalheight = traits->height * devicePixelScale;

    if (traits->shareWindow)
    {
        // share the _instance, _physicalDevice and _device;
        share(*traits->shareWindow);
    }

    _extent2D.width = finalwidth;
    _extent2D.height = finalheight;

    _first_macos_timestamp = [[NSProcessInfo processInfo] systemUptime];
    _first_macos_time_point = vsg::clock::now();

    // show
    //vsgMacOS::createApplicationMenus();
    [NSApp activateIgnoringOtherApps:YES];
    [_window makeKeyAndOrderFront:nil];
    
    // manually trigger configure here??
    vsg::clock::time_point event_time = vsg::clock::now();
    bufferedEvents.emplace_back(vsg::ConfigureWindowEvent::create(this, event_time, _traits->x, _traits->y, finalwidth, finalheight));
}

MacOS_Window::~MacOS_Window()
{
    clear();
}

void MacOS_Window::_initSurface()
{
    if (!_instance) _initInstance();

    _surface = new vsgMacOS::MacOSSurface(_instance, _view);
}

bool MacOS_Window::pollEvents(vsg::UIEvents& events)
{
    for (;;)
    {
        NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:[NSDate distantPast]
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        if (event == nil)
            break;

        [NSApp sendEvent:event];
    }

    return Window::pollEvents(events);
}

void MacOS_Window::resize()
{
    const NSRect contentRect = [_view frame];

    auto devicePixelScale = _traits->hdpi ? [_window backingScaleFactor] : 1.0f;
    //[_metalLayer setContentsScale:devicePixelScale];

    _extent2D.width = contentRect.size.width * devicePixelScale;
    _extent2D.height = contentRect.size.height * devicePixelScale;

    buildSwapchain();
}

bool MacOS_Window::handleNSEvent(NSEvent* anEvent)
{
    switch([anEvent type])
    {
        // mouse events
        case NSEventTypeMouseMoved:
        case NSEventTypeLeftMouseDown:
        case NSEventTypeLeftMouseUp:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDown:
        case NSEventTypeRightMouseUp:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDown:
        case NSEventTypeOtherMouseUp:
        case NSEventTypeOtherMouseDragged:
        {
            NSRect contentRect = [_view frame];
            NSPoint pos = [anEvent locationInWindow];
            
            // dpi scale as needed
            auto devicePixelScale = _traits->hdpi ? [_window backingScaleFactor] : 1.0f;
            contentRect.size.width = contentRect.size.width * devicePixelScale;
            contentRect.size.height = contentRect.size.height * devicePixelScale;
            
            pos.x = pos.x * devicePixelScale;
            pos.y = pos.y * devicePixelScale;
            
            
            NSInteger buttonNumber = [anEvent buttonNumber];
            NSUInteger pressedButtons = [NSEvent pressedMouseButtons];
            
            //std::cout << "NSEventTypeMouseMoved(etc): " << pos.x << ", " << pos.y << std::endl;

            auto buttonMask = 0;
            if(pressedButtons & (1 << 0)) buttonMask |= vsg::BUTTON_MASK_1;
            if(pressedButtons & (1 << 1)) buttonMask |= vsg::BUTTON_MASK_2;
            if(pressedButtons & (1 << 2)) buttonMask |= vsg::BUTTON_MASK_3;

            switch([anEvent type])
            {
                case NSEventTypeMouseMoved:
                case NSEventTypeLeftMouseDragged:
                case NSEventTypeRightMouseDragged:
                case NSEventTypeOtherMouseDragged:
                {
                    bufferedEvents.emplace_back(vsg::MoveEvent::create(this, getEventTime([anEvent timestamp]), pos.x, contentRect.size.height - pos.y, vsg::ButtonMask(buttonMask)));
                    break;
                }
                case NSEventTypeLeftMouseDown:
                case NSEventTypeRightMouseDown:
                case NSEventTypeOtherMouseDown:
                {
                    bufferedEvents.emplace_back(vsg::ButtonPressEvent::create(this, getEventTime([anEvent timestamp]), pos.x, contentRect.size.height - pos.y, vsg::ButtonMask(buttonMask), buttonNumber));
                    break;
                }
                case NSEventTypeLeftMouseUp:
                case NSEventTypeRightMouseUp:
                case NSEventTypeOtherMouseUp:
                {
                    bufferedEvents.emplace_back(vsg::ButtonReleaseEvent::create(this, getEventTime([anEvent timestamp]), pos.x, contentRect.size.height - pos.y, vsg::ButtonMask(buttonMask), buttonNumber));
                    break;
                }
                default: break;
            }
            return true;
        }
        // keyboard events
        case NSEventTypeKeyDown:
        case NSEventTypeKeyUp:
        //case NSEventTypeFlagsChanged:
        {
            vsg::KeySymbol keySymbol, modifiedKeySymbol;
            vsg::KeyModifier keyModifier;
            if (!_keyboard->getKeySymbol(anEvent, keySymbol, modifiedKeySymbol, keyModifier))
                return false;

            switch([anEvent type])
            {
                case NSEventTypeKeyDown:
                {
                    bufferedEvents.emplace_back(vsg::KeyPressEvent::create(this, getEventTime([anEvent timestamp]), keySymbol, modifiedKeySymbol, keyModifier));
                    break;
                }
                case NSEventTypeKeyUp:
                {
                    bufferedEvents.emplace_back(vsg::KeyReleaseEvent::create(this, getEventTime([anEvent timestamp]), keySymbol, modifiedKeySymbol, keyModifier));
                    break;
                }
                default: break;
            }

            return true;
        }
        // scrollWheel events
        case NSEventTypeScrollWheel:
        {
            bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(this, getEventTime([anEvent timestamp]), vsg::vec3([anEvent deltaX], [anEvent deltaY], [anEvent deltaZ])));
            return true;
        }

        default: break;
    }
    return false;
}
