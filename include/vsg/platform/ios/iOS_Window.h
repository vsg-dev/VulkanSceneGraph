//
//  vsg_iOS_ApplicationDelegate.h
//  IOS_vsg_native_example
//
//  Created by jaume dominguez faus on 23/5/21.
//

#ifndef vsg_iOS_Window_h
#define vsg_iOS_Window_h



#include <vsg/app/Window.h>
#include <vsg/ui/KeyEvent.h>
#include <vulkan/vulkan_metal.h>
#include <UIKit/UiKit.h>

@interface vsg_iOS_Window : UIWindow
- (vsg::ref_ptr<vsg::Window>) vsgWindow;
- (instancetype)initWithTraits:(vsg::ref_ptr<vsg::WindowTraits>)traits andVsgViewer:(vsg::ref_ptr<vsg::Viewer>) vsgViewer;
@end


@class vsg_iOS_View;

namespace vsgiOS
{
    extern vsg::Names getInstanceExtensions();

    /// KeyboardMap maps iOS keyboard events to vsg::KeySymbol
    class KeyboardMap : public vsg::Object
    {
    public:
        KeyboardMap();

        using kVKKeyCodeToKeySymbolMap = std::map<unsigned short, vsg::KeySymbol>;

        bool getKeySymbol(UIEvent* anEvent, vsg::KeySymbol& keySymbol, vsg::KeySymbol& modifiedKeySymbol, vsg::KeyModifier& keyModifier);

    protected:
        kVKKeyCodeToKeySymbolMap _keycodeMap;
    };


    /// iOS_Window implements iOS specific window creation, event handling and vulkan Surface setup.
    class iOS_Window : public vsg::Inherit<vsg::Window, iOS_Window>
    {
    public:

        iOS_Window(vsg::ref_ptr<vsg::WindowTraits> traits);
        iOS_Window() = delete;
        iOS_Window(const iOS_Window&) = delete;
        iOS_Window operator = (const iOS_Window&) = delete;

        const char* instanceExtensionSurfaceName() const override { return VK_EXT_METAL_SURFACE_EXTENSION_NAME; }

        bool valid() const override { return _window; }

        bool pollEvents(vsg::UIEvents& events) override;

       // bool resized() const override;

        void resize() override;

        bool handleUIEvent(UIEvent* anEvent);

        // OS native objects
        vsg_iOS_Window* window() { return _window; };
      //  vsg_iOS_View* view() { return _view; };
        CAMetalLayer* layer() { return _metalLayer; };

        vsg::clock::time_point getEventTime(double eventTime)
        {
            long elapsedmilli = long(double(eventTime - _first_macos_timestamp) * 1000.0f);
            return _first_macos_time_point + std::chrono::milliseconds(elapsedmilli);
        }

        void queueEvent(vsg::UIEvent* anEvent) { _bufferedEvents.emplace_back(anEvent); }

    protected:
        virtual ~iOS_Window();

        void _initSurface() override;

        vsg_iOS_Window* _window;
        vsg_iOS_View* _view;
        CAMetalLayer* _metalLayer;

        double _first_macos_timestamp = 0;
        vsg::clock::time_point _first_macos_time_point;

        vsg::UIEvents _bufferedEvents;
        vsg::ref_ptr<KeyboardMap> _keyboard;
    };

} // namespace vsgMacOS

EVSG_type_name(vsgiOS::iOS_Window);

#endif /* vsg_iOS_Window_h */
