#pragma  once

#include "shader_include/glsl_shared.h"


//MARK: this should be tied to shaders as well

#include "math_mode.h"

#define _enable_gui 1
#define _renderbatchsize 128
#define _rendergroupcount 3
#define _usergba 0

#ifdef CPP_PASS

#define REFL REFL
#define REFLCOMPONENT REFLCOMPONENT

#else

#define REFL
#define REFLCOMPONENT

#endif

#ifdef _WIN32


#define _FileSlash '\\'

#pragma warning(disable:4996)
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4311)
#pragma warning(disable:4302)
#pragma warning(disable:4312) // from stb_image.h
#pragma warning(disable:4577) // from cmath (Windows)
#pragma warning(disable:4530) // from xlocale (Windows)

#define MODEL_PATH(name) "..\\..\\rsrc\\models\\"#name
#define IMAGE_PATH(name) "..\\..\\rsrc\\images\\"#name
#define AUDIO_PATH(name) "..\\..\\rsrc\\audio\\"#name
#define TEXTURE_PATH(name) "..\\..\\rsrc\\textures\\"#name
#define SHADER_PATH(name) "..\\..\\rsrc\\shaders\\"#name
#define SHADER_PATH(name) "..\\..\\rsrc\\shaders\\"#name
#define FONT_PATH(name) "C:\\Windows\\Fonts\\"#name
#define _dllexport __declspec(dllexport)

#else

#define _FileSlash '/'

#define MODEL_PATH(name) "../rsrc/models/"#name
#define IMAGE_PATH(name) "../rsrc/images/"#name
#define AUDIO_PATH(name) "../rsrc/audio/"#name
#define TEXTURE_PATH(name) "../rsrc/textures/"#name
#define SHADER_PATH(name) "../rsrc/shaders/"#name
#define FONT_PATH(name) "/usr/share/fonts/truetype/"#name

#endif



#define _COMPFILE "OBJ.comp"



#ifdef DEBUG

#ifdef __clang__

#pragma GCC diagnostic warning "-Wnull-dereference"
#pragma GCC diagnostic error "-Wnull-dereference"
#pragma GCC diagnostic ignored "-Wnull-dereference"

#pragma GCC diagnostic warning "-Wc99-extensions"
#pragma GCC diagnostic error "-Wc99-extensions"
#pragma GCC diagnostic ignored "-Wc99-extensions"

#pragma GCC diagnostic warning "-Wunused-function"
#pragma GCC diagnostic error "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-function"

#pragma GCC diagnostic warning "-Wunused-variable"
#pragma GCC diagnostic error "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"

#pragma GCC diagnostic warning "-Wunused-value"
#pragma GCC diagnostic error "-Wunused-value"
#pragma GCC diagnostic ignored "-Wunused-value"

#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic error "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#pragma GCC diagnostic warning "-Wformat-security"
#pragma GCC diagnostic error "-Wformat-security"
#pragma GCC diagnostic ignored "-Wformat-security"


#pragma GCC diagnostic warning "-Wcast-align"
#pragma GCC diagnostic error "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-align"


#pragma GCC diagnostic warning "-Wshadow"
#pragma GCC diagnostic error "-Wshadow"
#pragma GCC diagnostic ignored "-Wshadow"

#pragma GCC diagnostic warning "-Wgnu-anonymous-struct"
#pragma GCC diagnostic error "-Wgnu-anonymous-struct"
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"

#pragma GCC diagnostic warning "-Wnested-anon-types"
#pragma GCC diagnostic error "-Wnested-anon-types"
#pragma GCC diagnostic ignored "-Wnested-anon-types"

#pragma GCC diagnostic warning "-Wundef"
#pragma GCC diagnostic error "-Wundef"
#pragma GCC diagnostic ignored "-Wundef"

#pragma GCC diagnostic warning "-Wignored-qualifiers"
#pragma GCC diagnostic error "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"


#pragma GCC diagnostic warning "-Wswitch"
#pragma GCC diagnostic error "-Wswitch"
#pragma GCC diagnostic ignored "-Wswitch"


#pragma GCC diagnostic warning "-Wstring-compare"
#pragma GCC diagnostic error "-Wstring-compare"
#pragma GCC diagnostic ignored "-Wstring-compare"


#pragma GCC diagnostic warning "-Wconstant-logical-operand"
#pragma GCC diagnostic error "-Wconstant-logical-operand"
#pragma GCC diagnostic ignored "-Wconstant-logical-operand"

#endif

#endif
