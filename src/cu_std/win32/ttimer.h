#pragma once
#include "mode.h"
#include "ttype.h"

#ifdef _MSC_VER
#include "intrin.h"
#else
#include "x86intrin.h"
#endif

#include "Windows.h"

#define Rdtsc __rdtsc

typedef LARGE_INTEGER TimeSpec;

void TInitTimer();


void _ainline GetTime(TimeSpec* timespec){
  QueryPerformanceCounter(timespec);
}

f32 GetTimeDifferenceMS(TimeSpec,TimeSpec);

void _ainline SleepMS(f32 time){
  Sleep((DWORD)time);
}
