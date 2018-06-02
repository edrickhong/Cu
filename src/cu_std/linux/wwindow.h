#pragma once

#include "ttype.h"
#include "mode.h"

#include "X11/Xlib.h"
#include "X11/Xutil.h"

#include "wayland-client.h"

#define _X11_WINDOW 1
#define _WAYLAND_WINDOW 2

struct WWindowContext{
    
    u32 type;
    
    u16 width;
    u16 height;
    
    union{
        
        struct{
            Display* x11_handle;
            Window x11_rootwindow;
            VisualID x11_visualid;
        };
        
        struct{
            wl_display* wayland_handle;
            wl_surface* wayland_rootwindow;
        };
        
    };  
    
};

enum WCreateFlags{
    W_CREATE_NONE = 0,
    W_CREATE_NORESIZE = 1,
    
    //TODO: actually use these
    W_CREATE_FORCE_WAYLAND = 1 << 1,
    W_CREATE_FORCE_XLIB = 1 << 2,
};

struct WKeyboardEvent{
    u32 keycode;
};

struct WMouseEvent{
    
    union{
        u32 keycode;
        struct {
            //relative to the top left corner of the window
            u16 x;
            u16 y;
        };
    };
};

enum WEventType{
    W_EVENT_NONE = 0,
    W_EVENT_EXPOSE = Expose,
    W_EVENT_CLOSE = 0xFFFFFFFF,
    W_EVENT_RESIZE = 0xFFFFFFFE,
    
    W_EVENT_KBEVENT_KEYDOWN = KeyPress,
    W_EVENT_KBEVENT_KEYUP = KeyRelease,
    
    
    W_EVENT_MSEVENT_MOVE = MotionNotify,
    
    W_EVENT_MSEVENT_DOWN,
    W_EVENT_MSEVENT_UP,
};

struct WWindowEvent{
    WEventType type;
    
    union{
        WKeyboardEvent keyboard_event;
        WMouseEvent mouse_event;
    };
    
};


struct KeyboardState{
    s8 prevkeystate[256] = {};
    s8 curkeystate[256] = {};
};

struct MouseState{
    u16 x;
    u16 y;
    s8 curstate[8];
    s8 prevstate[8];
};

enum MouseButton{
    MOUSEBUTTON_LEFT = 0,
    MOUSEBUTTON_RIGHT = 1,
    MOUSEBUTTON_MIDDLE = 2,
    MOUSEBUTTON_BUTTON1 = 3,
    MOUSEBUTTON_BUTTON2 = 4,
    MOUSEBUTTON_SCROLLUP = 5,
    MOUSEBUTTON_SCROLLDOWN = 6,
};

u32 _ainline IsKeyPressed(KeyboardState* state,u32 keysym){
    return state->curkeystate[keysym] & !state->prevkeystate[keysym];
}

u32 _ainline IsKeyDown(KeyboardState* state,u32 keysym){
    return state->curkeystate[keysym];
}

u32 _ainline  IsKeyUp(KeyboardState* state,u32 keysym){
    return !state->curkeystate[keysym];
}


u32 _ainline IsKeyPressed(MouseState* state,MouseButton mousekey){
    return state->curstate[mousekey] & !state->prevstate[mousekey];
}

u32 _ainline  IsKeyDown(MouseState* state,MouseButton mousekey){
    return state->curstate[mousekey];
}

u32 _ainline  IsKeyUp(MouseState* state,MouseButton mousekey){
    return !state->curstate[mousekey];
}

u32 WWaitForWindowEvent(WWindowContext* windowcontext,WWindowEvent* event);


WWindowContext WCreateWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,u32 width,
                             u32 height);

//TODO: implement this
WWindowContext WCreateVulkanWindow(void* vk_instance,const s8* title,WCreateFlags flags,u32 x,u32 y,u32 width,
                                   u32 height);

void WDestroyWindow(WWindowContext* windowcontext);

void WMessageBox(WWindowContext* windowcontext,const s8* text,const s8* caption,u32 type);

s8 WKeyCodeToASCII(u32 keycode);

//TODO: Doesn't work
void WSetIcon(WWindowContext* windowcontext,void* icondata,u32 width,u32 height);

void WSetTitle(WWindowContext* windowcontext,const s8* title_string);




