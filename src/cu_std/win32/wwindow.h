#pragma once

#include "ttype.h"
#include "ccontainer.h"
#include "windows.h"


//~ means flip the bits. eg: ~1100_2 is 0011_2

/*
TODO:
set icon
drag and drop with xdnd protocol
keyboard support
message box?
*/

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

u32 _ainline IsKeyPressed(KeyboardState* state,u32 keysym){
  return state->curkeystate[keysym] && !state->prevkeystate[keysym];
}

u32 _ainline  IsKeyDown(KeyboardState* state,u32 keysym){
  return state->curkeystate[keysym];
}

u32 _ainline  IsKeyUp(KeyboardState* state,u32 keysym){
  return !state->curkeystate[keysym];
}


enum WCreateFlags{
  W_CREATE_NONE = 0,
  W_CREATE_NORESIZE =  1,
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

u32 _ainline IsKeyPressed(MouseState* state,MouseButton mousekey){
  return state->curstate[mousekey] & !state->prevstate[mousekey];
}

u32 _ainline  IsKeyDown(MouseState* state,MouseButton mousekey){
  return state->curstate[mousekey];
}

u32 _ainline  IsKeyUp(MouseState* state,MouseButton mousekey){
  return !state->curstate[mousekey];
}

enum WEventType{
  W_EVENT_NONE = 0,
  W_EVENT_EXPOSE = WM_ACTIVATEAPP,
  W_EVENT_CLOSE = 0xFFFFFFFF,
  W_EVENT_RESIZE = 0xFFFFFFFE,
  
  W_EVENT_KBEVENT_KEYDOWN = WM_KEYDOWN,
  W_EVENT_KBEVENT_KEYUP = WM_KEYUP,

  
  W_EVENT_MSEVENT_MOVE = WM_MOUSEMOVE,
  W_EVENT_MSEVENT_DOWN,
  W_EVENT_MSEVENT_UP,

  Internal_ForceMyEnumIntSize = 0xFFFFFFFF,  
};


struct WKeyboardEvent{
  u32 keycode;
};

struct WMouseEvent{
  
  union{
    struct {
      //relative to the top left corner of the window
      u16 x;
      u16 y;
    };
    u32 keycode;
  };
};

struct WWindowEvent{
  WEventType type;

  union{
    WKeyboardEvent keyboard_event;
    WMouseEvent mouse_event;
  };
};


struct WWindowContext{
  HWND rootwindow;
  u16 width;
  u16 height;
  HMODULE handle;
};


u32 WWaitForWindowEvent(WWindowContext* windowcontext,WWindowEvent* event);

WWindowContext WCreateWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,u32 width,
				u32 height);

void WDestroyWindow(WWindowContext* windowcontext);

//destroy and removes a subwindow at index from the subwindow list 
void WDestroySubWindow(WWindowContext* windowcontext,ptrsize index);

//pushes a subwindow to the subwindow list
void WCreateSubWindow(WWindowContext* windowcontext);

void WMessageBox(WWindowContext* windowcontext,const s8* text,const s8* caption,u32 type);


void WSetIcon(WWindowContext* windowcontext,void* icondata,u32 width,u32 height);

void WSetTitle(WWindowContext* windowcontext,const s8* title_string);

s8 WKeyCodeToASCII(u32 keycode);
