
#include "tthread.h"
#include "aallocator.h"

struct InternalArgs{
  s32 (*threadproc)(void*);
  void* args;
};

DWORD WINAPI InternalThreadProc(void* args){
  
  auto internal = *((InternalArgs*)args);

  unalloc(args);

  internal.threadproc(internal.args);

  return 0;
}

TThreadContext TCreateThread(s32 (*call_fptr)(void*),u32 stack_size,void* args){

  auto intargs = (InternalArgs*)alloc(sizeof(InternalArgs));
  *intargs = {call_fptr,args};

  return {CreateThread(0,stack_size,InternalThreadProc,intargs,0,0)};
}

TSemaphore TCreateSemaphore(u32 value){
  return CreateSemaphore(0,value,value + 1000,0);
}
