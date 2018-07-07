#include "wwindow.h"
#include "libload.h"

#include "pparse.h"

_persist LibHandle wwindowlib_handle = 0;
_persist u32 loaded_lib_type = 0;
_persist s8 wtext_buffer[256] ={};

//function implementations
_persist s8 (*impl_wkeycodetoascii)(u32) = 0;
_persist u32 (*impl_wwaitforevent)(WWindowContext*,WWindowEvent*) = 0;
_persist void (*impl_wsettitle)(WWindowContext*,const s8*) = 0;

#include "x11_wwindow.cpp"

_persist WWindowEvent wayland_event_array[32];
_persist u32 wayland_event_count = 0;

//TODO: We can probably get rid of some of the branching

//FIXME: for some reason swapchain creation is failing cos we don't link to wayland-client (SDL manages this so we will play around w this)

wl_proxy* (*wl_proxy_marshal_constructor_fptr)(wl_proxy*,u32,const wl_interface*,...) = 0;

s32 (*wl_proxy_add_listener_fptr)(wl_proxy*,void (**)(void), void*) = 0;

void (*wl_proxy_marshal_fptr)(wl_proxy*,u32,...) = 0;

void
(*wl_proxy_set_user_data_fptr)(wl_proxy*,void*) = 0; 

void* (*wl_proxy_get_user_data_fptr)(wl_proxy*) = 0;

u32 (*wl_proxy_get_version_fptr)(wl_proxy*) = 0;

void (*wl_proxy_destroy_fptr)(wl_proxy*) = 0;

wl_proxy* (*wl_proxy_marshal_constructor_versioned_fptr)(wl_proxy*,
                                                         u32,
                                                         const wl_interface*,u32,...) = 0;

const wl_interface* wl_display_interface_ptr = 0;
const wl_interface* wl_registry_interface_ptr = 0;
const wl_interface* wl_compositor_interface_ptr = 0;
const wl_interface* wl_seat_interface_ptr = 0;
const wl_interface* wl_shell_interface_ptr = 0;
const wl_interface* wl_pointer_interface_ptr = 0;
const wl_interface* wl_keyboard_interface_ptr = 0;
const wl_interface* wl_surface_interface_ptr = 0;
const wl_interface* wl_shell_surface_interface_ptr = 0;
const wl_interface* wl_callback_interface_ptr = 0;
const wl_interface* wl_region_interface_ptr = 0;

const wl_interface* wl_buffer_interface_ptr = 0;
const wl_interface* wl_shm_pool_interface_ptr = 0;
const wl_interface* wl_data_source_interface_ptr = 0;
const wl_interface* wl_data_device_interface_ptr = 0;
const wl_interface* wl_touch_interface_ptr = 0;
const wl_interface* wl_subsurface_interface_ptr = 0;




logic InternalLoadLibraryWayland(){
    
    //TODO: we should check if vulkan supports this first. return failure if  it doesn't
    
    if(wwindowlib_handle){
        
        if(loaded_lib_type != _WAYLAND_WINDOW){
            return false;
        }
        
        return true;
    }
    
    const s8* wayland_paths[] = 
    {
        "libwayland-client.so.0.3.0",
        "libwayland-client.so.0",
        "libwayland-client.so",
    };
    
    for(u32 i = 0; i < _arraycount(wayland_paths); i++){
        
        wwindowlib_handle = LLoadLibrary(wayland_paths[i]);
        
        if(wwindowlib_handle){
            break;  
        }
        
    }
    
    if(!wwindowlib_handle){
        return false;
    }
    
    loaded_lib_type = _WAYLAND_WINDOW;
    
    return true;
}



//wayland stuff

s8 WKeyCodeToASCIIWayland(u32 keycode){
    _kill("",1);
    return 0;
}

u32 WWaitForWindowEventWayland(WWindowContext* windowcontext,
                               WWindowEvent* event){
    _kill("",1);
    return 0;
}

void WSetTitleWayland(WWindowContext* context,const s8* title){
    
    _kill("",1);
    
    //    wl_proxy_marshal_fptr((wl_proxy*)wdata.shell_surface,WL_SHELL_SURFACE_SET_TITLE,title);
    
}


void WaylandKeyboardMap(void* data,wl_keyboard* keyboard,u32 format,s32 fd,u32 size){
    
}

void WaylandKeyboardEnter(void* data,wl_keyboard* keyboard,u32 serial,
                          wl_surface* surface,wl_array* keys){
    
}

void WaylandKeyboardLeave(void* data,wl_keyboard* keyboard,u32 serial,wl_surface* surface){}

void WaylandKeyboardKey(void* data,wl_keyboard* keyboard,u32 serial,u32 time,u32 key,u32 state){
    
    /*TODO: fill these w key press events*/
    
    auto event = &wayland_event_array[wayland_event_count];
    wayland_event_count++;
    
    
    
    if(state){
        event->type = W_EVENT_KBEVENT_KEYDOWN;
    }
    
    else{
        event->type = W_EVENT_KBEVENT_KEYUP;
    }
    
    event->keyboard_event.keycode = key;
}

void WaylandKeyboardModifiers(void* data,wl_keyboard* keyboard,u32 serial,u32 mods_depressed,u32 mods_latched,u32 mods_locked,u32 group){
    
}

void WaylandPointerEnter(void* data,wl_pointer* pointer,u32 serial, wl_surface* surface,
                         wl_fixed_t sx, wl_fixed_t sy){
    
}

void WaylandPointerLeave(void* data,wl_pointer* pointer,u32 serial,wl_surface* surface){}

void WaylandPointerMotion(void* data,wl_pointer* pointer,u32 time, wl_fixed_t sx, wl_fixed_t sy){
    
    /*TODO: fill w motion events*/
    
    auto event = &wayland_event_array[wayland_event_count];
    wayland_event_count++;
    
    event->type = W_EVENT_MSEVENT_MOVE;
    event->mouse_event.x = sx;
    event->mouse_event.y = sy;
}

void WaylandPointerButton(void* data,wl_pointer* pointer,u32 serial,u32 time,u32 button,u32 state){
    
    /*TODO: fill w button events*/
    
    auto event = &wayland_event_array[wayland_event_count];
    wayland_event_count++;
    
    if(state){
        event->type = W_EVENT_MSEVENT_DOWN;
    }
    
    else{
        event->type = W_EVENT_MSEVENT_UP;
    }
    
    event->mouse_event.keycode = button;
}

void WaylandPointerAxis(void* data,wl_pointer* pointer,u32 time,u32 axis,wl_fixed_t value){
    
    /*TODO: fill w mouse scroll events*/
}

_persist wl_pointer_listener pointer_listener = {
    WaylandPointerEnter,
    WaylandPointerLeave,
    WaylandPointerMotion,
    WaylandPointerButton,
    WaylandPointerAxis
};

_persist wl_keyboard_listener keyboard_listener = {
    WaylandKeyboardMap,
    WaylandKeyboardEnter,
    WaylandKeyboardLeave,
    WaylandKeyboardKey,
    WaylandKeyboardModifiers
};


void SeatCapabilities(void* data,wl_seat* seat,u32 caps){
    
    auto w = (WaylandData*)data;
    
    if(caps & WL_SEAT_CAPABILITY_POINTER){
        
        w->pointer = wl_seat_get_pointer(seat);
        
        wl_pointer_add_listener(w->pointer,&pointer_listener,data);
        
    }
    
    if(caps & WL_SEAT_CAPABILITY_KEYBOARD){
        
        w->keyboard = wl_seat_get_keyboard(seat);
        
        wl_keyboard_add_listener(w->keyboard,&keyboard_listener,data);
        
    }
}

_persist const wl_seat_listener seat_listener = {
    SeatCapabilities
};

void Wayland_Display_Handle_Global(void* data, struct wl_registry* registry, u32 id,const s8* interface, u32 version){
    
    auto w = (WaylandData*)data;
    
    
    if(PHashString(interface) == PHashString("wl_compositor")){
        
        w->compositor = (wl_compositor*)wl_registry_bind(registry,id,&wl_compositor_interface,3);
    }
    
    if(PHashString(interface) == PHashString("wl_shell")){
        
        w->shell = (wl_shell*)wl_registry_bind(registry,id,&wl_shell_interface,1);
    }
    
    if(PHashString(interface) == PHashString("wl_seat")){
        
        w->seat = (wl_seat*)wl_registry_bind(registry,id,&wl_seat_interface,1);
        
        wl_seat_add_listener(w->seat,&seat_listener,data);
        
    }
    
}

_persist const wl_registry_listener registry_listener = {Wayland_Display_Handle_Global,0};

void Wayland_Ping(void* data,wl_shell_surface* shell_surface,u32 serial){
    
    wl_shell_surface_pong(shell_surface,serial);
}

void Wayland_Configure(void* data,wl_shell_surface* shell_surface,u32 edges,s32 width,s32  height){
    
}

void Wayland_Popupdone(void* data,wl_shell_surface* shell_surface){}


_persist const wl_shell_surface_listener shell_surface_listener =
{ Wayland_Ping, Wayland_Configure, Wayland_Popupdone };


void InternalLoadWaylandSymbols(){
    
    wl_proxy_marshal_constructor_fptr= (wl_proxy* (*)(wl_proxy*,u32,const wl_interface*,...))LGetLibFunction(wwindowlib_handle,"wl_proxy_marshal_constructor");
    
    wl_proxy_add_listener_fptr = (s32 (*)(wl_proxy*,void (**)(void), void*))LGetLibFunction(wwindowlib_handle,"wl_proxy_add_listener");
    
    wl_proxy_marshal_fptr = (void (*)(wl_proxy*,u32,...))LGetLibFunction(wwindowlib_handle,"wl_proxy_marshal");
    
    wl_proxy_set_user_data_fptr = 
        (void (*)(wl_proxy*,void*))LGetLibFunction(wwindowlib_handle,"wl_proxy_set_user_data"); 
    
    wl_proxy_get_user_data_fptr = (void* (*)(wl_proxy*))LGetLibFunction(wwindowlib_handle,"wl_proxy_get_user_data");
    
    wl_proxy_get_version_fptr = (u32 (*)(wl_proxy*))LGetLibFunction(wwindowlib_handle,"wl_proxy_get_version");
    
    wl_proxy_destroy_fptr = (void (*)(wl_proxy*))LGetLibFunction(wwindowlib_handle,"wl_proxy_destroy");
    
    wl_proxy_marshal_constructor_versioned_fptr = 
    
        (wl_proxy* (*)(wl_proxy*,
                       u32,
                       const wl_interface*,u32,...))LGetLibFunction(wwindowlib_handle,"wl_proxy_marshal_constructor_versioned");
    
    //wl_interface* 
    wl_display_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_display_interface");
    
    wl_registry_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_registry_interface");
    
    wl_compositor_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_compositor_interface");
    
    wl_seat_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_seat_interface");
    
    wl_shell_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_shell_interface");
    
    wl_pointer_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_pointer_interface");
    
    wl_keyboard_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_keyboard_interface");
    
    wl_surface_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_surface_interface");
    
    wl_shell_surface_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_shell_surface_interface");
    
    wl_callback_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_callback_interface");
    
    wl_region_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_region_interface");
    
    wl_buffer_interface_ptr =
        (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_buffer_interface");
    
    wl_shm_pool_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_shm_pool_interface");
    
    wl_data_source_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_data_source_interface");
    
    wl_data_device_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_data_device_interface");
    
    wl_touch_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_touch_interface");
    
    wl_subsurface_interface_ptr = (wl_interface*)LGetLibFunction(wwindowlib_handle,"wl_subsurface_interface");
    
}


logic InternalCreateWaylandWindow(WWindowContext* context,const s8* title,
                                  WCreateFlags flags,u32 x,u32 y,u32 width,u32 height){
    
    if(!InternalLoadLibraryWayland()){
        return false;    
    }
    
    *context = {};
    
    WaylandData wdata = {};
    
    //get all the functions needed for init
    auto wl_display_connect_fptr =
        (wl_display* (*)(const s8*))LGetLibFunction(wwindowlib_handle,"wl_display_connect");
    
    auto wl_display_dispatch_fptr = (s32 (*)(wl_display*))LGetLibFunction(wwindowlib_handle,"wl_display_dispatch");
    auto wl_display_roundtrip_fptr = (s32 (*)(wl_display*))LGetLibFunction(wwindowlib_handle,"wl_display_roundtrip");
    
    
    auto display = wl_display_connect_fptr(0);
    
    if(!display){
        LUnloadLibrary(wwindowlib_handle);
        wwindowlib_handle = 0;
        loaded_lib_type = 0;
        return false;
    }
    
    
    InternalLoadWaylandSymbols();
    
    
    //TODO: do whatever that needs to be done to open a wayland window
    
    wl_registry* registry = wl_display_get_registry(display);
    
    wl_registry_add_listener(registry,&registry_listener,(void*)&wdata);
    
    wl_display_dispatch_fptr(display);
    wl_display_roundtrip_fptr(display);
    
    
    //create surfaces
    
    wdata.surface = wl_compositor_create_surface(wdata.compositor);
    
    wdata.shell_surface = 
        wl_shell_get_shell_surface(wdata.shell,wdata.surface);
    
    wl_shell_surface_add_listener(wdata.shell_surface, &shell_surface_listener,(void*)&wdata);
    
    wl_shell_surface_set_toplevel(wdata.shell_surface);
    wl_shell_surface_set_title(wdata.shell_surface,title);
    
    
    impl_wkeycodetoascii = WKeyCodeToASCIIWayland;
    impl_wwaitforevent = WWaitForWindowEventWayland;
    impl_wsettitle = WSetTitleWayland;
    
    
    context->type = _WAYLAND_WINDOW;
    context->width = width;
    context->height = height;
    
    context->window = wdata.surface;
    context->handle = display;
    
    return true;
}





s8 WKeyCodeToASCII(u32 keycode){
    return impl_wkeycodetoascii(keycode);
}

u32 WWaitForWindowEvent(WWindowContext* windowcontext,
                        WWindowEvent* event){
    
    return impl_wwaitforevent(windowcontext,event);
}

void WSetTitle(WWindowContext* context,const s8* title){
    impl_wsettitle(context,title);
}

#define W_CREATE_NO_CHECK (1 << 3)

WWindowContext WCreateWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,
                             u32 width,u32 height){
    
    WWindowContext context = {};
    
    logic res = 0;
    
    if(!(flags & W_CREATE_FORCE_XLIB)){
        res = InternalCreateWaylandWindow(&context,title,flags,x,y,width,height);
    }
    
    if(!res && !(flags & W_CREATE_FORCE_WAYLAND)){
        res = InternalCreateX11Window(&context,title,flags,x,y,width,height);  
    }
    
    
    _kill("Create window failed: either failed to load window lib,failed to connect to window manager or failed to get a hw enabled window\n",!res && !(W_CREATE_NO_CHECK & flags));
    
    return context;
}

#include "vvulkan.h"
#include "pparse.h"

WWindowContext WCreateVulkanWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,u32 width,
                                   u32 height){
    
    WWindowContext context = {};
    
    
    VkExtensionProperties extension_array[32] = {};
    u32 count = 0;
    
    _kill("VInitVulkan must be called before calling this function\n",vkEnumerateInstanceExtensionProperties == 0);
    
    vkEnumerateInstanceExtensionProperties(0,&count,0);
    
    _kill("too many\n",count > _arraycount(extension_array));
    
    vkEnumerateInstanceExtensionProperties(0,&count,&extension_array[0]);
    
    logic wayland_enabled = false;
    
    for(u32 i = 0; i < count; i++){
        
        if(PHashString(extension_array[i].extensionName) == PHashString("VK_KHR_wayland_surface")){
            wayland_enabled = true;
            break;
        }
    }
    
    context = WCreateWindow(title,(WCreateFlags)(flags | W_CREATE_FORCE_WAYLAND | W_CREATE_NO_CHECK),x,y,width,height);
    
    if(!context.handle){
        context = WCreateWindow(title,(WCreateFlags)(flags | W_CREATE_FORCE_XLIB | W_CREATE_NO_CHECK),x,y,width,height);
    }
    
    _kill("Create window failed: either failed to load window lib,failed to connect to window manager or failed to get a hw enabled window\n",!context.handle);
    
    return context;
}