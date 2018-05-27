#pragma  once

#define _encode_rgba(r,g,b,a) ((u8(r)) | ((u8(g)) << 8) | ((u8(b)) << 16) | ((u8(a)) << 24))
#define _encode_bgra(b,g,r,a) ((u8(b)) | ((u8(g)) << 8) | ((u8(r)) << 16) | ((u8(a)) << 24))

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

#define _restrict __restrict

#define _deprecated __declspec(deprecated)

#define _optnone

#define _ainline __forceinline

#define _align(alignment) __declspec( align(alignment) )

#define _membarrier() MemoryBarrier()

#define _packed
#define _cachealign _align(64)

#define _winapi __stdcall

#define MODEL_PATH(name) "..\\..\\rsrc\\models\\"#name

#define IMAGE_PATH(name) "..\\..\\rsrc\\images\\"#name

#define AUDIO_PATH(name) "..\\..\\rsrc\\audio\\"#name

#define TEXTURE_PATH(name) "..\\..\\rsrc\\textures\\"#name

#define SHADER_PATH(name) "..\\..\\rsrc\\shaders\\"#name

#define SHADER_PATH(name) "..\\..\\rsrc\\shaders\\"#name

#define FONT_PATH(name) "C:\\Windows\\Fonts\\"#name

#define _dllexport __declspec(dllexport)

#else

#define _deprecated __attribute__ ((deprecated))

#define _FileSlash '/'

#define MODEL_PATH(name) "../rsrc/models/"#name

#define IMAGE_PATH(name) "../rsrc/images/"#name

#define AUDIO_PATH(name) "../rsrc/audio/"#name

#define TEXTURE_PATH(name) "../rsrc/textures/"#name

#define SHADER_PATH(name) "../rsrc/shaders/"#name

#define FONT_PATH(name) "/usr/share/fonts/truetype/"#name

#define _restrict __restrict__
#define _optnone __attribute__ ((optnone))
#define _ainline inline __attribute__((always_inline))
#define _align(alignment) __attribute__ ((aligned (alignment)))
#define _membarrier() asm volatile ("" : : : "memory")
#define _packed __attribute__((packed))

#define _cachealign _align(64)

#define _winapi

#define _dllexport

#endif

/* define WARN_IF(EXP) \ */
/*      do { if (EXP) \ */
/* 	 fprintf (stderr, "Warning: " #EXP "\n"); } \ */
/*      while (0) */
/*        WARN_IF (x == 0); */
/* ==> do { if (x == 0) */
/* 	fprintf (stderr, "Warning: " "x == 0" "\n"); } while (0); */

#define _COMPFILE "OBJ.comp"

#define _enable_gui 1

#define _renderbatchsize 128

#define _rendergroupcount 3

#define _usergba 0

#define _debug 1

#define _either(condition,value1,value2) ((condition) ? value1: value2)

//we do not need to free an object when it happens when our program closes
#define _ignorefree 0

#define _useasm 1

#define _internal static

#define _persist static

#define _global static

#define _countsize(size,type) size/(sizeof(type))

#define _parse
#define _reflect

#define _typebitcount(a) (sizeof(a) * 8)

#define _arraycount(ar) (sizeof(ar)/sizeof(ar[0]))

#define _in_

#define _out_

#define _optnl_

#define _unsigned_max(value) (((u64)1 << (sizeof(value) * 8)) - 1)


/*

NOTE: This calls a function that aligns the stack for it before it actually calls the 
relevant function. Getting better cache performance when using lambdas cos of this

  _lambda(u32,(u32 a,u32 b),{
  return a * b;
  })
*/
#define _lambda(ret,args,body) []args->ret body

#define _kilobytes(value) ((value) * 1024)
#define _megabytes(value) (_kilobytes(value) * 1024)
#define _gigabytes(value) (_megabytes((uint64_t)value) * 1024)
#define _terrabytes(value) (_gigabytes((uint64_t)value) * 1024)

#define _alignpow2(value,pow) ((value + (pow -1)) & (~(pow -1)))
#define _align4(value) _alignpow2(value,4)
#define _align8(value) _alignpow2(value,8)
#define _align16(value) _alignpow2(value,16)
#define _align32(value) _alignpow2(value,32)
#define _align64(value) _alignpow2(value,64)
#define _align128(value) _alignpow2(value,128)
#define _align256(value) _alignpow2(value,256)

#define _devicealign(value) _align256(value)
#define _mapalign(value) _align128(value)

//kind of like rounding down by power of 2
#define _dalignpow2(value,pow) ((value) & (~(pow -1)))
#define _dmapalign(value) _dalignpow2(value,128)

#define REFLCOMPONENT
#define IDREF(COMPONENT_STRUCT_NAME)

#if _debug

#define _disable_wayland_path 1

#define _not_renderdoc_friendly 0

#define _test_dim 64


#ifdef __clang__

#pragma GCC diagnostic warning "-Wc99-extensions"
#pragma GCC diagnostic error "-Wc99-extensions"
#pragma GCC diagnostic ignored "-Wc99-extensions"

#pragma GCC diagnostic warning "-Wnull-dereference"
#pragma GCC diagnostic error "-Wnull-dereference"
#pragma GCC diagnostic ignored "-Wnull-dereference"

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


#include "stdlib.h"
#include "stdio.h"
#include "string.h"


#define _kill(string,condition) if((condition)) {printf("%s %s %d\n",__FUNCTION__,__FILE__,__LINE__);*(int *)0 = 0;}
#define _dprint(string, ...) printf(string, __VA_ARGS__);



#define _uninitptr 0xCF

#define _alloc(type,size) ((type*)malloc(sizeof(type) * size))

#define _free(type,ptr) free(ptr); ptr = (type*)0xFEFEFEFEFEFEFEFE

#define _destroy(ptr) ptr->Destroy();free(ptr);ptr = (void*)0xFEFEFEFEFEFEFEFE;

#else

#define _kill(string,condition)
#define _dprint(string, ...)

#define _alloc(type,size) ((type*)malloc(sizeof(type) * size))

#define _destroy(ptr) {ptr->Destroy();free(ptr)}

#define _free(type,ptr)

#define _gettime(time)

#define _timediff(var,start,end)

#endif
