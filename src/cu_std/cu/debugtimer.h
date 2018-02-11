#pragma once

#include "mode.h"
#include "ttimer.h"

//MARK:
#include "tthread.h"

#include "ccolor.h"

#if _debug

#define _enable_debugtimer 1

#else

#define _enable_debugtimer 0
#endif

#if (_debug && _enable_debugtimer)

#define TIMEBLOCK(COLOR) TimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)__FUNCTION__,COLOR)

#define TIMEBLOCKTAGGED(NAME,COLOR) TimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)NAME,COLOR)

#define MASTERTIMEBLOCKSTART(COLOR) MasterTimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)"MASTERTIMEBLOCK",COLOR)

#define EXECTIMEBLOCK(COLOR) ExecTimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)"EXECBLOCK",COLOR)

#define RECORDTHREAD() RecordThread()

#define BUILDGUIGRAPH(to_draw) BuildGraph(to_draw);

#define PRINTBLOCKS() PrintEntries()

#define PRINTTIMEBLOCK() PrintTimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)__FUNCTION__,White)

#define PRINTTIMEBLOCKTAGGED(NAME) PrintTimeBlock t_##__LINE__((const s8*)__FILE__,__LINE__,(const s8*)NAME,White)

#define PAUSEGRAPH() PauseTimer()

#define INIT_DEBUG_TIMER()


#else

#define RECORDTHREAD()

#define TIMEBLOCK(COLOR)
#define TIMEBLOCKTAGGED(NAME,COLOR)
#define MASTERTIMEBLOCKSTART(COLOR)
#define EXECTIMEBLOCK(COLOR)
#define BUILDGUIGRAPH(GUI,FONT,UPPERBOUND)
#define PRINTBLOCKS()
#define PRINTTIMEBLOCK(COLOR)
#define PAUSEGRAPH()
#define INIT_DEBUG_TIMER()
#define PRINTTIMEBLOCKTAGGED(NAME)
#endif

struct DebugRecord{
    
    Color color;
  
    TimeSpec start_stamp;
    f32 timelen;
    u64 cyclelen;
    
    const s8* file;
    const s8* function;
    u32 line;
};

#ifndef WIN32DLL

void SubmitRecord(ThreadID tid,DebugRecord record);

#else

_persist  f32 (*timediff)(TimeSpec,TimeSpec) = 0;

#define GetTimeDifferenceMS(a,b) timediff(a,b)

_persist  void (*SubmitRecord)(ThreadID,DebugRecord) = 0;

#endif

void RecordThread();

void SetStartTimeBlock(TimeSpec timestamp);

void SetFrameTime(f32 time);

void SetExecTime(f32 time);

u32 GetThreadIndex(ThreadID tid);

struct TimeBlock{
  
  TimeSpec start_stamp;
  u64 start_cycle;
    
    const s8* file;
    const s8* function;
    u32 line;
    Color color;
    
    
    TimeBlock(const s8* File,u32 Line,const s8* Function,Color c){
      
      file = File;
      line = Line;
      function = Function;
      color = c;

      start_cycle = Rdtsc();
	
      GetTime(&start_stamp);
    }
    
    ~TimeBlock(){
        
        auto end_cycle = Rdtsc();
        
        auto cyclediff = end_cycle - start_cycle;
        
        TimeSpec end;  
        
        GetTime(&end);
        
        auto len = GetTimeDifferenceMS(start_stamp,end);
        
        SubmitRecord(TGetThisThreadID(),{color,start_stamp,len,cyclediff,file,function,line});
    }
};

struct MasterTimeBlock : TimeBlock{

 MasterTimeBlock(const s8* File,u32 Line,const s8* Function,Color c) :
  TimeBlock(File,Line,Function,c){
    SetStartTimeBlock(start_stamp);
  }

 ~MasterTimeBlock(){
   
   TimeSpec end;  
        
   GetTime(&end);
   
   SetFrameTime(GetTimeDifferenceMS(start_stamp,end));
  }
  
};


struct ExecTimeBlock : TimeBlock{

 ExecTimeBlock(const s8* File,u32 Line,const s8* Function,Color c) :
  TimeBlock(File,Line,Function,c){}

  ~ExecTimeBlock(){
   
    TimeSpec end;  
        
    GetTime(&end);
   
    SetExecTime(GetTimeDifferenceMS(start_stamp,end));
  }
  
};

struct PrintTimeBlock : TimeBlock{
    
    PrintTimeBlock(const s8* File,u32 Line,const s8* Function,Color c) :
    TimeBlock(File,Line,Function,c){
    }
    
    ~PrintTimeBlock(){
        
        TimeSpec end;  
        
        GetTime(&end);
        
        auto len = GetTimeDifferenceMS(start_stamp,end);
        printf("%s %s %d : %f\n",file,function,line,len);
    }
};



void BuildGraph(logic to_draw);


void PrintEntries();

void InitDebugTimer();

void PauseTimer();


struct _cachealign RecordArray{
  DebugRecord array[100];
    
  DebugRecord& operator [](ptrsize index){
    return array[index];
  }
};

struct DebugTable{
  TimeSpec timestamp;
  volatile ThreadID threadid_array[15] = {};//should set this to your threadcount
  volatile u32 thread_count = 0;
    
  //We should cache align these
  // DebugRecord record_array[15][100] = {};
  RecordArray record_array[15];
  u32 recordcount_array[15] = {};
};
