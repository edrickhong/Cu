

#ifdef TGetEntryIndexD
#undef TGetEntryIndex
#endif

void ThreadPushStack(ThreadWorkStack* stack,WorkProc proc,void* data){
    
    stack->buffer[stack->count].workcall = proc;
    stack->buffer[stack->count].data = data;
    
    LockedIncrement(&stack->count);
}

logic ThreadPopStack(ThreadWorkStack* stack,void* thread_data){
    
    u32 count = stack->count;
    
    if(!count){
        return false;
    }
    
    TIMEBLOCK(Orange);
    
    u32 actual_count = LockedCmpXchg(&stack->count,count,count -1);
    
    if(actual_count == count){
        
        u32 index = count - 1;
        
        ThreadWorkEntry entry = {stack->buffer[index].workcall,stack->buffer[index].data};
        
        _kill("work entry contains no call. we should be worried and start tracing\n",!entry.workcall);
        
        entry.workcall(entry.data,thread_data);
        
        if(index == 0){
            stack->completed = 1;
        }
        
    }
    
    return true;
}

void ThreadSubmit(ThreadWorkStack* stack,TSemaphore sem){
    
    for(u32 i = 0; i < stack->count;i++){
        TSignalSemaphore(sem);  
    }
    
}


void ThreadSubmit(TSemaphore sem,u32 threads){
    
    for(u32 i = 0; i < threads;i++){
        TSignalSemaphore(sem);  
    }
    
}

void ThreadPushPath(ThreadWorkPath* path,WorkProc proc,void* data){
    
    path->buffer[path->count].workcall = proc;
    path->buffer[path->count].data = data;
    
    LockedIncrement(&path->count);
}

void ThreadExecutePath(ThreadWorkPath* path,void* thread_data){
    
    TIMEBLOCK(Orange);
    
    LockedIncrement(&path->enter_count);
    
    for(u32 i = 0; i < path->count;i++){
        ThreadWorkEntry entry = {path->buffer[i].workcall,path->buffer[i].data};
        entry.workcall(entry.data,thread_data);
    }
    
    
    LockedIncrement(&path->exit_count);
    
    //make sure that all threads have left(aka submitted) before we can say that the work is done.
    if(path->enter_count == path->exit_count){
        path->complete = true;
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
    
    if(index == queue->count){
        return false;
    }
    
    TIMEBLOCK(Magenta);
    
    u32 actual_index = LockedCmpXchg(&queue->index,index,index + 1);
    
    if(actual_index == index){
        
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
    
    return actual_count;
}