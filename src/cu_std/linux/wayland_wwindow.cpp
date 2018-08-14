#pragma once

/*
TODO:
fix the pointer to use the default pointer (requires dumb parsing)
fix quit message (requires xdg extensions)

xdg-shell-client-protocol.h
*/

#include "wayland_dyn.h"
#include "ssys.h"
#include "xkbcommon/xkbcommon.h"

_persist LibHandle xkb_lib = 0;
_persist xkb_context* xkb_ctx = 0;
_persist xkb_keymap* xkb_kbmap = 0;
_persist xkb_state* xkb_kbstate = 0;

s32 (*xkb_state_key_get_utf8_fptr)(xkb_state*,xkb_keycode_t,s8*,size_t) = 0;


_persist WWindowEvent wayland_event_array[32] = {};
_persist u32 wayland_event_count = 0;

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

s32 (*wl_display_prepare_read_fptr)(wl_display*) = 0;

s32 (*wl_display_dispatch_pending_fptr)(wl_display*) = 0;

s32 (*wl_display_flush_fptr)(wl_display*) = 0;

s32 (*wl_display_read_events_fptr)(wl_display*) = 0;

s32 (*wl_display_get_fd_fptr)(wl_display*) = 0;

s32 (*wl_display_dispatch_ftpr)(wl_display*) = 0;

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

struct WaylandData{
    //We don't touch these alot
    wl_compositor* compositor;
    wl_shell* shell;
    wl_seat* seat;
    wl_pointer* pointer;
    wl_keyboard* keyboard;
};




logic InternalLoadLibraryWayland(){
    
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
    
    const s8* xkb_paths[] = {
        "libxkbcommon.so.0.0.0",
        "libxkbcommon.so.0",
        "libxkbcommon.so"
    };
    
    for(u32 i = 0; i < _arraycount(xkb_paths); i++){
        
        xkb_lib = LLoadLibrary(xkb_paths[i]);
        
        if(xkb_lib){
            break;  
        }
        
    }
    
    if(!wwindowlib_handle || !xkb_lib){
        return false;
    }
    
    
    loaded_lib_type = _WAYLAND_WINDOW;
    
    return true;
}



//wayland stuff

s8 WKeyCodeToASCIIWayland(u32 keycode){
    
    s8 buffer[128] = {};
    
    xkb_state_key_get_utf8_fptr(xkb_kbstate,keycode,buffer,sizeof(buffer));
    
    return buffer[0];
}

#include "debugtimer.h"

u32 WWaitForWindowEventWayland(WWindowContext* windowcontext,
                               WWindowEvent* event){
    
    auto fd = wl_display_get_fd((wl_display*)windowcontext->handle);
    
    s32 res = 0;
    
    do{
        
        pollfd poll_info = {
            fd,
            POLLIN | POLLPRI,//wait for input events
        };
        
        //returns 0 if no events read and timed out
        //returns positive if got events
        //returns negative if error
        res = poll(&poll_info,1,0);
        
    }while(res < 0 && errno == EINTR);//EINTR - interrupt occured
    
    
    
    if(res){
        wl_display_dispatch((wl_display*)windowcontext->handle);
    }
    
    else{
        wl_display_dispatch_pending((wl_display*)windowcontext->handle);
    }
    
    if(wayland_event_count){
        
        wayland_event_count--;
        *event = wayland_event_array[wayland_event_count];
        
        return 1;
    }
    
    return 0;
}

void WSetTitleWayland(WWindowContext* context,const s8* title){
    
    wl_shell_surface_set_title((wl_shell_surface*)context->wayland_shell_surface,title);
    
}


void WaylandKeyboardMap(void* data,wl_keyboard* keyboard,u32 format,s32 fd,u32 size){
    
    auto string = mmap(0,size,PROT_READ,MAP_SHARED,fd,0);
    
    
    auto xkb_keymap_new_from_string_fptr = (xkb_keymap* (*)(xkb_context*,const s8*,xkb_keymap_format,xkb_keymap_compile_flags))
        LGetLibFunction(xkb_lib,"xkb_keymap_new_from_string");
    
    auto xkb_state_new_fptr = (xkb_state* (*)(xkb_keymap*))LGetLibFunction(xkb_lib,"xkb_state_new");
    
    //TODO: we might want to free if already initialized?
    
    xkb_kbmap = xkb_keymap_new_from_string_fptr(xkb_ctx,(const s8*)string,XKB_KEYMAP_FORMAT_TEXT_V1,XKB_KEYMAP_COMPILE_NO_FLAGS);
    
    xkb_kbstate = xkb_state_new_fptr(xkb_kbmap);
    
    munmap(string,size);
}

void WaylandKeyboardEnter(void* data,wl_keyboard* keyboard,u32 serial,
                          wl_surface* surface,wl_array* keys){
    
}

void WaylandKeyboardLeave(void* data,wl_keyboard* keyboard,u32 serial,wl_surface* surface){}

void WaylandKeyboardKey(void* data,wl_keyboard* keyboard,u32 serial,u32 time,u32 key,u32 state){
    
    _kill("too many events\n",wayland_event_count > _arraycount(wayland_event_array));
    
    auto event = &wayland_event_array[wayland_event_count];
    wayland_event_count++;
    
    
    
    if(state){
        event->type = W_EVENT_KBEVENT_KEYDOWN;
    }
    
    else{
        event->type = W_EVENT_KBEVENT_KEYUP;
    }
    
    event->keyboard_event.keycode = key + 8;
}

void WaylandKeyboardModifiers(void* data,wl_keyboard* keyboard,u32 serial,u32 mods_depressed,u32 mods_latched,u32 mods_locked,u32 group){
    
}

void WaylandPointerEnter(void* data,wl_pointer* pointer,u32 serial, wl_surface* surface,
                         wl_fixed_t sx, wl_fixed_t sy){
    
}

void WaylandPointerLeave(void* data,wl_pointer* pointer,u32 serial,wl_surface* surface){}

void WaylandPointerMotion(void* data,wl_pointer* pointer,u32 time, wl_fixed_t sx, wl_fixed_t sy){
    
    _kill("too many events\n",wayland_event_count > _arraycount(wayland_event_array));
    
    auto event = &wayland_event_array[wayland_event_count];
    wayland_event_count++;
    
    event->type = W_EVENT_MSEVENT_MOVE;
    
    event->mouse_event.x = wl_fixed_to_int(sx);
    event->mouse_event.y = wl_fixed_to_int(sy);
}

void WaylandPointerButton(void* data,wl_pointer* pointer,u32 serial,u32 time,u32 button,u32 state){
    
    _kill("too many events\n",wayland_event_count > _arraycount(wayland_event_array));
    
    auto event = &wayland_event_array[wayland_event_count];
    wayland_event_count++;
    
    if(state){
        event->type = W_EVENT_MSEVENT_DOWN;
    }
    
    else{
        event->type = W_EVENT_MSEVENT_UP;
    }
    
    switch(button){
        
        case 272:{
            event->mouse_event.keycode = MOUSEBUTTON_LEFT;
        }break;
        
        case 274:{
            event->mouse_event.keycode = MOUSEBUTTON_MIDDLE;
        }break;
        
        case 273:{
            event->mouse_event.keycode = MOUSEBUTTON_RIGHT;
        }break;
    }
}

void WaylandPointerAxis(void* data,wl_pointer* pointer,u32 time,u32 axis,wl_fixed_t value){
    
    /*MARK: fill mouse scroll events here*/
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
    
    auto w = (WaylandData*)(((WWindowContext*)data)->internaldata);
    
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
    
    auto w = (WaylandData*)(((WWindowContext*)data)->internaldata);
    
    
    if(PHashString(interface) == PHashString("wl_compositor")){
        
        w->compositor = (wl_compositor*)wl_registry_bind(registry,id,&wl_compositor_interface,1);
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
    
    
    wl_display_prepare_read_fptr = (s32 (*)(wl_display*))LGetLibFunction(wwindowlib_handle,"wl_display_prepare_read");
    
    wl_display_dispatch_pending_fptr = (s32 (*)(wl_display*))LGetLibFunction(wwindowlib_handle,"wl_display_dispatch_pending");
    
    wl_display_flush_fptr = (s32 (*)(wl_display*))LGetLibFunction(wwindowlib_handle,"wl_display_flush");
    
    wl_display_read_events_fptr = (s32 (*)(wl_display*))LGetLibFunction(wwindowlib_handle,"wl_display_read_events");
    
    
    wl_display_get_fd_fptr = (s32 (*)(wl_display*))LGetLibFunction(wwindowlib_handle,"wl_display_get_fd");
    
    wl_display_dispatch_ftpr = (s32 (*)(wl_display*))LGetLibFunction(wwindowlib_handle,"wl_display_dispatch");
    
    
    
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

void InternalLoadXkbSymbols(){
    
    xkb_state_key_get_utf8_fptr = (s32 (*)(xkb_state*,xkb_keycode_t,s8*,size_t))
        LGetLibFunction(xkb_lib,"xkb_state_key_get_utf8");
}


logic InternalCreateWaylandWindow(WWindowContext* context,const s8* title,
                                  WCreateFlags flags,u32 x,u32 y,u32 width,u32 height){
    
    if(!InternalLoadLibraryWayland()){
        return false;    
    }
    
    *context = {};
    
    //get all the functions needed for init
    
    auto xkb_context_new_fptr = (xkb_context* (*)(xkb_context_flags))
        LGetLibFunction(xkb_lib,"xkb_context_new");
    
    
    auto wl_display_connect_fptr =
        (wl_display* (*)(const s8*))LGetLibFunction(wwindowlib_handle,"wl_display_connect");
    
    auto wl_display_dispatch_fptr = (s32 (*)(wl_display*))LGetLibFunction(wwindowlib_handle,"wl_display_dispatch");
    auto wl_display_roundtrip_fptr = (s32 (*)(wl_display*))LGetLibFunction(wwindowlib_handle,"wl_display_roundtrip");
    
    
    auto display = wl_display_connect_fptr(0);
    xkb_ctx= xkb_context_new_fptr(XKB_CONTEXT_NO_FLAGS);
    
    if(!display || !xkb_ctx){
        LUnloadLibrary(wwindowlib_handle);
        wwindowlib_handle = 0;
        loaded_lib_type = 0;
        
        //TODO: disconnect display and xkb_ctx
        
        return false;
    }
    
    context->internaldata = alloc(sizeof(WaylandData));
    auto wdata = (WaylandData*)context->internaldata;
    
    
    InternalLoadWaylandSymbols();
    InternalLoadXkbSymbols();
    
    
    //wayland stuff
    wl_registry* registry = wl_display_get_registry(display);
    
    wl_registry_add_listener(registry,&registry_listener,(void*)context);
    
    wl_display_dispatch_fptr(display);
    wl_display_roundtrip_fptr(display);
    
    
    //create surfaces
    
    context->window = (void*)wl_compositor_create_surface(wdata->compositor);
    
    context->wayland_shell_surface = 
        wl_shell_get_shell_surface(wdata->shell,(wl_surface*)context->window);
    
    wl_shell_surface_add_listener((wl_shell_surface*)context->wayland_shell_surface, &shell_surface_listener,(void*)context);
    
    wl_shell_surface_set_toplevel((wl_shell_surface*)context->wayland_shell_surface);
    
    wl_shell_surface_set_title((wl_shell_surface*)context->wayland_shell_surface,title);
    
    wl_shell_surface_set_class((wl_shell_surface*)context->wayland_shell_surface,title);
    
    
    impl_wkeycodetoascii = WKeyCodeToASCIIWayland;
    impl_wwaitforevent = WWaitForWindowEventWayland;
    impl_wsettitle = WSetTitleWayland;
    
    
    context->type = _WAYLAND_WINDOW;
    context->width = width;
    context->height = height;
    
    context->handle = display;
    
    
    return true;
}
