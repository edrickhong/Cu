#pragma once
#include "mode.h"
#include "ttype.h"

#include "x86intrin.h"
#include "time.h"

#define Rdtsc __rdtsc

typedef timespec TimeSpec;

#define TInitTimer();


void _ainline GetTime(TimeSpec* timespec){
    clock_gettime(CLOCK_MONOTONIC,timespec);
}

f32 GetTimeDifferenceMS(TimeSpec,TimeSpec);

void _ainline SleepMS(f32 time){
    
    struct timespec time1 = {0,(long)(time * 1000000.0f )};
    nanosleep(&time1,0);
}
