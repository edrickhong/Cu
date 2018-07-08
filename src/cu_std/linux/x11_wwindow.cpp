#pragma once

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


s8 WKeyCodeToASCIIX11(u32 keycode){
    return wtext_buffer[keycode];
}

u32 WWaitForWindowEventX11(WWindowContext* windowcontext,
                           WWindowEvent* event){
    
    //NOTE: we might need to create the exit atom. I have an example in the handmade dir
    
    auto queue_count = XPending((Display*)windowcontext->handle);
    
    
    if(queue_count){
        
        XEvent xevent = {};
        XNextEvent((Display*)windowcontext->handle,&xevent);
        
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
    XStoreName((Display*)context->handle,(Window)context->window,(s8*)title);
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
    
    context->handle = XOpenDisplay_fptr(0);
    
    if(!context->handle){
        LUnloadLibrary(wwindowlib_handle);
        wwindowlib_handle = 0;
        loaded_lib_type = 0;
        return false;
    }
    
    _kill("failed to open display\n",!context->handle);
    
    auto visual_ptr = DefaultVisual(context->handle,0);
    auto depth = DefaultDepth(context->handle,0);
    
    context->x11_visualid = XVisualIDFromVisual_fptr(visual_ptr);
    
    XSetWindowAttributes frame_attrib = {};
    frame_attrib.background_pixel = XWhitePixel_fptr((Display*)context->handle,0);
    
#define borderwidth 1
    
    context->window = (void*)XCreateWindow_fptr((Display*)context->handle,
                                                XRootWindow_fptr((Display*)context->handle,0),
                                                x,y,width,height,borderwidth,depth,InputOutput,
                                                visual_ptr,CWBackPixel,&frame_attrib);
    
    WSetTitle(context,title);
    
    XSelectInput_fptr((Display*)context->handle,(Window)context->window,
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
    
    XSetWMNormalHints((Display*)context->handle,(Window)context->window,&hints);
    
    //create exit atom - MARK:Idk if this handles the case where the atom already exists
    auto atom_exit = XInternAtom_fptr((Display*)context->handle,"WM_DELETE_WINDOW",false);
    
    if(atom_exit){
        XSetWMProtocols((Display*)context->handle,(Window)context->window,&atom_exit,1);  
    }
    
    //Set window class
    
    auto XSetClassHint_fptr = (void (*)(Display*,Window,XClassHint*))LGetLibFunction(wwindowlib_handle,"XSetClassHint");
    
    
    XClassHint hint = {(s8*)title,(s8*)title};
    
    XSetClassHint_fptr((Display*)context->handle,(Window)context->window,&hint);
    
    
    XFlush((Display*)context->handle);
    
    XMapWindow_fptr((Display*)context->handle,(Window)context->window);
    
    //MARK: we can set multiple with XSetWMProperties
    
    
    return true;
}