#pragma once

#ifdef _WAYLAND_CLIENT_H
#error Error Client is already included
#endif

#ifdef WAYLAND_CLIENT_PROTOCOL_H
#error Error Protocol is already included
#endif

#include "wayland-util.h"


/*
Handles the inline function replacement in wayland-client-protocol.h
and replaces them with our dynamic interfaces
*/

struct wl_interface;
struct wl_proxy;
struct wl_event_queue;
struct wl_display;
struct wl_surface;
struct wl_shm;

struct wl_compositor;
struct wl_shell;
struct wl_seat;
struct wl_pointer;
struct wl_keyboard;
struct wl_surface;
struct wl_shell_surface;

extern wl_proxy* (*wl_proxy_marshal_constructor_fptr)(wl_proxy*,u32,const wl_interface*,...);

extern s32 (*wl_proxy_add_listener_fptr)(wl_proxy*,void (**)(void), void*);

extern void (*wl_proxy_marshal_fptr)(wl_proxy*,u32,...);

extern void
(*wl_proxy_set_user_data_fptr)(wl_proxy*,void*); 

extern void* (*wl_proxy_get_user_data_fptr)(wl_proxy*);

extern u32 (*wl_proxy_get_version_fptr)(wl_proxy*);

extern void (*wl_proxy_destroy_fptr)(wl_proxy*);

extern wl_proxy* (*wl_proxy_marshal_constructor_versioned_fptr)(wl_proxy*,
                                                                u32,
                                                                const wl_interface*,u32,...);

extern s32 (*wl_display_prepare_read_fptr)(wl_display*);

extern s32 (*wl_display_dispatch_pending_fptr)(wl_display*);

extern s32 (*wl_display_flush_fptr)(wl_display*);

extern s32 (*wl_display_read_events_fptr)(wl_display*);

extern s32 (*wl_display_get_fd_fptr)(wl_display*);

extern s32 (*wl_display_dispatch_ftpr)(wl_display*);

#define wl_proxy_marshal_constructor wl_proxy_marshal_constructor_fptr
#define wl_proxy_add_listener wl_proxy_add_listener_fptr
#define wl_proxy_marshal wl_proxy_marshal_fptr
#define wl_proxy_set_user_data wl_proxy_set_user_data_fptr
#define wl_proxy_get_user_data wl_proxy_get_user_data_fptr
#define wl_proxy_get_version wl_proxy_get_version_fptr
#define wl_proxy_destroy wl_proxy_destroy_fptr
#define wl_proxy_marshal_constructor_versioned wl_proxy_marshal_constructor_versioned_fptr

#define wl_display_prepare_read wl_display_prepare_read_fptr
#define wl_display_dispatch_pending wl_display_dispatch_pending_fptr
#define wl_display_flush wl_display_flush_fptr
#define wl_display_read_events wl_display_read_events_fptr

#define wl_display_get_fd wl_display_get_fd_fptr

#define wl_display_dispatch wl_display_dispatch_ftpr



extern "C" const wl_interface* wl_display_interface_ptr;
extern "C" const wl_interface* wl_registry_interface_ptr;
extern "C" const wl_interface* wl_compositor_interface_ptr;
extern "C" const wl_interface* wl_seat_interface_ptr;
extern "C" const wl_interface* wl_shell_interface_ptr;
extern "C" const wl_interface* wl_pointer_interface_ptr;
extern "C" const wl_interface* wl_keyboard_interface_ptr;
extern "C" const wl_interface* wl_surface_interface_ptr;
extern "C" const wl_interface* wl_shell_surface_interface_ptr;
extern "C" const wl_interface* wl_callback_interface_ptr;
extern "C" const wl_interface* wl_region_interface_ptr;
extern "C" const wl_interface* wl_buffer_interface_ptr;
extern "C" const wl_interface* wl_shm_pool_interface_ptr;
extern "C" const wl_interface* wl_data_source_interface_ptr;
extern "C" const wl_interface* wl_data_device_interface_ptr;
extern "C" const wl_interface* wl_touch_interface_ptr;
extern "C" const wl_interface* wl_subsurface_interface_ptr;

#define wl_display_interface *wl_display_interface_ptr
#define wl_registry_interface *wl_registry_interface_ptr
#define wl_compositor_interface *wl_compositor_interface_ptr
#define wl_seat_interface *wl_seat_interface_ptr
#define wl_shell_interface *wl_shell_interface_ptr
#define wl_pointer_interface *wl_pointer_interface_ptr
#define wl_keyboard_interface *wl_keyboard_interface_ptr
#define wl_surface_interface *wl_surface_interface_ptr
#define wl_shell_surface_interface *wl_shell_surface_interface_ptr
#define wl_callback_interface *wl_callback_interface_ptr
#define wl_region_interface *wl_region_interface_ptr
#define wl_buffer_interface *wl_buffer_interface_ptr
#define wl_shm_pool_interface *wl_shm_pool_interface_ptr
#define wl_data_source_interface *wl_data_source_interface_ptr
#define wl_data_device_interface *wl_data_device_interface_ptr
#define wl_touch_interface *wl_touch_interface_ptr
#define wl_subsurface_interface *wl_subsurface_interface_ptr


#define WAYLAND_CLIENT_CORE_H
#define WAYLAND_CLIENT_H
#include "wayland-client-protocol.h"