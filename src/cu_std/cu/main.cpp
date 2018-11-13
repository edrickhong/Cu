

#include "audio_util.cpp"

#ifdef _WIN32
#include "../win32/main.cpp"
#else
#include "../linux/main.cpp"
#endif


#include "aallocator.h"
#include "iintrin.h"
#include "aanimation.h"
#include "debugtimer.h"

//alloc
#include "aallocator.cpp"
//animation
#include "aanimation.cpp"
//debug timer
#include "debugtimer.cpp"
//math
#include "mmath.cpp"
//parse
#include "pparse.cpp"


#include "image.cpp"

//threadx
#include "tthreadx.cpp"

//vulkan
#include "vvulkan.cpp"

//gui
#include "gui_draw.cpp"