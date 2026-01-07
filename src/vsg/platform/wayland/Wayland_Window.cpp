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
#include <iostream>
namespace vsg
{
    // Provide the Window::create(...) implementation that automatically maps to a Wayland_Window
    ref_ptr<Window> Window::create(vsg::ref_ptr<WindowTraits> traits)
    {
        return vsgWayland::Wayland_Window::create(traits);
    }
} // namespace vsg

using namespace vsg;
using namespace vsgWayland;

void WaylandRegistryState::keymapEvent(void* data, wl_keyboard* /*wl_keyboard*/, uint32_t format, int32_t fd, uint32_t size)
{
    WaylandRegistryState *state = static_cast<WaylandRegistryState*>(data);
    if(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    {
        char *map_shm = (char*)mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
        if(map_shm != MAP_FAILED)
        {
            struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
                                  state->_xkb_context, map_shm,
                                   XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
            munmap(map_shm, size);
            close(fd);
            struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
            xkb_keymap_unref(state->_xkb_keymap);
            xkb_state_unref(state->_xkb_state);
            state->_xkb_keymap = xkb_keymap;
            state->_xkb_state = xkb_state;
        }
    }

}

void WaylandRegistryState::kbd_enter_event(void *data,
                            struct wl_keyboard* /*wl_keyboard*/,
                            uint32_t /*serial*/,
                            struct wl_surface *surface,
                            struct wl_array *keys)
{
    WaylandRegistryState *state = static_cast<WaylandRegistryState*>(data);
    Wayland_Window *window = static_cast<Wayland_Window*>(wl_surface_get_user_data(surface));
    state->currentWindow = window;
    uint32_t* key;
    for (key = static_cast<uint32_t*>((keys)->data);
         (const char *) key < ((const char *) (keys)->data + (keys)->size);
         (key)++) {
        char buf[128];
        xkb_keysym_t sym = xkb_state_key_get_one_sym(state->_xkb_state, *key + 8);
        xkb_keysym_get_name(sym, buf, sizeof(buf));
        xkb_state_key_get_utf8(state->_xkb_state,*key + 8, buf, sizeof(buf));
    }
}

void WaylandRegistryState::kbd_key_event(void *data,
                          struct wl_keyboard* /*wl_keyboard*/,
                          uint32_t /*serial*/,
                          uint32_t /*time*/,
                          uint32_t key,
                          uint32_t state)
{
    vsg::clock::time_point event_time = vsg::clock::now();
    WaylandRegistryState *waylandState = static_cast<WaylandRegistryState*>(data);
    uint32_t keycode = key + 8;
    xkb_keysym_t sym = xkb_state_key_get_one_sym(waylandState->_xkb_state, keycode);
    xkb_mod_mask_t mod = xkb_state_key_get_consumed_mods(waylandState->_xkb_state,keycode);

    if(state == 0)
    {
        waylandState->currentWindow->bufferedEvents.emplace_back(vsg::KeyReleaseEvent::create(waylandState->currentWindow, event_time, 
                                                                                              vsg::KeySymbol(sym), 
                                                                                              vsg::KeySymbol(mod), 
                                                                                              vsg::KeyModifier(mod),0));

    }
    else
    {
        waylandState->currentWindow->bufferedEvents.emplace_back(vsg::KeyPressEvent::create(waylandState->currentWindow, event_time, 
                                                                                            vsg::KeySymbol(sym), 
                                                                                            vsg::KeySymbol(mod), 
                                                                                            vsg::KeyModifier(mod),0));
    }
}

void WaylandRegistryState::kbd_modifier_event(void *data,
                               struct wl_keyboard* /*wl_keyboard*/,
                               uint32_t /*serial*/,
                               uint32_t mods_depressed,
                               uint32_t mods_latched,
                               uint32_t mods_locked,
                               uint32_t group)
{
    WaylandRegistryState *state = static_cast<WaylandRegistryState*>(data);
    xkb_state_update_mask(state->_xkb_state,mods_depressed, mods_latched, mods_locked, 0, 0, group);
}


void Wayland_Window::xdg_surface_handle_configure(void* /*data*/,
        struct xdg_surface *xdg_surface, uint32_t serial) {
    xdg_surface_ack_configure(xdg_surface, serial);
}


void Wayland_Window::xdg_toplevel_handle_configure(void *data, struct xdg_toplevel* /*xdg_toplevel*/, int32_t width, int32_t height, struct wl_array* /*states*/) {
    if (width==0 || height==0)
        return;
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    vsg::clock::time_point event_time = vsg::clock::now();
    if(width != window->_width || height != window->_height)
    {
        window->_width = width;
        window->_height = height;
        window->_resize = true;
        window->resize();
        window->bufferedEvents.emplace_back(vsg::ConfigureWindowEvent::create(window,event_time, 0, 0, width, height));
    }
}

void Wayland_Window::xdg_toplevel_handle_close(void *data, struct xdg_toplevel* /*xdg_toplevel*/) {
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    vsg::clock::time_point event_time = vsg::clock::now();
    window->_state->currentWindow = nullptr;
    window->bufferedEvents.emplace_back(vsg::CloseWindowEvent::create(window, event_time));
}

void Wayland_Window::xdg_wm_base_ping(void* /*data*/, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}


void WaylandRegistryState::pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t /*surface_x*/, wl_fixed_t /*surface_y*/) {
    WaylandRegistryState *state = static_cast<WaylandRegistryState*>(data);
    Wayland_Window *window = static_cast<Wayland_Window*>(wl_surface_get_user_data(surface));
    window->_currentSurface = surface;
    state->currentWindow = window;
    std::string cursor = "left_ptr";

    const auto image = wl_cursor_theme_get_cursor(state->_cursorTheme, cursor.c_str())->images[0];
    wl_pointer_set_cursor(pointer, serial, state->_cursorSurface, image->hotspot_x, image->hotspot_y);
    wl_surface_attach(state->_cursorSurface, wl_cursor_image_get_buffer(image), 0, 0);
    wl_surface_damage(state->_cursorSurface, 0, 0, image->width, image->height);
    wl_surface_commit(state->_cursorSurface);
    vsg::clock::time_point event_time = vsg::clock::now();
    state->currentWindow->bufferedEvents.emplace_back(vsg::FocusInEvent::create(window, event_time));
}

void WaylandRegistryState::pointer_motion(void *data, struct wl_pointer* /*pointer*/, uint32_t /*time*/, wl_fixed_t x, wl_fixed_t y) {
    vsg::clock::time_point event_time = vsg::clock::now();
    WaylandRegistryState *state = static_cast<WaylandRegistryState*>(data);
    state->currentWindow->cursor_x = wl_fixed_to_int(x);
    state->currentWindow->cursor_y = wl_fixed_to_int(y);

    state->currentWindow->bufferedEvents.emplace_back(vsg::MoveEvent::create(state->currentWindow, event_time, 
                                                                             state->currentWindow->cursor_x, 
                                                                             state->currentWindow->cursor_y, 
                                                                             vsg::ButtonMask(state->maskButtons)));
}

void WaylandRegistryState::pointer_button(void *data, struct wl_pointer* /*pointer*/, uint32_t /*serial*/, uint32_t /*time*/, uint32_t button, uint32_t state) {

    WaylandRegistryState *waylandState = static_cast<WaylandRegistryState*>(data);
    vsg::clock::time_point event_time = vsg::clock::now();
    uint32_t in_button = 0;
    switch(button)
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

    if(state == 0)
    {
        uint16_t releasedButtonMask = BUTTON_MASK_1 << (in_button-1);
        waylandState->maskButtons = waylandState->maskButtons & ~releasedButtonMask;
        waylandState->currentWindow->bufferedEvents.emplace_back(vsg::ButtonReleaseEvent::create(waylandState->currentWindow, event_time, 
                                                                                                 waylandState->currentWindow->cursor_x, 
                                                                                                 waylandState->currentWindow->cursor_y, 
                                                                                                 vsg::ButtonMask(waylandState->maskButtons), in_button));
    }
    else
    {
        uint16_t pressedButtonMask = BUTTON_MASK_1 << (in_button-1);
        waylandState->maskButtons = waylandState->maskButtons | pressedButtonMask;
        waylandState->currentWindow->bufferedEvents.emplace_back(vsg::ButtonPressEvent::create(waylandState->currentWindow, event_time, waylandState->currentWindow->cursor_x, waylandState->currentWindow->cursor_y, vsg::ButtonMask(waylandState->maskButtons), in_button));
    }

}

void WaylandRegistryState::pointer_axis(void *data, struct wl_pointer* /*pointer*/, uint32_t /*time*/, uint32_t /*axis*/, wl_fixed_t value) {
    WaylandRegistryState *state = static_cast<WaylandRegistryState*>(data);
    vsg::clock::time_point event_time = vsg::clock::now();
    if(value > 0)
    {
        state->currentWindow->bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(state->currentWindow, event_time, vsg::vec3(0.0f, 1.0f, 0.0f)));
    }
    else if(value < 0)
    {
        state->currentWindow->bufferedEvents.emplace_back(vsg::ScrollWheelEvent::create(state->currentWindow, event_time, vsg::vec3(0.0f, -1.0f, 0.0f)));
    }
}

void WaylandRegistryState::seat_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities) {
    WaylandRegistryState *state = static_cast<WaylandRegistryState*>(data);
    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        struct wl_pointer *pointer = wl_seat_get_pointer (seat);
        wl_pointer_add_listener (pointer, &pointer_listener, data);
        state->_cursorSurface = wl_compositor_create_surface(state->_wlCompositor);
    }
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        struct wl_keyboard *keyboard = wl_seat_get_keyboard (seat);
        wl_keyboard_add_listener(keyboard, &wl_keyboard_listener, data);
    }
}

void WaylandRegistryState::registry_add_object(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version) {
    WaylandRegistryState *state = static_cast<WaylandRegistryState*>(data);
    
    if (strcmp(interface,"wl_compositor") == 0) {
        state->_wlCompositor = static_cast<wl_compositor*>(wl_registry_bind (registry, id, &wl_compositor_interface, 1));
    }
    else if (strcmp(interface,"wl_seat") == 0) {
        state->_seat = static_cast<wl_seat*>(wl_registry_bind (registry, id, &wl_seat_interface, 1));
        wl_seat_add_listener (state->_seat, &seat_listener, data);
    }
    else if (strcmp(interface, "wl_shm") == 0) {
        state->_shm = static_cast<wl_shm*>(wl_registry_bind(registry, id, &wl_shm_interface, version));
        state->_cursorTheme = wl_cursor_theme_load(nullptr, 16, state->_shm);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        state->_xdgWmBase = static_cast<struct xdg_wm_base*>(wl_registry_bind(registry, id, &xdg_wm_base_interface, version));
    }
    else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
    {
        state->_decorationManager = static_cast<zxdg_decoration_manager_v1 *>(wl_registry_bind(registry, id, &zxdg_decoration_manager_v1_interface, 1));
    }
}

void Wayland_Window::shell_surface_ping (void* /*data*/, struct wl_shell_surface *shell_surface, uint32_t serial) {
    wl_shell_surface_pong (shell_surface, serial);
}

void Wayland_Window::shell_surface_configure(void *data, struct wl_shell_surface* /*shell_surface*/, uint32_t /*edges*/, int32_t width, int32_t height) {
    Wayland_Window *window = static_cast<Wayland_Window*>(data);
    vsg::clock::time_point event_time = vsg::clock::now();
    if(width != window->_width || height != window->_height)
    {
        window->_width = width;
        window->_height = height;
        window->_resize = true;
        window->resize();
        window->bufferedEvents.emplace_back(vsg::ConfigureWindowEvent::create(window,event_time, 0, 0, width, height));
    }
}

void Wayland_Window::shell_surface_popup_done (void* /*data*/, struct wl_shell_surface* /*shell_surface*/) {
}

Wayland_surface::Wayland_surface(Instance* instance, wl_display* wlDisplay, wl_surface* wlSurface):
    Surface(VK_NULL_HANDLE, instance)
{
    
    VkWaylandSurfaceCreateInfoKHR createInfo = {};
     createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
     createInfo.display = wlDisplay;
     createInfo.surface = wlSurface;

     VkResult result = vkCreateWaylandSurfaceKHR(instance->vk(), &createInfo, NULL, &_surface);

    if (result != VK_SUCCESS)
    {
        throw Exception{"Failed to created Wayland suface.", result};
    }
}

WaylandRegistryState* WaylandRegistryState::initOrSetState(Wayland_Window* window){

    if(!initialized)
    {
        state = new WaylandRegistryState;
        

        state->_wlDisplay = wl_display_connect(NULL);
        if(!state->_wlDisplay)
        {
            throw Exception{"failed to create Wayland connection"};
        }
        state->_wlRegistry = wl_display_get_registry(state->_wlDisplay);
        state->_xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

        wl_registry_add_listener(state->_wlRegistry,&registry_listener,state);
        ref_ptr<Wayland_Window> windowPtr = ref_ptr<Wayland_Window>(window);
        state->addChild(windowPtr);
        state->maskButtons = BUTTON_MASK_OFF;
        initialized = true;
        return state;
    }
    else
    {
        ref_ptr<Wayland_Window> windowPtr = ref_ptr<Wayland_Window>(window);
        state->addChild(windowPtr);
        return state;
    }
}

WaylandRegistryState::~WaylandRegistryState(){
    if(_xdgWmBase)
    {
        zxdg_decoration_manager_v1_destroy(_decorationManager);
        xdg_wm_base_destroy(_xdgWmBase);
        wl_cursor_theme_destroy(_cursorTheme);
    }
    xkb_state_unref(_xkb_state);
    xkb_keymap_unref(_xkb_keymap);
    xkb_context_unref(_xkb_context);
    wl_compositor_destroy(_wlCompositor);
    wl_registry_destroy(_wlRegistry);
    wl_display_disconnect(_wlDisplay);
    
    delete state;
}

Wayland_Window::Wayland_Window(vsg::ref_ptr<WindowTraits> traits) :
    Inherit(traits)
{
    _state = WaylandRegistryState::initOrSetState(this);
    
    wl_display_roundtrip(_state->_wlDisplay);

    _width = traits->width;
    _height = traits->height;

    _extent2D.width = _width;
    _extent2D.height = _height;
    _windowTitle = traits->windowTitle;
}

Wayland_Window::~Wayland_Window()
{
    clear();
    if(_state->_xdgWmBase) {
        zxdg_toplevel_decoration_v1_destroy(_topDecoration);
        xdg_toplevel_destroy(_xdgToplevel);
        xdg_surface_destroy(_xdgSurface);
        wl_surface_destroy(_wlSurface);

    }

}

void Wayland_Window::_initSurface()
{
    if (!_instance) _initInstance();

    _wlSurface = wl_compositor_create_surface(_state->_wlCompositor);
    wl_surface_set_user_data(_wlSurface, this);
    if(_state->_xdgWmBase) {
        _xdgSurface = xdg_wm_base_get_xdg_surface(_state->_xdgWmBase, _wlSurface);
        _xdgToplevel = xdg_surface_get_toplevel(_xdgSurface);
        _topDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(_state->_decorationManager, _xdgToplevel);
        zxdg_toplevel_decoration_v1_set_mode(_topDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
        xdg_surface_add_listener(_xdgSurface, &xdg_surface_listener, this);
        xdg_toplevel_add_listener(_xdgToplevel, &xdg_toplevel_listener, this);
        xdg_wm_base_add_listener(_state->_xdgWmBase, &xdg_wm_base_listener, this);
        xdg_toplevel_set_title(_xdgToplevel, _windowTitle.c_str());
        xdg_toplevel_set_app_id(_xdgToplevel, _windowTitle.c_str());
    }
    else
    {
        throw Exception{"failed to create Wayland window"};
    }

    wl_display_roundtrip(_state->_wlDisplay);
    if(!_surface)
        _surface = new Wayland_surface(_instance,_state->_wlDisplay,_wlSurface);
    wl_display_roundtrip(_state->_wlDisplay);
}

bool Wayland_Window::valid() const
{
    return (_state->_wlDisplay != nullptr);
}

bool Wayland_Window::visible() const
{
    return (_state->_wlCompositor != nullptr);
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
    wl_display_dispatch_pending(_state->_wlDisplay);
    return Window::pollEvents(events);
}
