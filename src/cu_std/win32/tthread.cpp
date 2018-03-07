
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