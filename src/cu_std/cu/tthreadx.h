#pragma once
#include "iintrin.h"
#include "debugtimer.h"

typedef void(*WorkProc)(void*,void*); // function args, threadcontext

typedef volatile u32 SpinMutex;

struct ThreadWorkEntry{
  WorkProc workcall;
  void* data;
};

struct ThreadWorkQueue{
  volatile ThreadWorkEntry buffer[40];
  volatile  _cachealign u32 count = 0;
  volatile  _cachealign u32 index = 0;
  volatile  _cachealign u32 completed_count = 0;
};

struct ThreadWorkStack{
  volatile ThreadWorkEntry buffer[40];
  volatile  _cachealign u32 count = 0;
  volatile  _cachealign u32 completed = 0;
};

//NOTE: Should we cache align these?? MARK:
struct ThreadWorkPath{

  //only written by main thread
  volatile ThreadWorkEntry buffer[40];
  volatile u32 count = 0;

  volatile _cachealign u32 enter_count = 0;
  volatile _cachealign u32 exit_count = 0;
  volatile _cachealign u32 complete = 1;
};

void _ainline Clear(ThreadWorkPath* path){
  path->count = 0;
}

void _ainline Clear(ThreadWorkQueue* queue){
  queue->count = 0;
  queue->index = 0;
  queue->completed_count = 0;
}


void ThreadPushStack(ThreadWorkStack* stack,WorkProc proc,void* data);

logic ThreadPopStack(ThreadWorkStack* stack,void* thread_data);


void ThreadSubmit(ThreadWorkStack* stack,TSemaphore sem);

void ThreadSubmit(ThreadWorkPath* path,TSemaphore sem,u32 threads);

void ThreadPushPath(ThreadWorkPath* path,WorkProc proc,void* data);

void ThreadExecutePath(ThreadWorkPath* path,void* thread_data);


void inline MainThreadDoWorkStack(ThreadWorkStack* stack,void* thread_data){

  TIMEBLOCK(Red);

  while(!stack->completed){
    ThreadPopStack(stack,thread_data);
  }

  stack->completed = 0;
}

void PushThreadWorkQueue(ThreadWorkQueue* queue,WorkProc proc,void* data,
			 TSemaphore sem);

logic ExecuteThreadWorkQueue(ThreadWorkQueue* queue,void* thread_data);

void inline MainThreadDoWorkQueue(ThreadWorkQueue* queue,void* thread_data){

  TIMEBLOCK(Green);
  
  while(queue->completed_count != queue->count){
    ExecuteThreadWorkQueue(queue,thread_data);
  }
  
}



void inline MainThreadExecutePath(ThreadWorkPath* path,void* thread_data){

  TIMEBLOCK(Green);
  
  ThreadExecutePath(path,thread_data);

  while(!path->complete){
    _mm_pause();
  }
  
}

void _ainline ThreadReadyPath(ThreadWorkPath* path){
  path->enter_count = 0;
  path->exit_count = 0;
  path->complete = 0;
}


void _ainline SpinLock(SpinMutex* mutex){

  TIMEBLOCK(LimeGreen);

 SpinLock_tryagain:  

  u32 value = LockedCmpXchg(mutex,0,1);

  if(value){
    
    while(*mutex){
      _mm_pause();
    }

    goto SpinLock_tryagain;
  }
  
  

  *mutex = 1;
}

void _ainline SpinUnlock(SpinMutex* mutex){
  *mutex = 0;
}
