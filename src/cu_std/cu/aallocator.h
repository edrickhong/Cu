#pragma once
#include "mode.h"
#include "ttype.h"

/*
TODO: We should log and do bounds checking for our regular mallocs as well
 */

void* TAlloc(u32 size);

void InitInternalAllocator();

void InitTAlloc(u32 size);

void ResetTAlloc();

#if _debug

void* DebugMalloc(size_t size,const s8* file,const s8* function,u32 line);
void DebugFree(void* ptr);

u32 DebugDumpTotalMallocSize();
u32 DebugDumpTotalMallocCount();
void DebugDumpMallocs();
void DebugPrintMallocEntry(u32 i);


#define _enable_allocboundscheck 1
#define _enable_boundscheck_warning 0

void* DebugTAlloc(u32 size,const s8* file,const s8* function,u32 line);

void DebugDumpTAllocs();

void DebugPrintTAllocEntry(u32 i);

#if  _enable_allocboundscheck

template <class T>
struct DebugAllocedPtr{
  
  T* ptr;
  u32 forward_size;
  u32 backward_size;
  
  s8* file;
  s8* function;
  u32 line;

  DebugAllocedPtr(){}

  DebugAllocedPtr(u32 f_alloc_size,s8* file,s8* function,u32 line){
    ptr = (T*)DebugTAlloc(f_alloc_size,file,function,line);
    forward_size = f_alloc_size;
    backward_size = 0;
    this->file = file;
    this->function = function;
    this->line = line;
  }

  DebugAllocedPtr operator+ (u32 count){

    u32 add_size = (count * sizeof(T));

    _kill("over reached\n",add_size > forward_size);

    DebugAllocedPtr n_dbg_ptr = *this;

    n_dbg_ptr.ptr += count;
    n_dbg_ptr.forward_size -= add_size;
    n_dbg_ptr.backward_size += add_size;

    return n_dbg_ptr;
  }

  DebugAllocedPtr operator- (u32 count){

    u32 minus_size = (count * sizeof(T));

    _kill("over reached\n",minus_size > backward_size);

    DebugAllocedPtr n_dbg_ptr = *this;

    n_dbg_ptr.ptr -= count;
    n_dbg_ptr.forward_size += minus_size;
    n_dbg_ptr.backward_size -= minus_size;

    return n_dbg_ptr;
  }

  T& operator[] (u32 index){

    u32 add_size = (index * sizeof(T));

    _kill("over reached\n",add_size > forward_size);

    return ptr[index];
  }

  template <class A>
  operator A*(){

#if _enable_boundscheck_warning
    
    printf("WARNING: Raw ptr, no bounds checking can be done for passed ptr : %s %s %d\n",
	   file,function,line);

#endif
    
    return (A*)ptr;
  }

  T* operator-> (){
    return ptr;
  }

  T& operator* (){
    return *ptr;
  }
  
};

template <class T>
DebugAllocedPtr<T> MakeDebugPtr(u32 f_alloc_size,s8* file,s8* function,u32 line){
  DebugAllocedPtr<T> a(f_alloc_size,file,function,line);
  return a;
}

template <class T>
void memcpy(DebugAllocedPtr<T> dst,DebugAllocedPtr<T> src,u32 size){
  
  _kill("over reach\n",(size > dst.forward_size) || (size > src.forward_size));

  memcpy(dst.ptr,src.ptr,size);
}


#define TAlloc(type,count) MakeDebugPtr<type>(sizeof(type) * count,(s8*)__FILE__,(s8*)__FUNCTION__,__LINE__)


#else

#define TAlloc(type,count) (type*)DebugTAlloc(sizeof(type) * count,__FILE__,__FUNCTION__,__LINE__)

#endif

#else

#define TAlloc(type,count) (type*)TAlloc(sizeof(type) * count)


#endif


struct AAllocatorContext;

AAllocatorContext* GetAAllocatorContext();
void SetAAllocatorContext(AAllocatorContext* context);



#if _debug

#define alloc(size) DebugMalloc(size,__FILE__,__FUNCTION__,__LINE__)
#define unalloc DebugFree

#else

#define alloc malloc
#define unalloc free

#endif
