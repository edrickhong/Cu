
#include "wwindow.h"
#include "string.h"

#include "Windowsx.h"


_persist WWindowEvent event_array[10];
_persist u32 event_count = 0;

void _ainline PostEvent(WWindowEvent event){

  _kill("msg stack overflow\n",event_count > _arraycount(event_array));
  
  event_array[event_count] = event;
  event_count++;
}

#define _WIN32_DOWN_BIT (1 << 30)

LRESULT CALLBACK WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
  
  LRESULT result = 0;

  switch (uMsg){
  case WM_SIZE:
    {

    } break;

  case WM_DESTROY:
    {
      PostEvent({W_EVENT_CLOSE});
      PostQuitMessage(0);
    } break;

  case WM_CLOSE:
    {
      PostEvent({W_EVENT_CLOSE});
      PostQuitMessage(0);
    } break;

  case WM_ACTIVATEAPP:{
    PostEvent({(WEventType)uMsg});
  }break;

  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
    {
      auto vcode = (u32)wParam;

      if(!(_WIN32_DOWN_BIT & lParam)){
	WWindowEvent event = {};
	event.type = W_EVENT_KBEVENT_KEYDOWN;
	event.keyboard_event.keycode = vcode;
	PostEvent(event);
      }
      
    }break;
  case WM_KEYUP:{

    auto vcode = (u32)wParam;

    if((_WIN32_DOWN_BIT & lParam)){
      WWindowEvent event = {};
      event.type = W_EVENT_KBEVENT_KEYUP;
      event.keyboard_event.keycode = vcode;
      PostEvent(event);
    }
    
  }break;

  case WM_MOUSEMOVE:{
    WWindowEvent event = {};
    event.type = W_EVENT_MSEVENT_MOVE;
    event.mouse_event.x = GET_X_LPARAM(lParam);
    event.mouse_event.y = GET_Y_LPARAM(lParam);
    PostEvent(event);
  }break;

  case WM_LBUTTONDOWN:{
    WWindowEvent event = {};
    event.type = W_EVENT_MSEVENT_DOWN;
    event.mouse_event.keycode = MOUSEBUTTON_LEFT;
    PostEvent(event);
  }break;

  case WM_LBUTTONUP:{
    WWindowEvent event = {};
    event.type = W_EVENT_MSEVENT_UP;
    event.mouse_event.keycode = MOUSEBUTTON_LEFT;
    PostEvent(event);
  }break;

  case WM_RBUTTONDOWN: {
	  WWindowEvent event = {};
	  event.type = W_EVENT_MSEVENT_DOWN;
	  event.mouse_event.keycode = MOUSEBUTTON_RIGHT;
	  PostEvent(event);
  }break;

  case WM_RBUTTONUP: {
	  WWindowEvent event = {};
	  event.type = W_EVENT_MSEVENT_UP;
	  event.mouse_event.keycode = MOUSEBUTTON_RIGHT;
	  PostEvent(event);
  }break;

  default:
    {
      result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    } break;
  }

  return result;
}

WWindowContext WCreateWindow(const s8* title,WCreateFlags flags,u32 x,u32 y,u32 width,
			     u32 height){
  
  WWindowContext context;

  context.width = width;
  context.height = height;

  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 0,
		    &context.handle);

  WNDCLASSEX wndclass = {};

  wndclass.cbSize = sizeof(WNDCLASSEX);
  wndclass.style = flags;
  wndclass.lpfnWndProc = WindowCallback;
  wndclass.hInstance = context.handle;
  wndclass.lpszClassName = "WIN32WNDCLASSEX";
  wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

  auto res = RegisterClassEx(&wndclass);

  auto style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

  if(W_CREATE_NORESIZE & flags){
    style ^= (WS_MAXIMIZEBOX | WS_THICKFRAME);
  }

  
  context.rootwindow =
    CreateWindow(wndclass.lpszClassName,title,style, x, y,
		 width, height, 0, 0, context.handle, 0);

  _kill("Failed to register WNDCLASS", !res);
  _kill("Unable to create window", !(context.rootwindow));

  return context;
}

u32 WWaitForWindowEvent(WWindowContext* windowcontext,WWindowEvent* event){

  MSG msg;

  auto ret = event_count;

  while(PeekMessage(&msg,windowcontext->rootwindow,0,0,PM_REMOVE | PM_NOYIELD) > 0){
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  if(ret){
    event_count --;
    *event = event_array[event_count];
  }

  return ret;
}


void WSetIcon(WWindowContext windowcontext,void* icondata,u32 width,u32 height){}

void WSetTitle(WWindowContext windowcontext,const s8* title_string){}


s8 WKeyCodeToASCII(u32 keycode){

  auto scancode = MapVirtualKey(keycode,MAPVK_VSC_TO_VK);

  WORD ascii_char[2];
  BYTE keyboardstate[256];

  auto error = GetKeyboardState(&keyboardstate[0]);

  _kill("failed to get keyboard state\n",!error);

  ToAscii(keycode,scancode,&keyboardstate[0],&ascii_char[0],0);
  
  return ascii_char[0];
}

