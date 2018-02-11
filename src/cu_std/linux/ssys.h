#pragma once
#include "mode.h"

#include "unistd.h"
#include "sys/mman.h"

#define MEMPROT_EXEC PROT_EXEC
#define MEMPROT_READ PROT_READ
#define MEMPROT_WRITE PROT_WRITE
#define MEMPROT_READWRITE (PROT_WRITE | PROT_READ)
#define MEMPROT_NOACCESS PROT_NONE


#define ALLOCFLAG_LARGEPAGES_ENABLE MAP_HUGETLB
#define ALLOCFLAG_LARGEPAGES_2MB (21 << MAP_HUGE_SHIFT)
#define ALLOCFLAG_LARGEPAGES_1GB (30 << MAP_HUGE_SHIFT)

u32 _ainline SGetTotalThreads(){
  return sysconf(_SC_NPROCESSORS_ONLN);
}

_ainline void* SSysAlloc(void* addr,ptrsize size,u32 prot,u32 flags){
  return mmap(addr, size, prot , MAP_PRIVATE | MAP_ANONYMOUS | flags,-1, 0);
}

_ainline void SSysAlloc(void* addr,ptrsize size){
  munmap(addr, size);
}
