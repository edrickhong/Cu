
#include "ttimer.h"

_persist long long freq;//counts/s

void TInitTimer(){
  LARGE_INTEGER t;
  QueryPerformanceFrequency(&t);

  freq = t.QuadPart;
}


f32 GetTimeDifferenceMS(TimeSpec start ,TimeSpec end){

  long long counts;

  if ((end.QuadPart - start.QuadPart) > 0) {
      counts = end.QuadPart - start.QuadPart;
    }
  else{
    counts = start.QuadPart - end.QuadPart;
  }

  

  return (1000.0f * (f32)counts) / (f32)freq;
}
