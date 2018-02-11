#include "iintrin.h"


#ifdef TAlloc
#undef TAlloc
#endif

struct AAllocatorContext{
  
  s8* frame_ptr = 0;
  volatile u32 curframe_count = 0;

#if _debug
  
  struct DebugAllocEntry{
    void* ptr;
    ThreadID tid;
    u32 size;
    const s8* file;
    const s8* function;
    u32 line;
  };

  u32 maxframe_count = 0;

  DebugAllocEntry alloc_array[1024] = {};
  u32 alloc_count = 0;

  DebugAllocEntry malloc_array[1024] = {};
  u32 malloc_count = 0;
  
#endif
};

_persist AAllocatorContext* context = 0;

#if _debug

void DebugSubmitTAlloc(void* base_ptr,u32 size,const s8* file,const s8* function,u32 line){
  
  u32 expected_count;
  u32 actual_count;

  do{

    _kill("too many allocs\n",context->alloc_count >= _arraycount( context->alloc_array));

    expected_count = context->alloc_count;

    actual_count = LockedCmpXchg(&context->alloc_count,expected_count,expected_count + 1);
    
  }while(expected_count != actual_count);

  context->alloc_array[actual_count] = {base_ptr,TGetThisThreadID(),size,file,function,line};
  
}

void DebugDumpTAllocs(){
  
  for(u32 i = 0; i < context->alloc_count; i++){
    auto entry = context->alloc_array[i];
    
    printf("%d %d %s %s %d: base_ptr %p %d\n",i,(u32)entry.tid,entry.file,entry.function,
	   entry.line,entry.ptr,entry.size);
  }
  
}

void DebugPrintTAllocEntry(u32 i){

  auto entry = context->alloc_array[i];
  
  printf("%d %d %s %s %d: base_ptr %p %d\n",i,(u32)entry.tid,entry.file,entry.function,
	 entry.line,entry.ptr,entry.size);
}


void DebugSubmitMalloc(void* base_ptr,u32 size,const s8* file,const s8* function,u32 line){

  if(!context){
    return;
  }
  
  u32 expected_count;
  u32 actual_count;

  do{

    _kill("too many mallocs\n",context->malloc_count >= _arraycount( context->malloc_array));

    expected_count = context->malloc_count;

    actual_count = LockedCmpXchg(&context->malloc_count,expected_count,
				 expected_count + 1);
    
  }while(expected_count != actual_count);

  context->malloc_array[actual_count] = {base_ptr,TGetThisThreadID(),size,file,function,line};
  
}

void DebugRemoveMalloc(void* base_ptr){

  if(!context){
    return;
  }

  for(u32 i = 0; i < context->malloc_count; i++){

    auto entry = &context->malloc_array[i];
    
    if(entry->ptr == base_ptr){
      entry->ptr = 0;
      return;
    }
    
  }

  _kill("invalid free\n",1);
}

u32 DebugDumpTotalMallocSize(){

  if(!context){
    return (u32)-1;
  }
  
  u32 size = 0;

  for(u32 i = 0; i < context->malloc_count; i++){

    auto entry = &context->malloc_array[i];
    
    if(entry->ptr){
      size += entry->size;
    }
    
  }

  return size;
}
u32 DebugDumpTotalMallocCount(){
  
  if(!context){
    return (u32)-1;
  }
  return context->malloc_count;
}

void DebugDumpMallocs(){

  if(!context){
    return;
  }
  
  for(u32 i = 0; i < context->malloc_count; i++){
    auto entry = context->malloc_array[i];
    
    printf("%d %d %s %s %d: base_ptr %p %d\n",i,(u32)entry.tid,entry.file,entry.function,
	   entry.line,entry.ptr,entry.size);
  }
  
}

void DebugPrintMallocEntry(u32 i){

  if(!context){
    return;
  }

  auto entry = context->malloc_array[i];
  
  printf("%d %d %s %s %d: base_ptr %p %d\n",i,(u32)entry.tid,entry.file,entry.function,
	 entry.line,entry.ptr,entry.size);
}

#endif


//FIXME: sometimes TAlloc resubmits an already allocated pointer
void* TAlloc(u32 size){
    
  size = _align16(size);
    
  u32 expected_curframe_count;
  u32 actual_curframe_count;
  u32 new_curframe_count;
  void* ptr;

  do{
    
    expected_curframe_count = context->curframe_count;
    new_curframe_count = context->curframe_count + size;

    actual_curframe_count = LockedCmpXchg(&context->curframe_count,expected_curframe_count,
					  new_curframe_count);

    ptr = context->frame_ptr + expected_curframe_count;
    
  }while(expected_curframe_count != actual_curframe_count);

  _kill("no more frame stack space\n",((s8*)ptr - context->frame_ptr) > context->maxframe_count);
  
  return ptr;
}

void* DebugTAlloc(u32 size,const s8* file,const s8* function,u32 line){

  auto ptr = TAlloc(size);

  DebugSubmitTAlloc(ptr,size,file,function,line);

  return ptr;
}


void Cu_InitInternalAllocator(u32 size){

  _kill("already allocated\n",context);
  
  context = (AAllocatorContext*)alloc(sizeof(AAllocatorContext));
  memset(context,0,sizeof(AAllocatorContext));
  
  context->frame_ptr = (s8*)alloc(size);
  context->maxframe_count = size;
}

void ResetTAlloc(){
  context->curframe_count = 0;

#if _debug
  context->alloc_count = 0;
#endif
}


AAllocatorContext* GetAAllocatorContext(){
  return context;
}
void SetAAllocatorContext(AAllocatorContext* this_context){
  context = this_context;
}

void* DebugMalloc(size_t size,const s8* file,const s8* function,u32 line){
  auto ptr = malloc(size);
  DebugSubmitMalloc(ptr,size,file,function,line);
  return ptr;
}
void DebugFree(void* ptr){
  DebugRemoveMalloc(ptr);
  free(ptr);
}
