#pragma once
#include "mode.h"

#include "Windows.h"

#define MEMPROT_EXEC PAGE_EXECUTE
#define MEMPROT_READ PAGE_READONLY
#define MEMPROT_WRITE PAGE_READWRITE
#define MEMPROT_READWRITE PAGE_READWRITE
#define MEMPROT_NOACCESS PAGE_NOACCESS


#define ALLOCFLAG_LARGEPAGES_ENABLE MAP_HUGETLB
#define ALLOCFLAG_LARGEPAGES_2MB (21 << MAP_HUGE_SHIFT)
#define ALLOCFLAG_LARGEPAGES_1GB (30 << MAP_HUGE_SHIFT)

u32 _ainline SGetTotalThreads(){
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;
}

#define SSysAlloc(addr,len,mem_prot,flags) VirtualAlloc(addr,len,MEM_COMMIT | MEM_RESERVE,mem_prot)

#define SSysFree(addr,len) VirtualFree(addr,len,MEM_RELEASE)
