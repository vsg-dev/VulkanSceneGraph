#pragma once

#include <vsg/app/Window.h>
#include <vsg/ui/KeyEvent.h>

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include <xkbcommon/xkbcommon.h>

#include <vector>
#include <vsg/core/Objects.h>
#include "wayland-xdg-shell-client-protocol.h"
#include "xdg-decoration-client.h"
#include <vsg/ui/PointerEvent.h>
#include <vulkan/vulkan_wayland.h>

namespace vsgWayland
{
    class Wayland_Window;

    /*
     * This is similar to xcb surface, but we store the pointer to window 
     * which will be used by wayland api to access window object
     */
    class Wayland_surface : public vsg::Surface
    {
    public:
        Wayland_surface(vsg::Instance* instance, wl_display* wlDisplay, wl_surface* wlSurface);

    protected:
        Wayland_Window* windowData;
    };

    /*
     * This is the object which holds wayland session infomration which is shared across all the windows
     * It uses global static object which will be referred by all the windows
     * Here we reuse vsg::Objects which will add the Wayland_Windows under it
     * the currentWindow member will track the actie window
     */
    class WaylandRegistryState : public vsg::Inherit<vsg::Objects, WaylandRegistryState>
    {
        friend class Wayland_Window;

    public:
        WaylandRegistryState(const WaylandRegistryState&) = delete;
        WaylandRegistryState& operator=(const WaylandRegistryState&) = delete;
        static WaylandRegistryState* initOrSetState(Wayland_Window* window); // hook for initializing it or adding a new window to it 
        ~WaylandRegistryState();

        /*
         * The below declerations are the listeners used by wayland, for some I used empty decleration
         * to avoid warnings, these can be implemented as per future need
         */
        static void keymapEvent(void* data, wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size);
        static void kbd_enter_event(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface, struct wl_array* keys);
        static void kbd_leave_event(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* surface){};
        static void kbd_key_event(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
        static void kbd_modifier_event(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
        static void kbd_repeat_event(void* data, wl_keyboard* wl_keyboard, int rate, int delay){};
        constexpr static struct wl_keyboard_listener wl_keyboard_listener = {
            .keymap = keymapEvent,
            .enter = kbd_enter_event,
            .leave = kbd_leave_event,
            .key = kbd_key_event,
            .modifiers = kbd_modifier_event,
            .repeat_info = kbd_repeat_event};

        static void pointer_enter(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
        static void pointer_leave(void* data, struct wl_pointer* pointer, uint32_t serial, struct wl_surface* surface){};
        static void pointer_motion(void* data, struct wl_pointer* pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y);
        static void pointer_button(void* data, struct wl_pointer* pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
        static void pointer_axis(void* data, struct wl_pointer* pointer, uint32_t time, uint32_t axis, wl_fixed_t value);
        static void pointer_frame(void* data, wl_pointer* wl_pointer){};
        static void pointer_axis_source(void* data, wl_pointer* wl_pointer, uint axis_source){};
        static void pointer_axis_stop(void* data, wl_pointer* wl_pointer, uint time, uint axis){};
        static void pointer_axis_discrete(void* data, wl_pointer* wl_pointer, uint axis, int discrete){};
        constexpr static struct wl_pointer_listener pointer_listener = {
            .enter = pointer_enter,
            .leave = pointer_leave,
            .motion = pointer_motion,
            .button = pointer_button,
            .axis = pointer_axis,
            .frame = pointer_frame,
            .axis_source = pointer_axis_source,
            .axis_stop = pointer_axis_stop,
            .axis_discrete = pointer_axis_discrete};

        static void seat_capabilities(void* data, struct wl_seat* seat, uint32_t capabilities);
        static void seat_name(void* data, wl_seat* wl_seat, const char* name){};
        constexpr static struct wl_seat_listener seat_listener = {
            .capabilities = seat_capabilities,
            .name = seat_name};

        static void registry_add_object(void* data, struct wl_registry* registry, uint32_t id, const char* interface, uint32_t version);
        static void registry_remove_object(void* data, struct wl_registry* registry, uint32_t id){};
        constexpr static struct wl_registry_listener registry_listener = {
            .global = registry_add_object,
            .global_remove = registry_remove_object};

    protected:
        struct wl_display* _wlDisplay = nullptr;
        struct wl_registry* _wlRegistry = nullptr;
        struct wl_compositor* _wlCompositor = nullptr;
        struct wl_seat* _seat = nullptr;
        struct wl_shm* _shm = nullptr;
        struct xdg_wm_base* _xdgWmBase = nullptr;
        struct zxdg_decoration_manager_v1* _decorationManager = nullptr;
        struct wl_cursor_theme* _cursorTheme = nullptr;
        struct xkb_state* _xkb_state;
        struct xkb_context* _xkb_context;
        struct xkb_keymap* _xkb_keymap;
        struct wl_surface* _cursorSurface = nullptr;
        uint16_t maskButtons;
        Wayland_Window* currentWindow;
        static inline WaylandRegistryState* state;
        static inline bool initialized;
        
        WaylandRegistryState(){}; //empty initializer needed for initializing from the initOrSetState
    };

    /*
     * The Wayland_Window class can access members of WaylandStateRegistry class and vice versa
     * 
     */
    class Wayland_Window : public vsg::Inherit<vsg::Window, Wayland_Window>
    {
        friend class WaylandRegistryState;

    public:
        Wayland_Window(vsg::ref_ptr<vsg::WindowTraits> traits);
        Wayland_Window() = delete;
        Wayland_Window(const Wayland_Window&) = delete;
        Wayland_Window& operator=(const Wayland_Window&) = delete;

        const char* instanceExtensionSurfaceName() const override { return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME; }

        bool valid() const override;

        bool visible() const override;

        void releaseWindow() override;
        void releaseConnection() override;

        bool pollEvents(vsg::UIEvents& events) override;

        void resize() override;

    protected:
        ~Wayland_Window();
        static WaylandRegistryState registryStateRef;
        void _initSurface() override;

        /*
         * The below declerations are the listeners used by wayland, for some I used empty decleration
         * to avoid warnings, these can be implemented as per future need
         */
        static void xdg_surface_handle_configure(void* data, struct xdg_surface* xdg_surface, uint32_t serial);
        constexpr static struct xdg_surface_listener xdg_surface_listener = {
            .configure = xdg_surface_handle_configure,
        };

        static void xdg_toplevel_handle_configure(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, struct wl_array* states);
        static void xdg_toplevel_handle_close(void* data, struct xdg_toplevel* xdg_toplevel);
        static void xdg_toplevel_handle_configure_bounds(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height){};
        constexpr static struct xdg_toplevel_listener xdg_toplevel_listener = {
            .configure = xdg_toplevel_handle_configure,
            .close = xdg_toplevel_handle_close,
            .configure_bounds = xdg_toplevel_handle_configure_bounds};

        static void shell_surface_ping(void* data, struct wl_shell_surface* shell_surface, uint32_t serial);
        static void shell_surface_configure(void* data, struct wl_shell_surface* shell_surface, uint32_t edges, int32_t width, int32_t height);
        static void shell_surface_popup_done(void* data, struct wl_shell_surface* shell_surface);
        constexpr static struct wl_shell_surface_listener shell_surface_listener = {
            .ping = shell_surface_ping,
            .configure = shell_surface_configure,
            .popup_done = shell_surface_popup_done};
        static void xdg_wm_base_ping(void* data, struct xdg_wm_base* xdg_wm_base, uint32_t serial);
        constexpr static struct xdg_wm_base_listener xdg_wm_base_listener = {
            .ping = xdg_wm_base_ping};

        struct wl_surface* _wlSurface = nullptr;
        struct xdg_surface* _xdgSurface = nullptr;
        struct xdg_toplevel* _xdgToplevel = nullptr;
        struct wl_surface* _currentSurface = nullptr;
        struct zxdg_toplevel_decoration_v1* _topDecoration = nullptr;
        WaylandRegistryState* _state;
        int _width = 256;
        int _height = 256;
        int cursor_x = -1;
        int cursor_y = -1;
        bool _resize = false;
        std::string _windowTitle;
    };
} // namespace vsgWayland

EVSG_type_name(vsgWayland::Wayland_Window);
