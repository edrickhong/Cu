
#include "wwindow.h"
#include "libload.h"


_persist s8 wtext_buffer[256] ={};

_persist LibHandle wwindowlib_handle = 0;
_persist u32 loaded_lib_type = 0;

//these fptrs will be shared w wayland as well
_persist void* wfptr_x11_xstorename = 0;
_persist void* wfptr_x11_xflush = 0;
_persist void* wfptr_x11_xpending = 0;
_persist void* wfptr_x11_xnextevent = 0;
_persist void* wfptr_x11_xsetwmnormalhints = 0;
_persist void* wfptr_x11_xrefreshkeyboardmapping = 0;
_persist void* wfptr_x11_xsync = 0;
_persist void* wfptr_x11_xsetwmprotocols = 0;
_persist void* wfptr_x11_xlookupstring = 0;
_persist void* wfptr_x11_xconfigurewindow = 0;


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

logic InternalLoadLibraryX11(){
    
    
    if(wwindowlib_handle){
        
        if(loaded_lib_type != _X11_WINDOW){
            return false;
        }
        
        return true;
    }
    
    const s8* x11_paths[] = 
    {
        "libX11.so.6.3.0",
        "libX11.so.6",
        "libX11.so",
    };
    
    for(u32 i = 0; i < _arraycount(x11_paths); i++){
        wwindowlib_handle = LLoadLibrary(x11_paths[i]);
        
        if(wwindowlib_handle){
            break;  
        }
        
    }
    
    if(!wwindowlib_handle){
        return false;
    }
    
    loaded_lib_type = _X11_WINDOW;
    
    wfptr_x11_xflush = LGetLibFunction(wwindowlib_handle,"XFlush");
    
    wfptr_x11_xpending = LGetLibFunction(wwindowlib_handle,"XPending");
    
    wfptr_x11_xnextevent = LGetLibFunction(wwindowlib_handle,"XNextEvent");
    
    wfptr_x11_xsetwmnormalhints = LGetLibFunction(wwindowlib_handle,"XSetWMNormalHints");
    
    wfptr_x11_xrefreshkeyboardmapping = LGetLibFunction(wwindowlib_handle,"XRefreshKeyboardMapping");
    
    wfptr_x11_xsync = LGetLibFunction(wwindowlib_handle,"XSync");
    
    wfptr_x11_xstorename = LGetLibFunction(wwindowlib_handle,"XStoreName");
    
    
    wfptr_x11_xsetwmprotocols = LGetLibFunction(wwindowlib_handle,"XSetWMProtocols");
    
    //TODO: we should be using the utf8 version: Xutf8LookupString XCreateIC
    wfptr_x11_xlookupstring = LGetLibFunction(wwindowlib_handle,"XLookupString");
    
    wfptr_x11_xconfigurewindow = LGetLibFunction(wwindowlib_handle,"XConfigureWindow");
    
    return true;
}

#define XStoreName ((s32 (*)(Display*,Window,s8*))wfptr_x11_xstorename)


#define XFlush ((s32 (*)(Display*))wfptr_x11_xflush)

#define XPending ((s32 (*)(Display*))wfptr_x11_xpending)

#define XNextEvent ((s32 (*)(Display*,XEvent*))wfptr_x11_xnextevent)

#define XSetWMNormalHints ((void (*)(Display*,Window,XSizeHints*))wfptr_x11_xsetwmnormalhints)

#define XRefreshKeyboardMapping ((s32 (*)(XMappingEvent*))wfptr_x11_xrefreshkeyboardmapping)

#define XSync ((s32 (*)(Display*,Bool))wfptr_x11_xsync)



#define XSetWMProtocols ((Status (*)(Display*,Window,Atom*,s32))wfptr_x11_xsetwmprotocols)


#define XLookupString ((s32 (*)(XKeyEvent*,s8*,s32,KeySym*,XComposeStatus*))wfptr_x11_xlookupstring)


#define XConfigureWindow ((s32 (*)(Display*,Window,u32,XWindowChanges*))wfptr_x11_xconfigurewindow)


//wayland stuff

//function implementations
_persist s8 (*impl_wkeycodetoascii)(u32) = 0;
_persist u32 (*impl_wwaitforevent)(WWindowContext*,WWindowEvent*) = 0;
_persist void (*impl_wsettitle)(WWindowContext*,const s8*) = 0;


logic InternalCreateWaylandWindow(WWindowContext* context,const s8* title,
                                  WCreateFlags flags,u32 x,u32 y,u32 width,u32 height){
    
    if(!InternalLoadLibraryWayland()){
        return false;    
    }
    
    //get all the functions needed for init
    auto wl_display_connect_fptr =
        (wl_display* (*)(const s8*))LGetLibFunction(wwindowlib_handle,"wl_display_connect");
    
    context->type = _WAYLAND_WINDOW;
    context->width = width;
    context->height = height;
    
    context->wayland_handle = wl_display_connect_fptr(0);
    
    if(!context->wayland_handle){
        LUnloadLibrary(wwindowlib_handle);
        wwindowlib_handle = 0;
        loaded_lib_type = 0;
        return false;
    }
    
    //TODO: do whatever that needs to be done to open a wayland window
    
    return true;
}


s8 WKeyCodeToASCIIX11(u32 keycode){
    return wtext_buffer[keycode];
}

u32 WWaitForWindowEventX11(WWindowContext* windowcontext,
                           WWindowEvent* event){
    
    //NOTE: we might need to create the exit atom. I have an example in the handmade dir
    
    auto queue_count = XPending(windowcontext->x11_handle);
    
    
    if(queue_count){
        
        XEvent xevent = {};
        XNextEvent(windowcontext->x11_handle,&xevent);
        
        switch(xevent.type){
            
            case Expose:{
                event->type = W_EVENT_EXPOSE;
            }break;
            
            case ClientMessage:{
                event->type = W_EVENT_CLOSE;
            }break;
            
            case ConfigureNotify:{
                event->type = W_EVENT_RESIZE;
            }break;
            
            case KeymapNotify:{
                XRefreshKeyboardMapping(&xevent.xmapping);
            }break;
            
            case KeyPress:{
                
                event->type = W_EVENT_KBEVENT_KEYDOWN;
                event->keyboard_event.keycode = xevent.xkey.keycode;
                
                XLookupString(&xevent.xkey,&wtext_buffer[xevent.xkey.keycode],1,0,0);
                
            }break;
            
            case KeyRelease:{
                event->type = W_EVENT_KBEVENT_KEYUP;
                event->keyboard_event.keycode = xevent.xkey.keycode;
            }break;
            
            case MotionNotify:{
                event->type = W_EVENT_MSEVENT_MOVE;
                event->mouse_event.x = xevent.xmotion.x;
                event->mouse_event.y = xevent.xmotion.y;
            }break;
            
            case ButtonPress:{
                
                event->type = W_EVENT_MSEVENT_DOWN;
                
                switch(xevent.xbutton.button){
                    
                    case 1:{
                        event->mouse_event.keycode =MOUSEBUTTON_LEFT;
                    }break;//left
                    
                    case 2:{
                        event->mouse_event.keycode =MOUSEBUTTON_MIDDLE;
                    }break;//middle
                    
                    case 3:{
                        event->mouse_event.keycode =MOUSEBUTTON_RIGHT;
                    }break;//right
                    
                    case 4:{
                        event->mouse_event.keycode =MOUSEBUTTON_SCROLLUP;
                    }break;//up
                    
                    case 5:{
                        event->mouse_event.keycode =MOUSEBUTTON_SCROLLDOWN;
                    }break;//down
                    
                    case 8:{
                        event->mouse_event.keycode =MOUSEBUTTON_BUTTON1;
                    }break;//button1
                    
                    case 9:{
                        event->mouse_event.keycode =MOUSEBUTTON_BUTTON2;
                    }break;//button2
                    
                }
                
            }break;
            
            case ButtonRelease:{
                
                event->type = W_EVENT_MSEVENT_UP;
                
                switch(xevent.xbutton.button){
                    
                    case 1:{
                        event->mouse_event.keycode =MOUSEBUTTON_LEFT;
                    }break;//left
                    
                    case 2:{
                        event->mouse_event.keycode =MOUSEBUTTON_MIDDLE;
                    }break;//middle
                    
                    case 3:{
                        event->mouse_event.keycode =MOUSEBUTTON_RIGHT;
                    }break;//right
                    
                    case 4:{
                        event->mouse_event.keycode =MOUSEBUTTON_SCROLLUP;
                    }break;//up
                    
                    case 5:{
                        event->mouse_event.keycode =MOUSEBUTTON_SCROLLDOWN;
                    }break;//down
                    
                    case 8:{
                        event->mouse_event.keycode =MOUSEBUTTON_BUTTON1;
                    }break;//button1
                    
                    case 9:{
                        event->mouse_event.keycode =MOUSEBUTTON_BUTTON2;
                    }break;//button2
                    
                }
                
            }break;
            
            default:{
            }break;
            
        }
        
    }
    
    return queue_count;  
}

void WSetTitleX11(WWindowContext* context,const s8* title){
    XStoreName(context->x11_handle,context->x11_rootwindow,(s8*)title);
}



logic InternalCreateX11Window(WWindowContext* context,const s8* title,WCreateFlags flags,
                              u32 x,u32 y,u32 width,u32 height){
    
    if(!InternalLoadLibraryX11()){
        return false;
    }
    
    impl_wkeycodetoascii = WKeyCodeToASCIIX11;
    impl_wwaitforevent = WWaitForWindowEventX11;
    impl_wsettitle = WSetTitleX11;
    
    //get all the functions needed for init
    
    auto XOpenDisplay_fptr =
        (Display* (*)(s8*))LGetLibFunction(wwindowlib_handle,"XOpenDisplay");
    
    auto XCreateWindow_fptr =
        (Window (*)(Display*,Window,s32,s32,u32,u32,u32,s32, u32, Visual*,unsigned long,
                    XSetWindowAttributes*))LGetLibFunction(wwindowlib_handle,"XCreateWindow");
    
    auto XWhitePixel_fptr =
        (unsigned long (*)(Display*,s32))LGetLibFunction(wwindowlib_handle,"XWhitePixel");
    
    auto XRootWindow_fptr =
        (Window (*)(Display*,s32))LGetLibFunction(wwindowlib_handle,"XRootWindow");
    
    auto XSelectInput_fptr =
        (s32 (*)(Display*,Window,long))LGetLibFunction(wwindowlib_handle,"XSelectInput");
    
    auto XMapWindow_fptr =
        (s32 (*)(Display*,Window))LGetLibFunction(wwindowlib_handle,"XMapWindow");
    
    auto XInternAtom_fptr =
        (Atom (*)(Display*,_Xconst char*,Bool))LGetLibFunction(wwindowlib_handle,"XInternAtom");
    
    auto XVisualIDFromVisual_fptr =
        (VisualID (*)(Visual*))LGetLibFunction(wwindowlib_handle,"XVisualIDFromVisual");
    
    context->type = _X11_WINDOW;
    context->width = width;
    context->height = height;
    
    context->x11_handle = XOpenDisplay_fptr(0);
    
    if(!context->x11_handle){
        LUnloadLibrary(wwindowlib_handle);
        wwindowlib_handle = 0;
        loaded_lib_type = 0;
        return false;
    }
    
    _kill("failed to open display\n",!context->x11_handle);
    
    auto visual_ptr = DefaultVisual(context->x11_handle,0);
    auto depth = DefaultDepth(context->x11_handle,0);
    
    context->x11_visualid = XVisualIDFromVisual_fptr(visual_ptr);
    
    XSetWindowAttributes frame_attrib = {};
    frame_attrib.background_pixel = XWhitePixel_fptr(context->x11_handle,0);
    
#define borderwidth 1
    
    context->x11_rootwindow = XCreateWindow_fptr(context->x11_handle,
                                                 XRootWindow_fptr(context->x11_handle,0),
                                                 x,y,width,height,borderwidth,depth,InputOutput,
                                                 visual_ptr,CWBackPixel,&frame_attrib);
    
    WSetTitle(context,title);
    
    XSelectInput_fptr(context->x11_handle,context->x11_rootwindow,
                      ExposureMask|ButtonPressMask|
                      ButtonReleaseMask|KeyReleaseMask|KeyPressMask|
                      StructureNotifyMask | PointerMotionMask);
    
    XSizeHints hints = {};
    
    hints.flags = PPosition | PSize;
    hints.x = (s32)x;
    hints.y = (s32)y;
    hints.width = width;
    hints.height = height;
    
    if(flags & W_CREATE_NORESIZE){
        
        hints.flags |= PMinSize | PMaxSize;
        hints.min_width = width;
        hints.min_height = height;
        hints.max_width = width;
        hints.max_height = height;
        
    }
    
    XSetWMNormalHints(context->x11_handle,context->x11_rootwindow,&hints);
    
    //create exit atom - MARK:Idk if this handles the case where the atom already exists
    auto atom_exit = XInternAtom_fptr(context->x11_handle,"WM_DELETE_WINDOW",false);
    
    if(atom_exit){
        XSetWMProtocols(context->x11_handle,context->x11_rootwindow,&atom_exit,1);  
    }
    
    //Set window class
    
    auto XSetClassHint_fptr = (void (*)(Display*,Window,XClassHint*))LGetLibFunction(wwindowlib_handle,"XSetClassHint");
    
    
    XClassHint hint = {(s8*)title,(s8*)title};
    
    XSetClassHint_fptr(context->x11_handle,context->x11_rootwindow,&hint);
    
    
    XFlush(context->x11_handle);
    
    XMapWindow_fptr(context->x11_handle,context->x11_rootwindow);
    
    //MARK: we can set multiple with XSetWMProperties
    
    
    return true;
}

WWindowContext WCreateWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,
                             u32 width,u32 height){
    
    WWindowContext context = {};
    
    logic res;
    
#if !(_disable_wayland_path)
    
    res = InternalCreateWaylandWindow(&context,title,flags,x,y,width,height);
    
    if(!res){
        res = InternalCreateX11Window(&context,title,flags,x,y,width,height);  
    }
    
#else
    
    res = InternalCreateX11Window(&context,title,flags,x,y,width,height);  
    
#endif
    
    
    _kill("Create window failed: either failed to load window lib,failed to connect to window manager or failed to get a hw enabled window\n",!res);
    
    return context;
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

#include "vvulkan.h"
#include "pparse.h"

WWindowContext WCreateVulkanWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,u32 width,
                                   u32 height){
    
    WWindowContext context = {};
    
    
    VkExtensionProperties extension_array[32] = {};
    u32 count = 0;
    
    _kill("VCreateInstance must be called before calling this function\n",vkEnumerateInstanceExtensionProperties == 0);
    
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
    
#if (_disable_wayland_path)
    
    wayland_enabled = false;
    
#endif
    
    logic res = false;
    
    if(wayland_enabled){
        
        res = InternalCreateWaylandWindow(&context,title,flags,x,y,width,height);
    }
    
    if(!res){
        res = InternalCreateX11Window(&context,title,flags,x,y,width,height);  
    }
    
    _kill("Create window failed: either failed to load window lib,failed to connect to window manager or failed to get a hw enabled window\n",!res);
    
    return context;
}