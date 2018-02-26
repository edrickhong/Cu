#include "ssys.h"
#include "tthreadx.h"
#include "gui_draw.h"

_persist DebugTable debugtable = {};


#define _max_debugtables 12000

_persist auto  global_debugtable_array =
(DebugTable*)SSysAlloc(0,sizeof(DebugTable) * _max_debugtables,
                       MEMPROT_READWRITE,0);

_persist auto  global_debugtable_count = 0;

struct PrintEntry{
    const s8* name;
    f32 time;
};

_persist PrintEntry print_array[500];
_persist u32 print_count = 0;

void PrintEntries(){
    
    qsort(print_array,print_count,sizeof(PrintEntry),
          [](const void * a, const void* b)->s32 {
          
          auto a_ptr = (PrintEntry*)a;
          auto b_ptr = (PrintEntry*)b;
          
          auto ret = (u32)((b_ptr->time - a_ptr->time) * 10000);
          
          return ret;
          });
    
    for(u32 i = 0; i < print_count; i++){
        auto ent = &print_array[i];
        
        printf("%s : %f\n",ent->name,ent->time);
    }
}

void InternalClearDebugTable(DebugTable* table){
    for(u32 i = 0; i < table->thread_count; i++){
        table->recordcount_array[i] = 0;
    }  
}

_persist f32 frame_time = 0;
_persist f32 exec_time = 0;
_persist u32 is_paused = 0;

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
            &debugtable.record_array[0][debugtable.recordcount_array[0] - 1];
        
        s8 buffer[128] = {};
        
        //FIXME: GUI cannot put things below a graph
        if(!display_profile){
            
            static f32 max_frame = 0.0f;
            
            auto highlight_frame = cur_frame_count - 1;
            
            if(GUIHistogram("Frame","Total Time",&frame_array[0],_arraycount(frame_array),
                            &frame_index,&max_frame,{0.5f,1.5f},&highlight_frame)){
                
                display_profile = true;
            }
            
            if(is_paused){
                
                if(GUIButton("Play")){
                    is_paused = false;
                }
                
            }
            
            else{
                
                if(GUIButton("Pause")){
                    is_paused = true;
                }
                
                
                frame_array[cur_frame_count] =
                {(f32)global_debugtable_count,masterblock->timelen};
                
                cur_frame_count++;
                
                if(cur_frame_count >= _arraycount(frame_array)){
                    cur_frame_count = 0;
                }
            }
            
        }
        
        else{
            
            auto table =
                &global_debugtable_array[(u32)frame_array[frame_index].x];
            
            if(GUIProfileView("Main Profile",table,{0.5f,1.5f})){
            }
            
            if(GUIButton("Back")){
                display_profile = false;
            }
            
        }
        
        sprintf(&buffer[0],"Frame Time:%f",masterblock->timelen);
        
        GUIString(&buffer[0]);
    }
    
    
    
    if(!is_paused){
        global_debugtable_array[global_debugtable_count] = debugtable;
        global_debugtable_count++;
        
        if(global_debugtable_count == _max_debugtables){
            global_debugtable_count = 0;
        }  
    }
    
    
    InternalClearDebugTable(&debugtable);
}

_persist SpinMutex locker = 0;


void RecordThread(){
    
    SpinLock(&locker);
    
    auto threadid_ptr = &debugtable.threadid_array[debugtable.thread_count];
    
    *threadid_ptr = TGetThisThreadID();
    
    debugtable.thread_count = debugtable.thread_count + 1;
    
    printf("Tracking thread index: %d : %lu\n",debugtable.thread_count - 1,
           *threadid_ptr);
    
    SpinUnlock(&locker);
    
}

u32 GetThreadIndex(ThreadID tid){
    
    u32 threadindex;
    
    for(threadindex = 0; threadindex < debugtable.thread_count; threadindex++){
        
        if(debugtable.threadid_array[threadindex] == tid){
            break;
        }
    }
    
    return threadindex;
}


void SubmitRecord(ThreadID tid,DebugRecord record){
    
    u32 threadindex = GetThreadIndex(tid);
    
#define _recordcount debugtable.recordcount_array[threadindex]
    
    debugtable.record_array[threadindex][ _recordcount] = record;
    _recordcount++;
    
    _kill("too many records\n", _recordcount >= _arraycount(debugtable.record_array[threadindex]));
    
#undef _recordcount
    
}

void SetStartTimeBlock(TimeSpec timestamp){
    debugtable.timestamp = timestamp;
}

void PauseTimer(){
    is_paused = true;
}


void SetFrameTime(f32 time){
    frame_time = time; 
}


void SetExecTime(f32 time){
    exec_time = time; 
}
