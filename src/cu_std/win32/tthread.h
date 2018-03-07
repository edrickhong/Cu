#pragma once
#include "ttype.h"
#include "mode.h"
#include "iintrin.h"

#include "Windows.h"

#define _threadprocreturn DWORD WINAPI



struct TThreadContext{
    HANDLE handle;
};

typedef HANDLE TSemaphore;

TThreadContext TCreateThread(s32 (*call_fptr)(void*),u32 stack_size,void* args);

TSemaphore TCreateSemaphore(u32 value = 0);

void TDestroySemaphore(TSemaphore sem);



void _ainline  TSignalSemaphore(TSemaphore sem){
    ReleaseSemaphore(sem,1,0);
}

void _ainline TWaitSemaphore(TSemaphore sem){
    WaitForSingleObject(sem,INFINITE);
}

void _ainline TWaitSemaphore(TSemaphore sem,f32 time_ms){
    WaitForSingleObject(sem,(DWORD)time_ms);
}

typedef DWORD ThreadID;

ThreadID  _ainline TGetThisThreadID(){
    return GetCurrentThreadId();
}

void _ainline TSetThreadAffinity(ThreadID threadid,u32 cpu_mask){
    
    auto handle = OpenThread(THREAD_SET_LIMITED_INFORMATION  |
                             THREAD_QUERY_LIMITED_INFORMATION ,false,threadid);
    
    _kill("failed to get handle in affinity\n",!handle);
    
    
    auto res = SetThreadAffinityMask(handle,cpu_mask);
    
    _kill("failed to set affinity\n",!res);
    
    res = CloseHandle(handle);
    
    _kill("failed to close in thread affinity\n",!res);
}


u32 TGetEntryIndex(u32* cur_index);

u32 TGetEntryIndex(u32* cur_index,u32 max_count);

#if _debug

#define TGetEntryIndex(a,b) TGetEntryIndex(a,b)

#else

#define TGetEntryIndex(a,b) TGetEntryIndex(a)

#endif