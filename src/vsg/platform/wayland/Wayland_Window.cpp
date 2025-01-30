#include <vsg/core/Exception.h>
#include <vsg/io/Logger.h>

#include <vsg/platform/wayland/Wayland_Window.h>

#include <vsg/ui/ApplicationEvent.h>

#include <vsg/ui/ScrollWheelEvent.h>

#include <chrono>
#include <cstring>
#include <thread>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-util.h>

namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to a Wayland_Window
    ref_ptr<Window> Window::create(vsg::ref_ptr<WindowTraits> traits)
    {
        return vsgWayland::Wayland_Window::create(traits);
    }
} // namespace vsg

// listeners

using namespace vsg;
using namespace vsgWayland;

void Wayland_Window::keymapEvent(void* data, wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size)
{
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    if (format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        char *map_shm = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
        if(map_shm != MAP_FAILED)
        {
            struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
                window->_xkb_context, map_shm,
                XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
            munmap(map_shm, size);
            close(fd);
            struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
            xkb_keymap_unref(window->_xkb_keymap);
            xkb_state_unref(window->_xkb_state);
            window->_xkb_keymap = xkb_keymap;
            window->_xkb_state = xkb_state;
        }
    }

}

void Wayland_Window::kbd_enter_event(void *data,
                            struct wl_keyboard *wl_keyboard,
                            uint32_t serial,
                            struct wl_surface *surface,
                            struct wl_array *keys)
{
//std::cout << "kbd enter " <<std::endl;
   // Wayland_Window *window = static_cast<Wayland_Window*>(data);
    /*fprintf(stderr, "keyboard enter; keys pressed are:\n");
    uint32_t* key;
    for (key = static_cast<uint32_t*>((keys)->data);
         (const char *) key < ((const char *) (keys)->data + (keys)->size);
         (key)++) {
        char buf[128];
        xkb_keysym_t sym = xkb_state_key_get_one_sym(
                        window->_xkb_state, *key + 8);
        xkb_keysym_get_name(sym, buf, sizeof(buf));
        fprintf(stderr, "sym: %-12s (%d), ", buf, sym);
        xkb_state_key_get_utf8(window->_xkb_state,
                        *key + 8, buf, sizeof(buf));
        fprintf(stderr, "utf8: '%s'\n", buf);
    }*/
}

void Wayland_Window::kbd_leave_event(void *data,
                            struct wl_keyboard *wl_keyboard,
                            uint32_t serial,
                            struct wl_surface *surface)
{
//std::cout << "kbd leave " << std::endl;
}

void Wayland_Window::kbd_key_event(void *data,
                          struct wl_keyboard *wl_keyboard,
                          uint32_t serial,
                          uint32_t time,
                          uint32_t key,
                          uint32_t state)
{
    vsg::clock::time_point event_time = vsg::clock::now();
    Wayland_Window *window = static_cast<Wayland_Window*>(data);

    uint32_t keycode = key + 8;
    xkb_keysym_t sym = xkb_state_key_get_one_sym(window->_xkb_state, keycode);
    xkb_mod_mask_t mod = xkb_state_key_get_consumed_mods(window->_xkb_state,keycode);

    if (state == 0)
    {
        window->bufferedEvents.emplace_back(vsg::KeyReleaseEvent::create(window, event_time, vsg::KeySymbol(sym), vsg::KeySymbol(mod), vsg::KeyModifier(mod),0));
    }
    else
    {
        window->bufferedEvents.emplace_back(vsg::KeyPressEvent::create(window, event_time, vsg::KeySymbol(sym), vsg::KeySymbol(mod), vsg::KeyModifier(mod),0));
    }
}

void Wayland_Window::kbd_modifier_event(void *data,
                               struct wl_keyboard *wl_keyboard,
                               uint32_t serial,
                               uint32_t mods_depressed,
                               uint32_t mods_latched,
                               uint32_t mods_locked,
                               uint32_t group)
{
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    xkb_state_update_mask(window->_xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}

void Wayland_Window::xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    xdg_surface_ack_configure(xdg_surface, serial);

    if ((window->_widthRequested && (window->_widthRequested != window->_width)) || (window->_heightRequested && (window->_heightRequested != window->_height)))
    {
        window->_width = window->_widthRequested;
        window->_height = window->_heightRequested;
        window->_resize = true;
        window->resize();

        window->_widthRequested = window->_heightRequested = 0;
    }

    wl_surface_commit(window->_wlSurface);
}

void Wayland_Window::xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    if ((width != 0) && (height != 0)) {
        window->_widthRequested = width;
        window->_heightRequested = height;
    }
}

void Wayland_Window::xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel) {
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    vsg::clock::time_point event_time = vsg::clock::now();
    window->bufferedEvents.emplace_back(vsg::CloseWindowEvent::create(window, event_time));
}

void Wayland_Window::xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

void Wayland_Window::pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    window->_currentSurface = surface;

    std::string cursor = "left_ptr";

    const auto image = wl_cursor_theme_get_cursor(window->_cursorTheme, cursor.c_str())->images[0];
    wl_pointer_set_cursor(pointer, serial, window->_cursorSurface, image->hotspot_x, image->hotspot_y);
    wl_surface_attach(window->_cursorSurface, wl_cursor_image_get_buffer(image), 0, 0);
    wl_surface_damage(window->_cursorSurface, 0, 0, image->width, image->height);
    wl_surface_commit(window->_cursorSurface);
}

void Wayland_Window::pointer_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {

}

void Wayland_Window::pointer_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
    vsg::clock::time_point event_time = vsg::clock::now();
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    window->cursor_x = wl_fixed_to_int(x);
    window->cursor_y = wl_fixed_to_int(y);

    window->bufferedEvents.emplace_back(vsg::MoveEvent::create(window, event_time, window->cursor_x, window->cursor_y, vsg::ButtonMask(window->maskButtons)));
}

void Wayland_Window::pointer_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    vsg::clock::time_point event_time = vsg::clock::now();
    uint32_t in_button = 0;
    switch (button)
    {
    case 272:
    {
        in_button = 1;
        break;
    }
    case 273:
    {
        in_button = 2;
        break;
    }
    case 274:
    {
        in_button = 3;
        break;
    }
    }

    if (state == 0)
    {
        uint16_t releasedButtonMask = BUTTON_MASK_1 << (in_button-1);
        window->maskButtons = window->maskButtons & ~releasedButtonMask;
        window->bufferedEvents.emplace_back(vsg::ButtonReleaseEvent::create(window, event_time, window->cursor_x, window->cursor_y, vsg::ButtonMask(window->maskButtons), in_button));
    }
    else
    {
        uint16_t pressedButtonMask = BUTTON_MASK_1 << (in_button-1);
        window->maskButtons = window->maskButtons | pressedButtonMask;
        window->bufferedEvents.emplace_back(vsg::ButtonPressEvent::create(window, event_time, window->cursor_x, window->cursor_y, vsg::ButtonMask(window->maskButtons), in_button));
    }

}

void Wayland_Window::pointer_axis(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    vsg::clock::time_point event_time = vsg::clock::now();
    if (value > 0)
    {
        window->bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(window, event_time, vsg::vec3(0.0f, 1.0f, 0.0f)));
    }
    else if (value < 0)
    {
        window->bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(window, event_time, vsg::vec3(0.0f, -1.0f, 0.0f)));
    }
}

void Wayland_Window::seat_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities) {
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        struct wl_pointer *pointer = wl_seat_get_pointer (seat);
        wl_pointer_add_listener (pointer, &pointer_listener, data);
        window->_cursorSurface = wl_compositor_create_surface(window->_wlCompositor);
    }
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        struct wl_keyboard *keyboard = wl_seat_get_keyboard (seat);
        wl_keyboard_add_listener(keyboard, &wl_keyboard_listener, data);
    }
}

void Wayland_Window::registry_add_object(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version) {
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        window->_wlCompositor = static_cast<wl_compositor*>(wl_registry_bind (registry, id, &wl_compositor_interface, 1));
    }
    /*else if (strcmp(interface, "wl_subcompositor") == 0) {
        subcompositor = static_cast<wl_subcompositor*>(wl_registry_bind(registry, name, &wl_subcompositor_interface, 1));
    }*/
    else if (strcmp(interface, wl_seat_interface.name) == 0) {
        window->_seat = static_cast<wl_seat*>(wl_registry_bind (registry, id, &wl_seat_interface, 1));
        wl_seat_add_listener (window->_seat, &seat_listener, data);
    }
    else if (strcmp(interface, wl_shm_interface.name) == 0) {
        window->_shm = static_cast<wl_shm*>(wl_registry_bind(registry, id, &wl_shm_interface, version));
        window->_cursorTheme = wl_cursor_theme_load(nullptr, 16, window->_shm);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        window->_xdgWmBase = static_cast<struct xdg_wm_base*>(wl_registry_bind(registry, id, &xdg_wm_base_interface, version));
    }
    else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
    {
        window->_decorationManager = static_cast<zxdg_decoration_manager_v1 *>(wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1));
    }
    else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0)
    {
        window->_zxdgOutputManager = static_cast<zxdg_output_manager_v1 *>(wl_registry_bind(registry, id, &zxdg_output_manager_v1_interface, version));
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
        window->_outputs.emplace_back(new Wayland_output(static_cast<struct wl_output *>(wl_registry_bind(registry, id, &wl_output_interface, version)), window->_zxdgOutputManager));
    }
}

void Wayland_Window::registry_remove_object(void *data, struct wl_registry *registry, uint32_t id) {

}

Wayland_surface::Wayland_surface(Instance* instance, wl_display* wlDisplay, wl_surface* wlSurface):
    Surface(VK_NULL_HANDLE, instance)
{
    VkWaylandSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.display = wlDisplay;
    createInfo.surface = wlSurface;

    auto result = vkCreateWaylandSurfaceKHR(*instance, &createInfo, instance->getAllocationCallbacks(), &_surface);
    if (result != VK_SUCCESS)
    {
        throw Exception{"Failed to created Wayland suface.", result};
    }
}

Wayland_Window::Wayland_Window(vsg::ref_ptr<WindowTraits> traits) :
    Inherit(traits)
{
    maskButtons = BUTTON_MASK_OFF;

    if (traits->systemConnection.has_value())
    {
        auto nativeConnection = std::any_cast<struct wl_display*>(traits->systemConnection);
        if (nativeConnection)
        {
            _wlDisplay = nativeConnection;
            _ownDisplay = false;
        }
    }

    if (_ownDisplay)
    {
        _wlDisplay = wl_display_connect(NULL);

        if(!_wlDisplay)
        {
            throw Exception{"failed to create Wayland connection"};
        }
    }

    _wlRegistry = wl_display_get_registry(_wlDisplay);
    _xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    wl_registry_add_listener(_wlRegistry, &registry_listener, this);
    wl_display_roundtrip(_wlDisplay);

    while (!std::all_of(_outputs.begin(), _outputs.end(), [](const auto output) { return output->_done; }))
    {
        wl_display_roundtrip(_wlDisplay);
    }

    if (traits->width)
    {
        _extent2D.width = _width = traits->width;
    }
    if (traits->height)
    {
        _extent2D.height = _height = traits->height;
    }

    if (traits->nativeWindow.has_value())
    {
        auto nativeWindow = std::any_cast<struct wl_surface *>(traits->nativeWindow);
        if (nativeWindow)
        {
            _wlSurface = nativeWindow;
            _ownSurface = false;
        }
    }

    if (_ownSurface)
    {
        _wlSurface = wl_compositor_create_surface(_wlCompositor);

        if (_xdgWmBase)
        {
            _xdgSurface = xdg_wm_base_get_xdg_surface(_xdgWmBase, _wlSurface);
            _xdgToplevel = xdg_surface_get_toplevel(_xdgSurface);
            _topDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(_decorationManager, _xdgToplevel);
            zxdg_toplevel_decoration_v1_set_mode(_topDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
            xdg_surface_add_listener(_xdgSurface, &xdg_surface_listener, this);
            xdg_toplevel_add_listener(_xdgToplevel, &xdg_toplevel_listener, this);
            xdg_wm_base_add_listener(_xdgWmBase, &xdg_wm_base_listener, this);
            xdg_toplevel_set_title(_xdgToplevel, traits->windowTitle.c_str());
            xdg_toplevel_set_app_id(_xdgToplevel, traits->windowTitle.c_str());
            if (_traits->fullscreen)
            {
                struct wl_output *wlOutput = nullptr;
                if (!traits->display.empty())
                {
                    const auto it = std::find_if(_outputs.begin(), _outputs.end(), [traits](const auto output) { return output->_name == traits->display; });
                    if (it != _outputs.end())
                    {
                        wlOutput = it->get()->_wlOutput;
                    }
                }
                xdg_toplevel_set_fullscreen(_xdgToplevel, wlOutput);
            }
            wl_surface_commit(_wlSurface);
        }
        else
        {
            throw Exception{"failed to create Wayland window"};
        }
    }

    if (traits->device)
    {
        share(traits->device);
    }
}

Wayland_Window::~Wayland_Window()
{
    clear();

    if (_ownSurface)
    {
        if (_xdgWmBase)
        {
            zxdg_toplevel_decoration_v1_destroy(_topDecoration);
            zxdg_decoration_manager_v1_destroy(_decorationManager);
            xdg_toplevel_destroy(_xdgToplevel);
            xdg_surface_destroy(_xdgSurface);
            wl_surface_destroy(_wlSurface);
            xdg_wm_base_destroy(_xdgWmBase);
            wl_cursor_theme_destroy(_cursorTheme);
        }
    }
    xkb_state_unref(_xkb_state);
    xkb_keymap_unref(_xkb_keymap);
    xkb_context_unref(_xkb_context);
    _outputs.clear();
    zxdg_output_manager_v1_destroy(_zxdgOutputManager);
    wl_compositor_destroy(_wlCompositor);
    wl_registry_destroy(_wlRegistry);
    if (_ownDisplay) wl_display_disconnect(_wlDisplay);
}

void Wayland_Window::_initSurface()
{
    if (!_instance) _initInstance();

    _surface = new Wayland_surface(_instance, _wlDisplay, _wlSurface);
}

bool Wayland_Window::valid() const
{
    return (_wlDisplay != nullptr);
}

bool Wayland_Window::visible() const
{
    return (_wlCompositor != nullptr);
}

void Wayland_Window::releaseWindow()
{
}

void Wayland_Window::releaseConnection()
{
}

void Wayland_Window::resize()
{
    if(_resize)
    {
        _extent2D.width = _width;
        _extent2D.height = _height;

        buildSwapchain();
        _resize = false;
    }
}

bool Wayland_Window::pollEvents(UIEvents& events)
{
    wl_display_dispatch_pending(_wlDisplay);

    return Window::pollEvents(events);
}
