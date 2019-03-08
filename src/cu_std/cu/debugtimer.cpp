#include "ssys.h"
#include "tthreadx.h"
#include "gui_draw.h"

#define _max_debugtables 12000


struct PrintEntry{
    const s8* name;
    f32 time;
};


struct DebugTimerContext{
    
    DebugTable debugtable;
    
    DebugTable* global_debugtable_array;
    
    u32 global_debugtable_count;
    
    PrintEntry print_array[500];
    u32 print_count = 0;
    
    f32 frame_time = 0;
    f32 exec_time = 0;
    u32 is_paused = 0;
    
    SpinMutex locker = 0;
    
};

_persist DebugTimerContext* context = 0;

void InitDebugTimer(){
    
    context = (DebugTimerContext*)alloc(sizeof(DebugTimerContext));
    
    context->global_debugtable_array =
        (DebugTable*)SSysAlloc(0,sizeof(DebugTable) * _max_debugtables,
                               MEMPROT_READWRITE,0);
}

void* DebugTimerGetContext(){
    return context;
}

void DebugTimerSetContext(void* tcontext){
    context = (DebugTimerContext*)tcontext;
}

void PrintEntries(){
    
    qsort(context->print_array,context->print_count,sizeof(PrintEntry),
          [](const void * a, const void* b)->s32 {
          
          auto a_ptr = (PrintEntry*)a;
          auto b_ptr = (PrintEntry*)b;
          
          auto ret = (u32)((b_ptr->time - a_ptr->time) * 10000);
          
          return ret;
          });
    
    for(u32 i = 0; i < context->print_count; i++){
        auto ent = &context->print_array[i];
        
        printf("%s : %f\n",ent->name,ent->time);
    }
}

void InternalClearDebugTable(DebugTable* table){
    for(u32 i = 0; i < table->thread_count; i++){
        table->recordcount_array[i] = 0;
    }  
}

void BuildGraph(logic to_draw){
    
    static GUIVec2 pos = {-1.0f,0.898333f};
    static GUIDim2 dim = {GUIDEFAULT_W * 2.5f,GUIDEFAULT_H * 1.4f};
    
    static u32 frame_index = 0;
    
    static u32 display_profile = false;
    
    static GUIVec2 frame_array[128] = {};
    static u32 cur_frame_count = 0;
    
    if (to_draw){
        
        GUIBeginWindow("Profile View",&pos,&dim);
        
        const DebugRecord* masterblock =
            &context->debugtable.record_array[0][context->debugtable.recordcount_array[0] - 1];
        
        s8 buffer[128] = {};
        
        //FIXME: GUI cannot put things below a graph
        if(!display_profile){
            
            static f32 max_frame = 0.0f;
            
            auto highlight_frame = cur_frame_count - 1;
            
            if(GUIHistogram("Frame","Total Time",&frame_array[0],_arraycount(frame_array),
                            &frame_index,&max_frame,{0.5f,1.5f},&highlight_frame)){
                
                display_profile = true;
            }
            
            if(context->is_paused){
                
                if(GUIButton("Play")){
                    context->is_paused = false;
                }
                
            }
            
            else{
                
                if(GUIButton("Pause")){
                    context->is_paused = true;
                }
                
                
                frame_array[cur_frame_count] =
                {(f32)context->global_debugtable_count,masterblock->timelen};
                
                cur_frame_count++;
                
                if(cur_frame_count >= _arraycount(frame_array)){
                    cur_frame_count = 0;
                }
            }
            
        }
        
        else{
            
            auto table =
                &context->global_debugtable_array[(u32)frame_array[frame_index].x];
            
            if(GUIProfileView("Main Profile",table,{0.5f,1.5f})){
            }
            
            if(GUIButton("Back")){
                display_profile = false;
            }
            
        }
        
        sprintf(&buffer[0],"Frame Time:%f",masterblock->timelen);
        
        GUIString(&buffer[0]);
    }
    
    
    
    if(!context->is_paused){
        context->global_debugtable_array[context->global_debugtable_count] = context->debugtable;
        context->global_debugtable_count++;
        
        if(context->global_debugtable_count == _max_debugtables){
            context->global_debugtable_count = 0;
        }  
    }
    
    
    InternalClearDebugTable(&context->debugtable);
}


void RecordThread(){
    
    TSpinLock(&context->locker);
    
    auto threadid_ptr = &context->debugtable.threadid_array[context->debugtable.thread_count];
    
    *threadid_ptr = TGetThisThreadID();
    
    context->debugtable.thread_count = context->debugtable.thread_count + 1;
    
    printf("Tracking thread index: %d : %lu\n",context->debugtable.thread_count - 1,
           *threadid_ptr);
    
    TSpinUnlock(&context->locker);
    
}

u32 GetThreadIndex(ThreadID tid){
    
    u32 threadindex;
    
    for(threadindex = 0; threadindex < context->debugtable.thread_count; threadindex++){
        
        if(context->debugtable.threadid_array[threadindex] == tid){
            break;
        }
    }
    
    return threadindex;
}


void SubmitRecord(ThreadID tid,DebugRecord record){
    
    u32 threadindex = GetThreadIndex(tid);
    
#define _recordcount context->debugtable.recordcount_array[threadindex]
    
    context->debugtable.record_array[threadindex][ _recordcount] = record;
    _recordcount++;
    
    _kill("too many records\n", _recordcount >= _arraycount(context->debugtable.record_array[threadindex]));
    
#undef _recordcount
    
}

void SetStartTimeBlock(TimeSpec timestamp){
    context->debugtable.timestamp = timestamp;
}

void PauseTimer(){
    context->is_paused = true;
}


void SetFrameTime(f32 time){
    context->frame_time = time; 
}


void SetExecTime(f32 time){
    context->exec_time = time; 
}
