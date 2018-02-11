#pragma once

//TODO: This does not include numpad keys. Win32: stopped at VK_OEM_PERIOD

#ifdef _MSC_VER

#include "Winuser.h"

#define _stub

#define KCODE_KEY_ESC   VK_ESCAPE
#define KCODE_KEY_1   0x31
#define KCODE_KEY_2     0x32
#define KCODE_KEY_3   0x33
#define KCODE_KEY_4   0x34
#define KCODE_KEY_5	   0x35
#define KCODE_KEY_6	   0x36
#define KCODE_KEY_7	   0x37
#define KCODE_KEY_8	   0x38
#define KCODE_KEY_9	   0x39
#define KCODE_KEY_0	   0x30
#define KCODE_KEY_HYPEN	   20
#define KCODE_KEY_EQUALS	   21
#define KCODE_KEY_BACKSPACE	   VK_BACK
#define KCODE_KEY_TAB	   VK_TAB
#define KCODE_KEY_Q	   0x51
#define KCODE_KEY_W	   0x57
#define KCODE_KEY_E	   26
#define KCODE_KEY_R	   0x52
#define KCODE_KEY_T	   28
#define KCODE_KEY_Y	   29
#define KCODE_KEY_U	   30
#define KCODE_KEY_I	   31
#define KCODE_KEY_O	   0x4F
#define KCODE_KEY_P	   0x50
#define KCODE_KEY_OPENSQUAREBRACKET	   34
#define KCODE_KEY_CLOSESQUAREBRACKET	   35
#define KCODE_KEY_ENTER	   VK_RETURN
#define KCODE_KEY_LCTRL	   VK_LCONTROL
#define KCODE_KEY_A	   0x41
#define KCODE_KEY_S	   0x53
#define KCODE_KEY_D	   0x44
#define KCODE_KEY_F	   0x46
#define KCODE_KEY_G	   42
#define KCODE_KEY_H	   43
#define KCODE_KEY_J	   44
#define KCODE_KEY_K	   45
#define KCODE_KEY_L	   46
#define KCODE_KEY_SEMICOLON	   47
#define KCODE_KEY_APOSTROPHE	   48
#define KCODE_KEY_BACKQUOTE	   49
#define KCODE_KEY_LSHIFT	   VK_LSHIFT
#define KCODE_KEY_BACKSLASH	   51
#define KCODE_KEY_Z	   52
#define KCODE_KEY_X	   0x58
#define KCODE_KEY_C	   0x43
#define KCODE_KEY_V	   55
#define KCODE_KEY_B	   0x42
#define KCODE_KEY_N	   57
#define KCODE_KEY_M	   58
#define KCODE_KEY_COMMA	   VK_OEM_COMMA
#define KCODE_KEY_PERIOD	   VK_OEM_PERIOD
#define KCODE_KEY_SLASH	   61
#define KCODE_KEY_RSHIFT	   VK_RSHIFT
#define KCODE_KEY_ASTERISK   63
#define KCODE_KEY_LALT	   64
#define KCODE_KEY_SPACE    VK_SPACE
#define KCODE_KEY_CAPSLOCK	   VK_CAPITAL
#define KCODE_KEY_F1	   VK_F1
#define KCODE_KEY_F2	   VK_F2
#define KCODE_KEY_F3	   VK_F3
#define KCODE_KEY_F4	   VK_F4
#define KCODE_KEY_F5	   VK_F5
#define KCODE_KEY_F6	   VK_F6
#define KCODE_KEY_F7	   VK_F7
#define KCODE_KEY_F8	   VK_F8
#define KCODE_KEY_F9	   VK_F9
#define KCODE_KEY_F10	   VK_F10
#define KCODE_KEY_NUMLOCK	   VK_NUMLOCK
#define KCODE_KEY_SCROLLLOCK	   VK_SCROLL
#define KCODE_KEY_HOME7	   79
#define KCODE_KEY_UP8	   80
#define KCODE_KEY_PGUP9	   81
#define KCODE_KEY_HYPHENMINUS   VK_OEM_MINUS
#define KCODE_KEY_LEFT4   83
//#define KCODE_KEY_5   84
#define KCODE_KEY_RTARROW   85
#define KCODE_KEY_PLUS	   VK_OEM_PLUS
#define KCODE_KEY_END1	   87
#define KCODE_KEY_DOWN2	   88
#define KCODE_KEY_PGDN3	   89
#define KCODE_KEY_INS	   90
//#define KCODE_KEY_DEL     91
/* /\* #define KCODE_KEY_84						 *\/ */
/* /\* #define KCODE_KEY_85						 *\/ */
/* /\* #define KCODE_KEY_86						 *\/ */
#define KCODE_KEY_F11	   VK_F11
#define KCODE_KEY_F12     VK_F12
/* /\* #define KCODE_KEY_89						 *\/ */
/* /\* #define KCODE_KEY_90						 *\/ */
/* /\* #define KCODE_KEY_91						 *\/ */  
/* /\* #define KCODE_KEY_92						 *\/ */   
/* /\* #define KCODE_KEY_93						 *\/ */   
/* /\* #define KCODE_KEY_94						 *\/ */   
/* /\* #define KCODE_KEY_95						 *\/ */  
#define KCODE_KEY_RENTER	   104
#define KCODE_KEY_RCTRL	   VK_RCONTROL
#define KCODE_KEY_SLASHMARK	   106
#define KCODE_KEY_PRTSCR	   107
#define KCODE_KEY_RALT    108
/* #define KCODE_KEY_101 */
#define KCODE_KEY_HOME	   VK_HOME
#define KCODE_KEY_UP	   VK_UP
#define KCODE_KEY_PGUP	   VK_PRIOR
#define KCODE_KEY_LEFT	   VK_LEFT
#define KCODE_KEY_RIGHT	   VK_RIGHT
#define KCODE_KEY_END	   VK_END
#define KCODE_KEY_DOWN	   VK_DOWN
#define KCODE_KEY_PGDN	   VK_NEXT
#define KCODE_KEY_INSERT	   VK_INSERT
#define KCODE_KEY_DEL     VK_DELETE
/* #define KCODE_KEY_112						 */  
/* #define KCODE_KEY_113						 */  
/* #define KCODE_KEY_114						 */  
/* #define KCODE_KEY_115						 */  
/* #define KCODE_KEY_116						 */  
/* #define KCODE_KEY_117						 */  
/* #define KCODE_KEY_118						 */  
#define KCODE_KEY_PAUSE   VK_PAUSE

#undef _stub

#else

#define KCODE_KEY_ESC   9
#define KCODE_KEY_1   10
#define KCODE_KEY_2     11
#define KCODE_KEY_3   12
#define KCODE_KEY_4   13
#define KCODE_KEY_5	   14
#define KCODE_KEY_6	   15
#define KCODE_KEY_7	   16
#define KCODE_KEY_8	   17
#define KCODE_KEY_9	   18
#define KCODE_KEY_0	   19
#define KCODE_KEY_HYPEN	   20
#define KCODE_KEY_EQUALS	   21
#define KCODE_KEY_BACKSPACE	   22
#define KCODE_KEY_TAB	   23
#define KCODE_KEY_Q	   24
#define KCODE_KEY_W	   25
#define KCODE_KEY_E	   26
#define KCODE_KEY_R	   27
#define KCODE_KEY_T	   28
#define KCODE_KEY_Y	   29
#define KCODE_KEY_U	   30
#define KCODE_KEY_I	   31
#define KCODE_KEY_O	   32
#define KCODE_KEY_P	   33
#define KCODE_KEY_OPENSQUAREBRACKET	   34
#define KCODE_KEY_CLOSESQUAREBRACKET	   35
#define KCODE_KEY_ENTER	   36
#define KCODE_KEY_LCTRL	   37
#define KCODE_KEY_A	   38
#define KCODE_KEY_S	   39
#define KCODE_KEY_D	   40
#define KCODE_KEY_F	   41
#define KCODE_KEY_G	   42
#define KCODE_KEY_H	   43
#define KCODE_KEY_J	   44
#define KCODE_KEY_K	   45
#define KCODE_KEY_L	   46
#define KCODE_KEY_SEMICOLON	   47
#define KCODE_KEY_APOSTROPHE	   48
#define KCODE_KEY_BACKQUOTE	   49
#define KCODE_KEY_LSHIFT	   50
#define KCODE_KEY_BACKSLASH	   51
#define KCODE_KEY_Z	   52
#define KCODE_KEY_X	   53
#define KCODE_KEY_C	   54
#define KCODE_KEY_V	   55
#define KCODE_KEY_B	   56
#define KCODE_KEY_N	   57
#define KCODE_KEY_M	   58
#define KCODE_KEY_COMMA	   59
#define KCODE_KEY_PERIOD	   60
#define KCODE_KEY_SLASH	   61
#define KCODE_KEY_RSHIFT	   62
#define KCODE_KEY_ASTERISK   63
#define KCODE_KEY_LALT	   64
#define KCODE_KEY_SPACE    65
#define KCODE_KEY_CAPSLOCK	   66
#define KCODE_KEY_F1	   67
#define KCODE_KEY_F2	   68
#define KCODE_KEY_F3	   69
#define KCODE_KEY_F4	   70
#define KCODE_KEY_F5	   71
#define KCODE_KEY_F6	   72
#define KCODE_KEY_F7	   73
#define KCODE_KEY_F8	   74
#define KCODE_KEY_F9	   75
#define KCODE_KEY_F10	   76
#define KCODE_KEY_NUMLOCK	   77
#define KCODE_KEY_SCROLLLOCK	   78
#define KCODE_KEY_HOME7	   79
#define KCODE_KEY_UP8	   80
#define KCODE_KEY_PGUP9	   81
#define KCODE_KEY_HYPHENMINUS   82
#define KCODE_KEY_LEFT4   83
//#define KCODE_KEY_5   84
#define KCODE_KEY_RTARROW   85
#define KCODE_KEY_PLUS	   86
#define KCODE_KEY_END1	   87
#define KCODE_KEY_DOWN2	   88
#define KCODE_KEY_PGDN3	   89
#define KCODE_KEY_INS	   90
//#define KCODE_KEY_DEL     91
/* /\* #define KCODE_KEY_84						 *\/ */
/* /\* #define KCODE_KEY_85						 *\/ */
/* /\* #define KCODE_KEY_86						 *\/ */
#define KCODE_KEY_F11	   95
#define KCODE_KEY_F12     96
/* /\* #define KCODE_KEY_89						 *\/ */
/* /\* #define KCODE_KEY_90						 *\/ */
/* /\* #define KCODE_KEY_91						 *\/ */  
/* /\* #define KCODE_KEY_92						 *\/ */   
/* /\* #define KCODE_KEY_93						 *\/ */   
/* /\* #define KCODE_KEY_94						 *\/ */   
/* /\* #define KCODE_KEY_95						 *\/ */  
#define KCODE_KEY_RENTER	   104
#define KCODE_KEY_RCTRL	   105
#define KCODE_KEY_SLASHMARK	   106
#define KCODE_KEY_PRTSCR	   107
#define KCODE_KEY_RALT    108
/* #define KCODE_KEY_101 */
#define KCODE_KEY_HOME	   110
#define KCODE_KEY_UP	   111
#define KCODE_KEY_PGUP	   112
#define KCODE_KEY_LEFT	   113
#define KCODE_KEY_RIGHT	   114
#define KCODE_KEY_END	   115
#define KCODE_KEY_DOWN	   116
#define KCODE_KEY_PGDN	   117
#define KCODE_KEY_INSERT	   118
#define KCODE_KEY_DEL     119
/* #define KCODE_KEY_112						 */  
/* #define KCODE_KEY_113						 */  
/* #define KCODE_KEY_114						 */  
/* #define KCODE_KEY_115						 */  
/* #define KCODE_KEY_116						 */  
/* #define KCODE_KEY_117						 */  
/* #define KCODE_KEY_118						 */  
#define KCODE_KEY_PAUSE   127

#endif
