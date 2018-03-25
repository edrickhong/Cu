#pragma once
#include "ttype.h"
#include "mode.h"
#include "pthread.h"
#include "semaphore.h"

struct TThreadContext{
    pthread_t handle;
};

typedef sem_t* TSemaphore;

TThreadContext TCreateThread(s32(*call_fptr)(void*),u32 stack_size,void* args);

TSemaphore TCreateSemaphore(u32 value = 0);

void TDestroySemaphore(TSemaphore sem);

void TSignalSemaphore(TSemaphore sem);

void TWaitSemaphore(TSemaphore sem);

void TWaitSemaphore(TSemaphore sem,f32 time_ms);

typedef pthread_t ThreadID;

ThreadID TGetThisThreadID();

void TSetThreadAffinity(ThreadID threadid,u32 cpu_mask);