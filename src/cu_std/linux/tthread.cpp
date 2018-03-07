#include "tthread.h"
#include "libload.h"
#include "iintrin.h"
#include "aallocator.h"

#ifdef TGetEntryIndex
#undef TGetEntryIndex
#endif

void TWaitSemaphore(TSemaphore sem,f32 time_ms){
    
    timespec time = {};
    time.tv_nsec = time_ms * 1000000.0f;
    
    sem_timedwait(sem,&time); 
}

void TWaitSemaphore(TSemaphore sem){
    sem_wait(sem);
}

ThreadID TGetThisThreadID(){
    return pthread_self();
}

void TSetThreadAffinity(ThreadID threadid,u32 cpu_mask){
    
    _kill("mask of 0 used\n",!cpu_mask);
    
    u32 count = 32 - BSR(cpu_mask);
    
    cpu_set_t cpuset;
    
    CPU_ZERO(&cpuset);
    
    for(u32 i = 0; i < count; i++){
        
        if( (1 << i) & cpu_mask){
            CPU_SET(i, &cpuset);
        }
    }
    
    auto res = pthread_setaffinity_np(threadid,sizeof(cpuset),&cpuset);
    
    _kill("failed to set affinity\n",res);
    
}

TThreadContext  TCreateThread(s32(*call_fptr)(void*),u32 stack_size,void* args){
    
    TThreadContext context;
    pthread_attr_t attr;
    
    pthread_attr_init(&attr);
    
    u32 err = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    
    _kill("failed to set detached\n",err != 0);
    
    err = pthread_attr_setstacksize(&attr,stack_size);
    
    _kill("failed to set stack size\n",err != 0);
    
    err = pthread_create(&context.handle,&attr,(void *(*)(void *))call_fptr,args);
    
    pthread_attr_destroy(&attr);
    
    _kill("failed to create thread\n",err != 0);
    
    return context;
}

TSemaphore TCreateSemaphore(u32 value){
    
    TSemaphore sem = (TSemaphore)alloc(sizeof(sem_t));
    sem_init(sem,0,value);
    
    return sem;
}

void TDestroySemaphore(TSemaphore sem){
    sem_destroy(sem);
    unalloc(sem);
}

void TSignalSemaphore(TSemaphore sem){
    sem_post(sem);
}


u32 TGetEntryIndex(u32* cur_index){
    
    u32 expected_count;
    u32 actual_count;
    
    do{
        
        expected_count = *cur_index;
        
        actual_count = LockedCmpXchg(cur_index,expected_count,expected_count + 1);
        
    }while(expected_count != actual_count);
    
    return actual_count;
}

u32 TGetEntryIndex(u32* cur_index,u32 max_count){
    
    u32 expected_count;
    u32 actual_count;
    
    do{
        
        _kill("exceeded max entries\n",*cur_index >= max_count);
        
        expected_count = *cur_index;
        
        actual_count = LockedCmpXchg(cur_index,expected_count,expected_count + 1);
        
    }while(expected_count != actual_count);
    
    return actual_count;
}


/*NOTE:We will not be implementing sys_clone. it needs to be implemented fully in assembly.
When sys_clone is called, the new thread's IP will be right after the system call. The thread is not
spawned in the passed function. Instead, the passed function needs to be written to the new stack
and the new thread will jump to the new function when it encounters the ret instruction, 
jumping to the address at the top of its stack(the function address we just wrote to the stack).
Since this requires a system call, we might as well use the one provided in libc*/


// _persist LibHandle threadlib = 0;

// _persist void* pthread_setaffinity_np_ptr = 0;
// _persist void* pthread_attr_setstacksize_ptr = 0;
// _persist void* pthread_create_ptr = 0;
// _persist void* sem_init_ptr = 0;
// _persist void* sem_destroy_ptr = 0;
// _persist void* sem_timedwait_ptr = 0;
// _persist void* sem_post_ptr = 0;
// _persist void* pthread_self_ptr = 0;
// _persist void* sem_wait_ptr = 0;

// #define pthread_setaffinity_np ((int (*)(pthread_t,size_t,const cpu_set_t*))pthread_setaffinity_np_ptr)

// #define pthread_attr_setstacksize ((int (*)(pthread_attr_t*,size_t))pthread_attr_setstacksize_ptr)
// #define pthread_create ((int (*)(pthread_t*,const pthread_attr_t*,void* (*)(void*),void*))pthread_create_ptr)
// #define sem_init ((int (*)(sem_t*,int,unsigned int))sem_init_ptr)
// #define sem_destroy ((int (*)(sem_t*))sem_destroy_ptr)
// #define sem_timedwait ((int (*)(sem_t*,const struct timespec*))sem_timedwait_ptr)
// #define sem_post ((int (*)(sem_t*))sem_post_ptr)
// #define pthread_self ((pthread_t (*)(void))pthread_self_ptr)
// #define sem_wait ((int (*)(sem_t*))sem_wait_ptr)

// void LoadThreadLib(){

//   if(threadlib){
//     return;
//   }

//   const s8* lib_array[] = {
//     "libpthread.so.0",
//     "libpthread.so",
//   };

//   for(u32 i = 0; i < _arraycount(lib_array); i++){

//     threadlib = LLoadLibrary(lib_array[i]);

//     if(threadlib){
//       break;
//     }

//   }

//   _kill("cannot load thread lib\n",!threadlib);

//   pthread_setaffinity_np_ptr = LGetLibFunction(threadlib,"pthread_setaffinity_np");
//   pthread_attr_setstacksize_ptr = LGetLibFunction(threadlib,"pthread_attr_setstacksize");
//   pthread_create_ptr = LGetLibFunction(threadlib,"pthread_create");
//   sem_init_ptr = LGetLibFunction(threadlib,"sem_init");
//   sem_destroy_ptr = LGetLibFunction(threadlib,"sem_destroy");
//   sem_timedwait_ptr = LGetLibFunction(threadlib,"sem_timedwait");
//   sem_post_ptr = LGetLibFunction(threadlib,"sem_post");
//   pthread_self_ptr = LGetLibFunction(threadlib,"pthread_self");
//   sem_wait_ptr = LGetLibFunction(threadlib,"sem_wait");
// }
