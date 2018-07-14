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
#include "wayland_wwindow.cpp"

//TODO: We can probably get rid of some of the branching



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

#include "pparse.h"
#include "vvulkan.h"

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