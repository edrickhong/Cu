

#ifdef TGetEntryIndexD
#undef TGetEntryIndex
#endif

void ThreadSubmit(TSemaphore sem,u32 threads){
    
    for(u32 i = 0; i < threads;i++){
        TSignalSemaphore(sem);  
    }
    
}


void PushThreadWorkQueue(ThreadWorkQueue* queue,WorkProc proc,void* data,
                         TSemaphore sem){
    
    queue->buffer[queue->count].workcall = proc;
    queue->buffer[queue->count].data = data;
    queue->count++;
    TSignalSemaphore(sem);
}

logic ExecuteThreadWorkQueue(ThreadWorkQueue* queue,void* thread_data){
    
    
    u32 index = queue->index;
    
    if(index >= queue->count){
        return false;
    }
    
    TIMEBLOCK(Magenta);
    
    u32 actual_index = LockedCmpXchg(&queue->index,index,index + 1);
    
    if(actual_index == index && index < queue->count){
        
        _kill("work entry contains no call. we should be worried and start tracing\n",
              !queue->buffer[index].workcall);
        
        queue->buffer[index].workcall(queue->buffer[index].data,thread_data);
        
        LockedIncrement(&queue->completed_count);
        
    }
    
    return true;
}





u32 TGetEntryIndex(volatile u32* cur_index){
    
    u32 expected_count;
    u32 actual_count;
    
    do{
        
        expected_count = *cur_index;
        
        actual_count = LockedCmpXchg(cur_index,expected_count,expected_count + 1);
        
    }while(expected_count != actual_count);
    
    return actual_count;
}

u32 TGetEntryIndex(volatile u32* cur_index,u32 max_count){
    
    u32 expected_count;
    u32 actual_count;
    
    do{
        
        _kill("exceeded max entries\n",*cur_index >= max_count);
        
        expected_count = *cur_index;
        
        actual_count = LockedCmpXchg(cur_index,expected_count,expected_count + 1);
        
    }while(expected_count != actual_count);
    
    _kill("exceeded max entries\n",actual_count >= max_count);
    
    return actual_count;
}

void TSingleEntryLock(EntryMutex* mutex,WorkProc proc,void* args,void* threadcontext){
    
    if((*mutex)){
        return;
    }
    
    auto is_locked = (*mutex);
    
    u32 actual_islocked = LockedCmpXchg(mutex,is_locked,is_locked + 1);
    
    if((is_locked != actual_islocked) || is_locked){
        return;
    }
    
    proc(args,threadcontext);
}