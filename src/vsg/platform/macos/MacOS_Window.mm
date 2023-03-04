/* <editor-fold desc="MIT License">

Copyright(c) 2018 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsg/platform/macos/MacOS_Window.h>

#include <vsg/core/Exception.h>
#include <vsg/core/observer_ptr.h>
#include <vsg/io/Logger.h>
#include <vsg/ui/KeyEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/TouchEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>
#include <vsg/vk/Extensions.h>

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
// NSWindow implementation
//------------------------------------------------------------------------

@interface vsg_MacOS_NSWindow : NSWindow {}
@end

@implementation vsg_MacOS_NSWindow

- (BOOL)canBecomeKeyWindow
{
    //vsg:debug("canBecomeKeyWindow");
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
    
    //vsg:debug("handleFrameSizeChange: ", width, ", ", height);
    
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
    //vsg:debug("windowDidBecomeKey");
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    //vsg:debug("windowDidResignKey");
}

@end

//------------------------------------------------------------------------
// NSView implementation
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
    // An explanation of mapping from uppercase to lowercase
    // The map _vk2vsg represents mapping from MacOS produced keycode
    // of the key that was physically pressed to the vsg character
    // that is produced when the key is pressed and no modifiers are active.
    // clang-format off
    _vk2vsg =
    {
        {kVK_ANSI_A,              vsg::KEY_a},
        {kVK_ANSI_S,              vsg::KEY_s},
        {kVK_ANSI_D,              vsg::KEY_d},
        {kVK_ANSI_F,              vsg::KEY_f},
        {kVK_ANSI_H,              vsg::KEY_h},
        {kVK_ANSI_G,              vsg::KEY_g},
        {kVK_ANSI_Z,              vsg::KEY_z},
        {kVK_ANSI_X,              vsg::KEY_x},
        {kVK_ANSI_C,              vsg::KEY_c},
        {kVK_ANSI_V,              vsg::KEY_v},
        {kVK_ANSI_B,              vsg::KEY_b},
        {kVK_ANSI_Q,              vsg::KEY_q},
        {kVK_ANSI_W,              vsg::KEY_w},
        {kVK_ANSI_E,              vsg::KEY_e},
        {kVK_ANSI_R,              vsg::KEY_r},
        {kVK_ANSI_Y,              vsg::KEY_y},
        {kVK_ANSI_T,              vsg::KEY_t},
        {kVK_ANSI_1,              vsg::KEY_1},
        {kVK_ANSI_2,              vsg::KEY_2},
        {kVK_ANSI_3,              vsg::KEY_3},
        {kVK_ANSI_4,              vsg::KEY_4},
        {kVK_ANSI_6,              vsg::KEY_6},
        {kVK_ANSI_5,              vsg::KEY_5},
        {kVK_ANSI_Equal,          vsg::KEY_Equals},
        {kVK_ANSI_9,              vsg::KEY_9},
        {kVK_ANSI_7,              vsg::KEY_7},
        {kVK_ANSI_Minus,          vsg::KEY_Minus},
        {kVK_ANSI_8,              vsg::KEY_8},
        {kVK_ANSI_0,              vsg::KEY_0},
        {kVK_ANSI_RightBracket,   vsg::KEY_Rightbracket},
        {kVK_ANSI_O,              vsg::KEY_o},
        {kVK_ANSI_U,              vsg::KEY_u},
        {kVK_ANSI_LeftBracket,    vsg::KEY_Leftbracket},
        {kVK_ANSI_I,              vsg::KEY_i},
        {kVK_ANSI_P,              vsg::KEY_p},
        {kVK_ANSI_L,              vsg::KEY_l},
        {kVK_ANSI_J,              vsg::KEY_j},
        {kVK_ANSI_Quote,          vsg::KEY_Quote},
        {kVK_ANSI_K,              vsg::KEY_k},
        {kVK_ANSI_Semicolon,      vsg::KEY_Semicolon},
        {kVK_ANSI_Backslash,      vsg::KEY_Backslash},
        {kVK_ANSI_Comma,          vsg::KEY_Comma},
        {kVK_ANSI_Slash,          vsg::KEY_Slash},
        {kVK_ANSI_N,              vsg::KEY_n},
        {kVK_ANSI_M,              vsg::KEY_m},
        {kVK_ANSI_Period,         vsg::KEY_Period},
        {kVK_ANSI_Grave,          vsg::KEY_Backquote},
        {kVK_ANSI_KeypadDecimal,  vsg::KEY_KP_Decimal},
        {kVK_ANSI_KeypadMultiply, vsg::KEY_KP_Multiply},
        {kVK_ANSI_KeypadPlus,     vsg::KEY_KP_Add},
        {kVK_ANSI_KeypadClear,    vsg::KEY_Undefined},
        {kVK_ANSI_KeypadDivide,   vsg::KEY_KP_Divide},
        {kVK_ANSI_KeypadEnter,    vsg::KEY_KP_Enter},
        {kVK_ANSI_KeypadMinus,    vsg::KEY_KP_Subtract},
        {kVK_ANSI_KeypadEquals,   vsg::KEY_KP_Equal},
        {kVK_ANSI_Keypad0,        vsg::KEY_KP_0},
        {kVK_ANSI_Keypad1,        vsg::KEY_KP_1},
        {kVK_ANSI_Keypad2,        vsg::KEY_KP_2},
        {kVK_ANSI_Keypad3,        vsg::KEY_KP_3},
        {kVK_ANSI_Keypad4,        vsg::KEY_KP_4},
        {kVK_ANSI_Keypad5,        vsg::KEY_KP_5},
        {kVK_ANSI_Keypad6,        vsg::KEY_KP_6},
        {kVK_ANSI_Keypad7,        vsg::KEY_KP_7},
        {kVK_ANSI_Keypad8,        vsg::KEY_KP_8},
        {kVK_ANSI_Keypad9,        vsg::KEY_KP_9},
        {kVK_Return,              vsg::KEY_Return},
        {kVK_Tab,                 vsg::KEY_Tab},
        {kVK_Space,               vsg::KEY_Space},
        {kVK_Delete,              vsg::KEY_BackSpace},
        {kVK_Escape,              vsg::KEY_Escape},
        {kVK_Command,             vsg::KEY_Meta_L},
        {kVK_Shift,               vsg::KEY_Shift_L},
        {kVK_CapsLock,            vsg::KEY_Caps_Lock},
        {kVK_Option,              vsg::KEY_Undefined},
        {kVK_Control,             vsg::KEY_Control_L},
        {kVK_RightCommand,        vsg::KEY_Meta_R},
        {kVK_RightShift,          vsg::KEY_Shift_R},
        {kVK_RightOption,         vsg::KEY_Alt_R},
        {kVK_RightControl,        vsg::KEY_Control_R},
        {kVK_Function,            vsg::KEY_Undefined},
        {kVK_F17,                 vsg::KEY_F17},
        {kVK_VolumeUp,            vsg::KEY_Undefined},
        {kVK_VolumeDown,          vsg::KEY_Undefined},
        {kVK_Mute,                vsg::KEY_Undefined},
        {kVK_F18,                 vsg::KEY_F18},
        {kVK_F19,                 vsg::KEY_F19},
        {kVK_F20,                 vsg::KEY_F20},
        {kVK_F5,                  vsg::KEY_F5},
        {kVK_F6,                  vsg::KEY_F6},
        {kVK_F7,                  vsg::KEY_F7},
        {kVK_F3,                  vsg::KEY_F3},
        {kVK_F8,                  vsg::KEY_F8},
        {kVK_F9,                  vsg::KEY_F9},
        {kVK_F11,                 vsg::KEY_F11},
        {kVK_F13,                 vsg::KEY_F13},
        {kVK_F16,                 vsg::KEY_F16},
        {kVK_F14,                 vsg::KEY_F14},
        {kVK_F10,                 vsg::KEY_F10},
        {kVK_F12,                 vsg::KEY_F12},
        {kVK_F15,                 vsg::KEY_F15},
        {kVK_Help,                vsg::KEY_Help},
        {kVK_Home,                vsg::KEY_Home},
        {kVK_PageUp,              vsg::KEY_Page_Up},
        {kVK_ForwardDelete,       vsg::KEY_Delete},
        {kVK_F4,                  vsg::KEY_F4},
        {kVK_End,                 vsg::KEY_End},
        {kVK_F2,                  vsg::KEY_F2},
        {kVK_PageDown,            vsg::KEY_Page_Down},
        {kVK_F1,                  vsg::KEY_F1},
        {kVK_LeftArrow,           vsg::KEY_Left},
        {kVK_RightArrow,          vsg::KEY_Right},
        {kVK_DownArrow,           vsg::KEY_Down},
        {kVK_UpArrow,             vsg::KEY_Up}
    };
    // clang-format on
}

void KeyboardMap::getModifierKeyChanges(NSEvent* anEvent, ModifierKeyChanges& changes)
{
    NSEventModifierFlags modifierFlags = [anEvent modifierFlags];
    // Then save the current flags for next time around.
    NSEventModifierFlags changedFlags = _lastFlags ^ modifierFlags; // xor the last and now to get what changedFlags.
    _lastFlags = modifierFlags; // this must come after the xor.

    // The below code could likely be accomplished with just bit masks but the if statements are clearer.
    // Work out any mod keys such as ctrl, alt, shift etc. are pressed
    // There must be a way to differentiate between left and right modifier keys on Mac
    // But for now I do not know how do do so. Hence for all modifier keys the "left" key is chosen.
    if (changedFlags & NSEventModifierFlagOption)
    {
        if (modifierFlags & NSEventModifierFlagOption)
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Alt_L, true));
        }
        else
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Alt_L, false));
        }
    }
    if (changedFlags & NSEventModifierFlagControl)
    {
        if (modifierFlags & NSEventModifierFlagControl)
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Control_L, true));
        }
        else
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Control_L, false));
        }
    }
    if (changedFlags & NSEventModifierFlagShift)
    {
        if (modifierFlags & NSEventModifierFlagShift)
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Shift_L, true));
        }
        else
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Shift_L, false));
        }
    }
    if (changedFlags & NSEventModifierFlagCapsLock)
    {
        if (modifierFlags & NSEventModifierFlagCapsLock)
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Caps_Lock, true));
        }
        else
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Caps_Lock, false));
        }
    }
    if (changedFlags & NSEventModifierFlagNumericPad)
    {
        if (modifierFlags & NSEventModifierFlagNumericPad)
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Num_Lock, true));
        }
        else
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Num_Lock, false));
        }
    }
    if (changedFlags & NSEventModifierFlagCommand)
    {
        if (modifierFlags & NSEventModifierFlagCommand)
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Meta_L, true));
        }
        else
        {
            changes.emplace_back(KeySymbolState(vsg::KEY_Meta_L, false));
        }
    }
}

bool KeyboardMap::getKeySymbol(NSEvent* anEvent, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier)
{
    unsigned short keycode = [anEvent keyCode];
    NSEventModifierFlags modifierFlags = [anEvent modifierFlags];
    //NSLog(@"keycode: %d", keycode);
    // try find the raw keycode
    auto itr = _vk2vsg.find(keycode);
    if (itr == _vk2vsg.end())
    {
        // if we don't find it, try the unmodified characters
        NSString* unmodcharacters = [anEvent charactersIgnoringModifiers];
        if ( [unmodcharacters length] == 0 ) return false; // dead key
        unsigned short unmodkeychar = [unmodcharacters characterAtIndex:0];
        itr = _vk2vsg.find(unmodkeychar);
        if (itr == _vk2vsg.end()) return false;
    }

    // Base unmodified key as a vsg::KEY_
    keySymbol = itr->second;
    modifiedKeySymbol = keySymbol;

    // Work out any mod keys such as ctrl, alt, shift etc. are pressed
    uint16_t modifierMask = 0;
    if (modifierFlags & NSEventModifierFlagOption) modifierMask |= vsg::KeyModifier::MODKEY_Alt;
    if (modifierFlags & NSEventModifierFlagControl) modifierMask |= vsg::KeyModifier::MODKEY_Control;
    if (modifierFlags & NSEventModifierFlagShift) modifierMask |= vsg::KeyModifier::MODKEY_Shift;
    if (modifierFlags & NSEventModifierFlagCapsLock) modifierMask |= vsg::KeyModifier::MODKEY_CapsLock;
    if (modifierFlags & NSEventModifierFlagNumericPad) modifierMask |= vsg::KeyModifier::MODKEY_NumLock;

    // cast it to a vsg::KeyModifier type
    keyModifier = (vsg::KeyModifier) modifierMask;

    if (modifierMask == 0) return true;

    // try find modified by using characters
    NSString* characters = [anEvent characters];
    if ([characters length] == 0) return true; // dead key

    //NSLog(@"characters: %@", characters);

    if ([characters length] == 1)
    {
        unsigned short keychar = [characters characterAtIndex:0];
        modifiedKeySymbol = static_cast<vsg::KeySymbol>(keychar);
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
    [_window setAcceptsMouseMovedEvents:YES];
    [_window setRestorable:YES];
    [_window setOpaque:YES];
    [_window setBackgroundColor:[NSColor whiteColor]];

    
    // create view
    _view = [[vsg_MacOS_NSView alloc] initWithVsgWindow:this];
    // [_view setWantsBestResolutionOpenGLSurface:_traits->hdpi];
    [_view setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable) ];
    [_view setWantsLayer:YES];

    // attach view to window
    [_window setContentView:_view];
    _window.initialFirstResponder = _view;
    [_window makeFirstResponder:_view];

    if (traits->fullscreen)
    {
        NSRect screenFrame = [[NSScreen mainScreen] frame];
        [_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
        [_window toggleFullScreen:NSApp.delegate];
    }

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

   // set the top left corner window position as offset from the top left corner of the screen
    NSPoint pos;
    int xmax = [[NSScreen mainScreen] frame].size.width - [_window frame].size.width;
    int ymax = [[NSScreen mainScreen] frame].size.height - [_window frame].size.height;
    pos.x = std::clamp(traits->x, 0, xmax);
    pos.y = ymax - std::clamp(traits->y, 0, ymax);
    // show
    [_window setFrame:CGRectMake(pos.x, pos.y, [_window frame].size.width, [_window frame].size.height) display:YES];

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

            //vsg:debug("NSEventTypeMouseMoved(etc): ", pos.x, ", ", pos.y);

            auto buttonMask = 0;
            if (pressedButtons & (1 << 0)) buttonMask |= vsg::BUTTON_MASK_1;
            if (pressedButtons & (1 << 1)) buttonMask |= vsg::BUTTON_MASK_2;
            if (pressedButtons & (1 << 2)) buttonMask |= vsg::BUTTON_MASK_3;

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
        case NSEventTypeFlagsChanged:
        {
            // This event type is triggered when ever a ctrl, alt etc keys are pressed.
            // Not sure why this is not just a KeyUp or KeyDown event.
            // both the modified and unmodified symbols for ctrl, alt, option, etc are the same
            // no modifiers for the modifiers themselves.
            vsg::KeyModifier modifier = static_cast<vsg::KeyModifier>(0);
            ModifierKeyChanges modifierKeyChanges;
            _keyboard->getModifierKeyChanges(anEvent, modifierKeyChanges);

            // If none of the flags that changed interest us, return indicating we did not process it.
//            if (modifierKeyChanges.empty()) { return false; }

            // otherwise loop through the flag keysymbols and create events.
            for (auto &pair: modifierKeyChanges)
            {
                if (pair.second)
                {
                    // key was pressed
                    bufferedEvents.emplace_back( vsg::KeyPressEvent::create(this, getEventTime([anEvent timestamp]), pair.first, pair.first, modifier) );
                }
                else
                {
                    // key was released
                    bufferedEvents.emplace_back( vsg::KeyReleaseEvent::create(this, getEventTime([anEvent timestamp]), pair.first, pair.first, modifier) );
                }
            }
            return true;
        }
        case NSEventTypeKeyDown:
        case NSEventTypeKeyUp:
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
