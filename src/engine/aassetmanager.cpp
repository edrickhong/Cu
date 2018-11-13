#include "aassetmanager.h"
#include "ssys.h"

/*
TODO:

Fresh start, we should diff this against the working implementation and patch in eviction.

*/

struct InternalTransferBuffer{
    VBufferContext buffer;
    VkDeviceSize offset;
    u32 size;
};

_persist InternalTransferBuffer main_transferbuffer = {};
_persist InternalTransferBuffer async_transferbuffer = {};

VkDeviceSize AllocateTransferBuffer(InternalTransferBuffer* _restrict transferbuffer,
                                    u32 size){
    
#if 0
    
    printf("TRANSFERBUFFER ALLOCATE\n");
    
#endif
    
    VkDeviceSize offset;
    VkDeviceSize new_offset;
    VkDeviceSize actual_offset;
    
    size = _mapalign(size);
    
    do{
        
        offset = transferbuffer->offset;
        new_offset = _mapalign(offset + size);
        
        actual_offset = LockedCmpXchg(&transferbuffer->offset,offset,new_offset);
        
    }while(offset != actual_offset);
    
    _kill("transferbuffer ran out of memory\n",new_offset >= transferbuffer->size);
    
    
    return offset;
}

// This is for the fetch thread only
_persist VBufferContext fetchthread_transferbuffer;
_persist VkDeviceSize fetchthread_transferbuffer_offset = 0;

_persist VDeviceContext* global_device = {};

//2500
#define _maxblocks 5000
#define _defarray_size 100

#define _maxassets 2000


struct MemoryBlock{
    s8* ptr;
    u32 size;
    MemoryBlock* higherblock;
    MemoryBlock* lowerblock;
};


struct MemoryState{
    void* global_ptr;
    
    MemoryBlock memoryblock_array[_maxblocks] = {};
    u32 memoryblock_count = 0;
    
    
    //memoryblock_array is a sparse array. We use check* to mark empty elements in the array
    //AKA no block points to it.
    
    u32 checkmemoryblock_array[_maxblocks] = {};
    u32 checkmemoryblock_count = 0;
    
    MemoryBlock* free_array[_maxblocks] = {};
    u32 free_count = 0;
    
    MemoryBlock* occupied_array[_maxblocks] = {};
    u32 occupied_count = 0;
    
    s8* asset_file[_maxassets] = {};
    TIMESTAMP asset_timestamp[_maxassets] = {};
    MemoryBlock* asset_memory[_maxassets] = {};
    u32 asset_count = 0;
};

struct GPUBlock{
    VkDeviceSize ptr;
    VkDeviceSize size;
    GPUBlock* higherblock;
    GPUBlock* lowerblock;
    VkBuffer buffer;
};

struct GPUMemState{
    VkDeviceMemory global_ptr;
    
    GPUBlock memoryblock_array[_maxblocks] = {};
    u32 memoryblock_count = 0;
    
    
    //memoryblock_array is a sparse array. We use check* to mark empty elements in the array
    //AKA no block points to it.
    
    u32 checkmemoryblock_array[_maxblocks] = {};
    u32 checkmemoryblock_count = 0;
    
    GPUBlock* free_array[_maxblocks] = {};
    u32 free_count = 0;
    
    GPUBlock* occupied_array[_maxblocks] = {};
    u32 occupied_count = 0;
    
    s8* asset_file[_maxassets] = {};
    TIMESTAMP asset_timestamp[_maxassets] = {};
    GPUBlock* asset_memory[_maxassets] = {};
    u32 asset_count = 0;
};

struct MemorySlot{
    MemoryBlock* block;
    u32 id;
};

struct GPUMemorySlot{
    GPUBlock* block;
    u32 id;
};


_persist MemoryState global_memstate = {};
_persist GPUMemState global_gpumemstate = {};

_persist TIMESTAMP global_timestamp = 0xFFFFFFFF;

#define _DAddBlocksFn(type,state) type* AddBlocks##type(){		\
    if(state.checkmemoryblock_count){					\
        state.checkmemoryblock_count--;					\
        auto index = state.checkmemoryblock_array[state.checkmemoryblock_count]; \
        return &state.memoryblock_array[index];				\
    }									\
    auto ptr = &state.memoryblock_array[state.memoryblock_count];	\
    state.memoryblock_count++;						\
    return ptr;								\
}
#define _DRemoveBlockFromStateArrayFn(type,state) void RemoveBlockFromArray##type( \
type* block){ \
    auto index = (block - (&state.memoryblock_array[0]));		\
    _kill("Invalid block\n",index > _maxblocks);			\
    state.checkmemoryblock_array[state.checkmemoryblock_count] = index; \
    state.checkmemoryblock_count++;					\
}
#define _DRemoveBlockFn(type) void RemoveBlock##type(			\
type** array,u32* count,type* block){ \
    for(u32 i = 0; i < (*count); i++){					\
        if(array[i] == block){						\
            array[i] = array[(*count) -1];					\
            (*count)--;							\
            return;								\
        }									\
    }									\
    _kill("block not in list\n",1);					\
}
#define _DRemoveSlotFn(slot_type,block_type) void RemoveSlot##block_type(slot_type* array, \
u32* count,u32 index){ \
    array[index] = array[(*count) - 1];					\
    (*count)--;								\
}									\
void RemoveSlot##block_type(slot_type* array,u32* count,block_type* block){ \
    for(u32 i = 0; i < (*count); i++){					\
        if(array[i].block == block){					\
            array[i] = array[(*count) -1];					\
            (*count)--;							\
            return;								\
        }									\
    }									\
    _kill("block not in list\n",1);					\
}

#define _DAddBlockFn(type) void AddBlock##type(type** array,u32* count,type* block){ \
    array[*count] = block;						\
    (*count)++;								\
}

#define _DAllocateFromFreeBlockFn(type,state) void AllocateFromFreeBlock##type(type* a, \
u32 size){ \
    _kill("0 size allocation\n",!size);					\
    _kill("insufficient blocksize\n",(a->size < size));			\
    if(a->size == size){						\
        _allocprint("exact alloc. Remove %p from free. Added %p to occupied %llu \n",(void*)a,(void*)a,a->size); \
        RemoveBlock##type(state.free_array,&state.free_count,a);		\
        AddBlock##type(state.occupied_array,&state.occupied_count,a);	\
        return;								\
    }									\
    type* b = AddBlocks##type();					\
    *b = {a->ptr + size,a->size - size,a->higherblock,a};		\
    if(a->higherblock){							\
        a->higherblock->lowerblock = b;					\
    }									\
    a->size = size;							\
    a->higherblock = b;							\
    _allocprint("Remove %p from free. Added %p to occupied. Added %p to free. %llu %llu \n",(void*)a,(void*)a,(void*)b,a->size,b->size); \
    RemoveBlock##type(state.free_array,&state.free_count,a);		\
    AddBlock##type(state.occupied_array,&state.occupied_count,a);	\
    AddBlock##type(state.free_array,&state.free_count,b);		\
}
#define _DAllocateFromOccupiedBlockFn(type,state) void AllocateFromOccupiedBlock##type(	\
type* a,u32 size){ \
    _kill("0 size allocation\n",!size);					\
    _kill("insufficient blocksize\n",(a->size < size));			\
    if(a->size == size){						\
        _allocprint("exact alloc%s\n","");				\
        return;								\
    }									\
    type* b = AddBlocks##type();					\
    *b = {a->ptr + size,a->size - size,a->higherblock,a};		\
    a->size = size;							\
    a->higherblock = b;							\
    _allocprint("Added %p to free %llu\n",(void*)b,b->size);		\
    AddBlock##type(state.free_array,&state.free_count,b);		\
}
#define _DAllocateBlockFn(type,state) type* AllocateBlock##type(u32 size){ \
    for(u32 i = 0; i < state.free_count; i++){ \
        auto block = state.free_array[i]; \
        if(block->size >= size){ \
            AllocateFromFreeBlock##type(block,size); \
            return block; \
        } \
    } \
    return 0; \
}

#define _DIsInListFn(slot_type,block_type) logic IsInList##block_type(slot_type* list,u32 count, \
block_type* block){ \
    for(u32 i = 0; i < count; i++){ \
        if(block == list[i].block){ \
            return 1; \
        } \
    } \
    return 0; \
} \
logic IsInList##block_type(block_type** list,u32 count,block_type* block){ \
    for(u32 i = 0; i < count; i++){ \
        if(block == list[i]){ \
            return 1; \
        } \
    } \
    return 0; \
}

#define _DMergeBlockFn(type) logic MergeBlock##type(type* a,type* b){	\
    if(b->lowerblock != a){						\
        return 0;					\
    }									\
    a->size += b->size;							\
    a->higherblock = b->higherblock;					\
    if(b->higherblock){							\
        b->higherblock->lowerblock = a;					\
    }									\
    RemoveBlockFromArray##type(b);					\
    return 1;								\
}

#define _DLinkAllocateBlockListFn(slot_type,block_type,state)		\
logic LinkAllocateBlockList##block_type(slot_type* list,u32* count,u32 size,slot_type* slot){	\
    _allocprint("Link alloc%s\n","");					\
    slot_type sort_array[_defarray_size];				\
    u32 sort_count = *count;						\
    _kill("Not enough space\n",sort_count > _defarray_size);		\
    memcpy(sort_array,list,sort_count * sizeof(slot_type));		\
    for(u32 i = 0; i < state.free_count; i++){				\
        sort_array[sort_count].block = state.free_array[i];		\
        sort_array[sort_count].id = -1;					\
        sort_count++;							\
    }									\
    qsort(sort_array,sort_count,sizeof(slot_type),			\
    [](const void * a, const void* b)->s32 {			\
        auto slot_a = (slot_type*)a;				\
        auto slot_b = (slot_type*)b;				\
        return slot_a->block->ptr - slot_b->block->ptr;		\
    });								\
    slot_type commitslot_array[_defarray_size];				\
    u32 commitslot_count = 0;						\
    u32 collectedsize = 0;						\
    for(u32 i = sort_count - 1; i != 0; i--){				\
        auto a = sort_array[i - 1].block;					\
        auto b = sort_array[i].block;					\
        commitslot_array[commitslot_count] = sort_array[i];		\
        commitslot_count ++;						\
        collectedsize += b->size;						\
        if(collectedsize >= size){					\
            for(u32 j = 0; j < commitslot_count - 1; j++){			\
                auto c = commitslot_array[j + 1].block;			\
                auto d = commitslot_array[j].block;				\
                auto res = MergeBlock##block_type(c,d);			\
                _kill("Failed to merge\n",!res);				\
                if(commitslot_array[j].id != (u32)-1){			\
                    state.asset_file[commitslot_array[j].id] = 0;		\
                    RemoveBlock##block_type(state.occupied_array,&state.occupied_count,d); \
                }								\
                else{								\
                    RemoveBlock##block_type(state.free_array,&state.free_count,d); \
                }								\
            }								\
            auto newslot = commitslot_array[commitslot_count - 1];		\
            if(newslot.id == (u32)-1){					\
                AddBlock##block_type(state.occupied_array,&state.occupied_count,newslot.block); \
            }								\
            AllocateFromOccupiedBlock##block_type(newslot.block,size);	\
            *slot = newslot;						\
            return 1;							\
        }									\
        if(b->lowerblock != a){						\
            collectedsize = 0;						\
            commitslot_count = 0;						\
        }									\
    }									\
    return 0;								\
}

#define _DMergeAllFreeFn(type,state) void MergeAllFree##type(){		\
    type* sort_array[_defarray_size];					\
    u32 sort_count = state.free_count;					\
    state.free_count = 0;						\
    _kill("Not enough space\n",sort_count > _defarray_size);		\
    memcpy(sort_array,state.free_array,sort_count * sizeof(type*));	\
    qsort(sort_array,sort_count,sizeof(type*),				\
    [](const void * a, const void* b)->s32 {			\
        auto block_a = (type*)a;					\
        auto block_b = (type*)b;					\
        return block_a->ptr - block_b->ptr;				\
    });								\
    for(u32 i = (sort_count - 1);;i --){				\
        if(i == 0){							\
            state.free_array[state.free_count] = sort_array[i];		\
            state.free_count++;						\
            break;								\
        }									\
        auto a = sort_array[i - 1];					\
        auto b = sort_array[i];						\
        if(!MergeBlock##type(a,b)){					\
            state.free_array[state.free_count] = b;				\
            state.free_count++;						\
        }									\
    }									\
}

_DAddBlocksFn(MemoryBlock,global_memstate);
_DAddBlocksFn(GPUBlock,global_gpumemstate);

_DRemoveBlockFromStateArrayFn(MemoryBlock,global_memstate);
_DRemoveBlockFromStateArrayFn(GPUBlock,global_gpumemstate);

_DRemoveBlockFn(GPUBlock);
_DRemoveBlockFn(MemoryBlock);

_DRemoveSlotFn(MemorySlot,MemoryBlock);
_DRemoveSlotFn(GPUMemorySlot,GPUBlock);

_DAddBlockFn(MemoryBlock);
_DAddBlockFn(GPUBlock);

_DAllocateFromFreeBlockFn(MemoryBlock,global_memstate);
_DAllocateFromFreeBlockFn(GPUBlock,global_gpumemstate);

_DAllocateFromOccupiedBlockFn(MemoryBlock,global_memstate);
_DAllocateFromOccupiedBlockFn(GPUBlock,global_gpumemstate);

_DAllocateBlockFn(MemoryBlock,global_memstate);
_DAllocateBlockFn(GPUBlock,global_gpumemstate);

_DIsInListFn(MemorySlot,MemoryBlock);
_DIsInListFn(GPUMemorySlot,GPUBlock);

_DMergeBlockFn(MemoryBlock);
_DMergeBlockFn(GPUBlock);

_DLinkAllocateBlockListFn(MemorySlot,MemoryBlock,global_memstate);
_DLinkAllocateBlockListFn(GPUMemorySlot,GPUBlock,global_gpumemstate);

_DMergeAllFreeFn(MemoryBlock,global_memstate);
_DMergeAllFreeFn(GPUBlock,global_gpumemstate);


void SwapBlock_CopyDown(MemoryBlock* a, MemoryBlock* b){// a < b
    
    printf("DEFRAG CPY");
    
    memcpy(a->ptr,b->ptr,b->size);
    
    auto t = b->ptr;
    b->ptr = a->ptr;
    a->ptr = t;
    
    b->lowerblock = a->lowerblock;
    a->higherblock = b->higherblock;
    b->higherblock = a;
    a->lowerblock = b;
    
    if(a->higherblock){
        a->higherblock->lowerblock = a;  
    }
    
    if(b->lowerblock){
        b->lowerblock->higherblock = b;  
    }
    
}

//TODO: I don't trust this 
void InternalDefrag(){
    
    /*If free blocks are always pushed up, we will get fragmented blocks again*/
    
    MergeAllFreeMemoryBlock();
    
    qsort(global_memstate.occupied_array,global_memstate.occupied_count,
          sizeof(MemoryBlock*),
          [](const void * a, const void* b)->s32 {
          
          auto block_a = (MemoryBlock*)a;
          auto block_b = (MemoryBlock*)b;
          
          return block_a->ptr - block_b->ptr;
          });
    
    //we should move the first item to the bottom of the heap
    if(global_memstate.occupied_array[0]->lowerblock){
        
        auto lowestblock = global_memstate.occupied_array[0]->lowerblock;
        
        SwapBlock_CopyDown(lowestblock, global_memstate.occupied_array[0]);
    }
    
    for(u32 i = 1; i < global_memstate.occupied_count; i++){
        
        auto start = global_memstate.occupied_array[i - 1];
        auto next = global_memstate.occupied_array[i];
        
        if(start->higherblock != next){
            
            auto freeblock = start->higherblock;//global_memstate.
            
            _kill("Unaccounted block\n",
                  !IsInListMemoryBlock(global_memstate.free_array,global_memstate.free_count,
                                       freeblock));
            
            SwapBlock_CopyDown(freeblock,next);
            
            //check if freeblock's higherblock is free. If it is, merge
            
            if(IsInListMemoryBlock(global_memstate.free_array,global_memstate.free_count,
                                   freeblock->higherblock)){
                
                auto higher = freeblock->higherblock;
                
                auto res = MergeBlockMemoryBlock(freeblock,higher);
                _kill("Failed to merge\n",!res);
                
                RemoveBlockMemoryBlock(global_memstate.free_array,&global_memstate.free_count,
                                       higher);
                
            }
            
        }
        
    }
    
}

void GPUSwapBlock_CopyDown(GPUBlock* a, GPUBlock* b,
                           const  VDeviceContext* _restrict vdevice,VkCommandBuffer cmdbuffer){
    
    printf("GPU DEFRAG CPY");
    
    if(!a->buffer){
        a->buffer =
            VRawCreateBuffer(vdevice,0,a->size,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    }
    
    if(!b->buffer){
        b->buffer =
            VRawCreateBuffer(vdevice,0,b->size,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    }
    
    VkBufferCopy buffercopy = {0,0,b->size};
    
    vkCmdCopyBuffer(cmdbuffer,a->buffer,
                    b->buffer,1,&buffercopy);
    
    auto t = b->ptr;
    b->ptr = a->ptr;
    a->ptr = t;
    
    b->lowerblock = a->lowerblock;
    a->higherblock = b->higherblock;
    b->higherblock = a;
    a->lowerblock = b;
    
    if(a->higherblock){
        a->higherblock->lowerblock = a;  
    }
    
    if(b->lowerblock){
        b->lowerblock->higherblock = b;  
    }
    
    vkBindBufferMemory(vdevice->device,a->buffer,global_gpumemstate.global_ptr,a->ptr);
    vkBindBufferMemory(vdevice->device,b->buffer,global_gpumemstate.global_ptr,b->ptr);
    
}

//TODO: I don't trust this 
void GPUInternalDefrag(const  VDeviceContext* _restrict vdevice,
                       VkCommandBuffer cmdbuffer){
    
    MergeAllFreeGPUBlock();
    
    qsort(global_gpumemstate.occupied_array,global_gpumemstate.occupied_count,
          sizeof(GPUBlock*),
          [](const void * a, const void* b)->s32 {
          
          auto block_a = (GPUBlock*)a;
          auto block_b = (GPUBlock*)b;
          
          return block_a->ptr - block_b->ptr;
          });
    
    if(global_gpumemstate.occupied_array[0]->lowerblock){
        
        auto lowestblock = global_gpumemstate.occupied_array[0]->lowerblock;
        
        GPUSwapBlock_CopyDown(lowestblock,global_gpumemstate.occupied_array[0],
                              vdevice,cmdbuffer);
    }
    
    for(u32 i = 1; i < global_gpumemstate.occupied_count; i++){
        
        auto start = global_gpumemstate.occupied_array[i - 1];
        auto next = global_gpumemstate.occupied_array[i];
        
        if(start->higherblock != next){
            
            auto freeblock = start->higherblock;//global_memstate.
            
            _kill("Unaccounted block\n",
                  !IsInListGPUBlock(global_gpumemstate.free_array,global_gpumemstate.free_count,
                                    freeblock));
            
            GPUSwapBlock_CopyDown(freeblock,next,vdevice,cmdbuffer);
            
            //check if freeblock's higherblock is free. If it is, merge
            if(IsInListGPUBlock(global_gpumemstate.free_array,global_gpumemstate.free_count,
                                freeblock->higherblock)){
                
                auto higher = freeblock->higherblock;
                
                auto res = MergeBlockGPUBlock(freeblock,higher);
                _kill("Failed to merge\n",!res);
                
                RemoveBlockGPUBlock(global_gpumemstate.free_array,&global_gpumemstate.free_count,
                                    higher);
                
            }
            
        }
        
    }
    
}

void InternalAllocateAsset(AssetHandle* handle,u32 size){
    
    TIMEBLOCK(LimeGreen);
    
    MemorySlot slot = {};
    
    auto allocsize = _align16(size);
    
    slot.block = AllocateBlockMemoryBlock(allocsize);
    slot.id = -1;
    
    if(!slot.block){
        
        _allocprint("evicting assets%s\n","");
        //allocate from existing asset
        
        MemorySlot evictslot_array[_defarray_size];
        u32 evictslot_count = 0;
        
        for(u32 i = 0; i < global_memstate.asset_count; i++){//get a list of all evictable assets
            
            if(global_memstate.asset_timestamp[i] != global_timestamp &&
               global_memstate.asset_file[i]){
                
                evictslot_array[evictslot_count].id = i;
                evictslot_count++;
            }
            
        }
        
        _kill("No assets can be evicted\n",!evictslot_count);
        
        for(u32 i = 0; i < evictslot_count; i++){
            auto block = global_memstate.asset_memory[evictslot_array[i].id];
            
            if(block->size >= allocsize){
                AllocateFromOccupiedBlockMemoryBlock(block,allocsize);
                slot.block = block;
                slot.id = evictslot_array[i].id;
                goto fin_allocation;
            }
            evictslot_array[i].block = block;
        }
        
        if(LinkAllocateBlockListMemoryBlock(evictslot_array,&evictslot_count,allocsize,&slot)){
            goto fin_allocation;
        }
        
        InternalDefrag();//TODO: Step through this - We have to replace this too
        
        slot.block = AllocateBlockMemoryBlock(allocsize);
        
        if(slot.block){
            goto fin_allocation;
        }
        
        if(LinkAllocateBlockListMemoryBlock(evictslot_array,&evictslot_count,allocsize,&slot)){
            goto fin_allocation;
        }
        
        _kill("Ran out of free memory\n",1);
    }
    
    fin_allocation:
    
    if(slot.id == (u32)-1){
        
        for(u32 i = 0; i < global_memstate.asset_count; i++){
            
            if(!global_memstate.asset_file[i]){
                slot.id = i;
            }
        }
        
        if(slot.id == (u32)-1){
            _kill("ran out of slots",global_memstate.asset_count >= _maxassets);
            slot.id = global_memstate.asset_count;
            global_memstate.asset_count++; 
        }
    }
    
    handle->id = slot.id;
    
    global_memstate.asset_file[handle->id] = handle->assetfile;
    
    global_memstate.asset_timestamp[handle->id] = global_timestamp;
    global_memstate.asset_memory[handle->id] = slot.block;
    
    handle->ptr = global_memstate.asset_memory[handle->id]->ptr;
}

void GPUInternalAllocateAsset(ModelAssetHandle* handle,u32 size){
    
    TIMEBLOCK(Silver);
    
    GPUMemorySlot slot = {};
    
    auto allocsize = _devicealign(size);
    
    slot.block = AllocateBlockGPUBlock(allocsize);
    slot.id = -1;
    
    if(!slot.block){
        
        _allocprint("evicting assets%s\n","");
        //allocate from existing asset
        
        GPUMemorySlot evictslot_array[_defarray_size];
        u32 evictslot_count = 0;
        
        for(u32 i = 0; i < global_gpumemstate.asset_count; i++){//get a list of all evictable assets
            
            if(global_gpumemstate.asset_timestamp[i] != global_timestamp &&
               global_gpumemstate.asset_file[i]){
                evictslot_array[evictslot_count].id = i;
                evictslot_count++;
            }
            
        }
        
        _kill("No assets can be evicted\n",!evictslot_count);
        
        for(u32 i = 0; i < evictslot_count; i++){
            auto block = global_gpumemstate.asset_memory[evictslot_array[i].id];
            
            if(block->size >= allocsize){
                AllocateFromOccupiedBlockGPUBlock(block,allocsize);
                slot.block = block;
                slot.id = evictslot_array[i].id;
                goto gpufin_allocation;
            }
            evictslot_array[i].block = block;
        }
        
        if(LinkAllocateBlockListGPUBlock(evictslot_array,&evictslot_count,allocsize,&slot)){
            goto gpufin_allocation;
        }
        
        _kill("Ran out of free memory\n",1);
    }
    
    gpufin_allocation:
    
    if(slot.id == (u32)-1){
        
        for(u32 i = 0; i < global_gpumemstate.asset_count; i++){
            
            if(!global_gpumemstate.asset_file[i]){
                slot.id = i;
            }
        }
        
        if(slot.id == (u32)-1){
            _kill("ran out of slots",global_gpumemstate.asset_count >= _maxassets);
            slot.id = global_gpumemstate.asset_count;
            global_gpumemstate.asset_count++; 
        }
    }
    
    handle->gpuid = slot.id;
    
    global_gpumemstate.asset_file[handle->gpuid] = handle->assetfile;
    global_gpumemstate.asset_timestamp[handle->gpuid] = global_timestamp;
    global_gpumemstate.asset_memory[handle->gpuid] = slot.block;
    
    handle->gpuptr = global_gpumemstate.asset_memory[handle->gpuid]->ptr;
    
}

void UnallocateAsset(AssetHandle* handle){
    
    if(global_memstate.asset_file[handle->id] == handle->assetfile){
        
        global_memstate.asset_file[handle->id] = 0;
        global_memstate.asset_timestamp[handle->id] = 0;
        auto block = global_memstate.asset_memory[handle->id];
        
        RemoveBlockMemoryBlock(global_memstate.occupied_array,
                               &global_memstate.occupied_count,block);
        
        AddBlockMemoryBlock(global_memstate.free_array,&global_memstate.free_count,block);
        
        _allocprint("removed %p\n",(void*)block);
        
        //TODO: Can probs use a more lightweight method
        MergeAllFreeMemoryBlock();
    }
}

void GPUUnallocateAsset(ModelAssetHandle* handle){
    
    if(global_gpumemstate.asset_file[handle->gpuid] == handle->assetfile){
        
        global_gpumemstate.asset_file[handle->gpuid] = 0;
        global_gpumemstate.asset_timestamp[handle->gpuid] = 0;
        auto block = global_gpumemstate.asset_memory[handle->gpuid];
        
        RemoveBlockGPUBlock(global_gpumemstate.occupied_array,
                            &global_gpumemstate.occupied_count,block);
        
        
        AddBlockGPUBlock(global_gpumemstate.free_array,
                         &global_gpumemstate.free_count,block);
        
        _allocprint("removed %p\n",(void*)block);
        
        //TODO: Can probs use a more lightweight method
        MergeAllFreeGPUBlock();
    }
}

u32 _ainline  CheckAsset(AssetHandle* handle){
    
    if(handle->assetfile == global_memstate.asset_file[handle->id]){
        global_memstate.asset_timestamp[handle->id] = global_timestamp;
        handle->ptr = global_memstate.asset_memory[handle->id]->ptr;
        return 1;
    }
    
    return 0;
}

u32 _ainline GpuCheckAsset(ModelAssetHandle* handle){
    
    if(global_gpumemstate.memoryblock_array[handle->gpuid].buffer){
        
        vkBindBufferMemory(global_device->device,handle->vertexbuffer.buffer,
                           global_gpumemstate.global_ptr,handle->gpuptr);
        
        vkBindBufferMemory(global_device->device,handle->indexbuffer.buffer,
                           global_gpumemstate.global_ptr,
                           handle->gpuptr + handle->vertexbuffer.size);
        
        VDestroyBuffer(global_device,
                       global_gpumemstate.memoryblock_array[handle->gpuid].buffer);
    }
    
    if(handle->assetfile == global_gpumemstate.asset_file[handle->gpuid]){
        global_gpumemstate.asset_timestamp[handle->gpuid] = global_timestamp;
        return 1;
    }
    
    return 0;
}

AudioAssetHandle AllocateAssetAudio(const s8* filepath){
    
    AudioAssetHandle handle = {};
    handle.assetfile = (s8*)filepath;
    
    u32 size = _fixed_audio;
    
    InternalAllocateAsset((AssetHandle*)&handle,size);
    
    ADFGetInfo(filepath,&handle.compression_type,&handle.file_size);
    handle.file_offset = 0;
    
    return handle;
}

void AllocateAssetAnimated(const s8* filepath,
                           const VDeviceContext* _restrict vdevice,
                           VkQueue queue,VkCommandBuffer commandbuffer,
                           u32 vertexbinding_no,AnimatedAssetHandle* animbone,
                           ModelAssetHandle* vertindex){
    
    *animbone = {};
    *vertindex = {};//for vertex data that will be discarded once on gpu
    
    animbone->assetfile = (s8*)filepath;
    vertindex->assetfile = (s8*)filepath;
    
    u32 vertindex_size;
    u32 animbone_size;
    
    LoadMDF(filepath,0,0,&vertindex_size,&animbone_size);
    
    vertindex->ptr = TAlloc(s8,vertindex_size);
    
    InternalAllocateAsset((AssetHandle*)animbone,animbone_size);
    
    auto mdf = LoadMDF(filepath,vertindex->ptr,animbone->ptr,0,0);
    
    _kill("model loaded is not skeletal\n",mdf.vertex_component != 7);
    
    vertindex->vert_component = mdf.vertex_component;
    
    vertindex->vert_fileoffset = mdf.vertexdata_offset;
    vertindex->index_fileoffset = mdf.indexdata_offset;
    
    animbone->animbonesize = animbone_size;
    animbone->anim_fileoffset = mdf.animdata_offset;
    animbone->bone_fileoffset = mdf.bonedata_offset;
    
    animbone->rootbone = mdf.root_linearskeleton;
    animbone->bone_count = mdf.bone_count;
    animbone->animationset_array =  mdf.animationset_array;
    animbone->animationset_count = mdf.animationset_count;
    
    auto vertsize = _devicealign(mdf.vertex_size);
    auto indexsize = _devicealign(mdf.index_size);
    
    GPUInternalAllocateAsset(vertindex,_devicealign(vertsize + indexsize));
    
    auto transferbuffer = main_transferbuffer.buffer;
    auto transferbuffer_offset =
        AllocateTransferBuffer(&main_transferbuffer,mdf.vertex_size + mdf.index_size);
    
    vertindex->vertexbuffer =
        VCreateStaticVertexBuffer(vdevice,commandbuffer,
                                  global_gpumemstate.global_ptr,
                                  vertindex->gpuptr,
                                  transferbuffer,transferbuffer_offset,mdf.vertex_data,
                                  mdf.vertex_size,vertexbinding_no);
    
    vertindex->indexbuffer =
        VCreateStaticIndexBufferX(vdevice,
                                  commandbuffer,
                                  global_gpumemstate.global_ptr,
                                  _devicealign(vertindex->gpuptr + mdf.vertex_size),
                                  transferbuffer,transferbuffer_offset + mdf.vertex_size,
                                  mdf.index_data,mdf.index_size);
    
    vertindex->ptr = 0;
}


ModelAssetHandle AllocateAssetModel(const s8* filepath,
                                    const VDeviceContext* _restrict vdevice,VkQueue queue,
                                    VkCommandBuffer commandbuffer,u32 vertexbinding_no){
    
    ModelAssetHandle vertindex = {};
    vertindex.animation_id = (u32)-1;
    
    vertindex.assetfile = (s8*)filepath;
    u32 vertindex_size;
    
    LoadMDF(filepath,0,0,&vertindex_size,0);
    
    vertindex.ptr = TAlloc(s8,vertindex_size);
    
    auto mdf = LoadMDF(filepath,vertindex.ptr,vertindex.ptr,0,0);
    
    _kill("model loaded is not static\n",mdf.vertex_component != 3);
    
    vertindex.vert_component = mdf.vertex_component;
    
    vertindex.vert_fileoffset = mdf.vertexdata_offset;
    vertindex.index_fileoffset = mdf.indexdata_offset;
    
    auto vertsize = _devicealign(mdf.vertex_size);
    auto indexsize = _devicealign(mdf.index_size);
    
    GPUInternalAllocateAsset(&vertindex,_devicealign(vertsize + indexsize));
    
    auto transferbuffer = main_transferbuffer.buffer;
    auto transferbuffer_offset =
        AllocateTransferBuffer(&main_transferbuffer,mdf.vertex_size + mdf.index_size);
    
    vertindex.vertexbuffer =
        VCreateStaticVertexBuffer(vdevice,commandbuffer,
                                  global_gpumemstate.global_ptr,
                                  vertindex.gpuptr,
                                  transferbuffer,transferbuffer_offset,mdf.vertex_data,
                                  mdf.vertex_size,vertexbinding_no);
    
    vertindex.indexbuffer =
        VCreateStaticIndexBufferX(vdevice,
                                  commandbuffer,
                                  global_gpumemstate.global_ptr,
                                  _devicealign(vertindex.gpuptr + mdf.vertex_size),
                                  transferbuffer,transferbuffer_offset + mdf.vertex_size,
                                  mdf.index_data,mdf.index_size);
    
    vertindex.ptr = 0;
    
    return vertindex;
}

void CommitModel(ModelAssetHandle* handle,VkCommandBuffer cmdbuffer){
    
    if(!GpuCheckAsset(handle)){
        
        
        if(!CheckAsset((AssetHandle*)handle)){
            
            InternalAllocateAsset((AssetHandle*)handle,
                                  (handle->vertexbuffer.size + handle->indexbuffer.size));
        }
        
        //copy data to cpu memory
        auto vert = (s8*)handle->ptr;
        
        auto ind = vert + handle->vertexbuffer.size;
        
        auto file = FOpenFile(handle->assetfile,F_FLAG_READONLY);
        
        FSeekFile(file,handle->vert_fileoffset,F_METHOD_START);
        
        FRead(file,vert,handle->vertexbuffer.size);
        
        FSeekFile(file,handle->index_fileoffset,F_METHOD_START);
        
        FRead(file,ind,handle->indexbuffer.size);
        
        FCloseFile(file);
        
        
        GPUInternalAllocateAsset(handle,(handle->vertexbuffer.size + handle->indexbuffer.size));
        
        //bind and transfer vertex and index buffer
        
        vkBindBufferMemory(global_device->device,handle->vertexbuffer.buffer,
                           global_gpumemstate.global_ptr,handle->gpuptr);
        
        vkBindBufferMemory(global_device->device,handle->indexbuffer.buffer,
                           global_gpumemstate.global_ptr,handle->gpuptr + handle->vertexbuffer.size);
        
        //copy data to transfer buffer
        
        s8* mappedmemory_ptr;
        
        auto transferbuffer = main_transferbuffer.buffer;
        auto transferbuffer_offset =
            AllocateTransferBuffer(&main_transferbuffer,
                                   handle->vertexbuffer.size + handle->indexbuffer.size);
        
        VMapMemory(global_device,transferbuffer.memory,transferbuffer_offset,
                   (handle->vertexbuffer.size + handle->indexbuffer.size),(void**)&mappedmemory_ptr);
        
        memcpy(mappedmemory_ptr,vert,handle->vertexbuffer.size);
        
        memcpy(mappedmemory_ptr + handle->vertexbuffer.size,ind,handle->indexbuffer.size);
        
        vkUnmapMemory(global_device->device,transferbuffer.memory);
        
        //this usually happens in the middle of a draw.
        
        //MARK: doing this on another queue is not a bad idea either.
        
        VkBufferCopy copyregion[2];
        
        copyregion[0].srcOffset = 0;
        copyregion[0].dstOffset = transferbuffer_offset;
        copyregion[0].size = handle->vertexbuffer.size;
        
        copyregion[1].srcOffset = 0;
        copyregion[1].dstOffset = transferbuffer_offset + handle->vertexbuffer.size;
        copyregion[1].size = handle->indexbuffer.size;
        
        vkCmdCopyBuffer(cmdbuffer,handle->vertexbuffer.buffer,transferbuffer.buffer,1,
                        &copyregion[0]);
        
        vkCmdCopyBuffer(cmdbuffer,handle->indexbuffer.buffer,transferbuffer.buffer,1,
                        &copyregion[1]);
        
        UnallocateAsset((AssetHandle*)handle);
    }
    
}

void CommitAnimated(AnimatedAssetHandle* handle){
    
    if(!CheckAsset((AssetHandle*)handle)){
        //TODO: Untested but should work
        InternalAllocateAsset((AssetHandle*)handle,handle->animbonesize);
        
        auto ptr = (s8*)handle->ptr;
        u32 offset = 0;
        
        auto file = FOpenFile(handle->assetfile,F_FLAG_READONLY);
        
        FSeekFile(file,handle->anim_fileoffset,F_METHOD_START);
        
        FileReadAnimation(file,ptr,&offset,handle->animationset_count);
        
        FSeekFile(file,handle->bone_fileoffset,F_METHOD_START);
        
        FileReadAnimBoneLinear(file,ptr,&offset,handle->bone_count,handle->animationset_count,
                               &handle->rootbone,(u32*)&handle->bone_count);
        
        FCloseFile(file);
        
    }
    
}

void CommitAudio(AudioAssetHandle* handle){
    
    TIMEBLOCK(LimeGreen);
    
    if(!CheckAsset((AssetHandle*)handle)){
        
        u32 size = _fixed_audio;
        
        InternalAllocateAsset((AssetHandle*)handle,size);
        
        handle->file_offset -= handle->avail_size;
        
        handle->avail_size = 0;
    }
    
}


//Virtual Texture table

#define _tpage_side 128
#define _texturehandle_max 16

_persist u32 phys_w = 0;
_persist u32 phys_h = 0;


#define _fetch_dim_scale_w 8
#define _fetch_dim_scale_h 8

_persist VTextureContext global_texturecache = {};



_persist u16* vt_freepages_array = 0;
_persist u32 vt_freepages_count = 0;

_persist TextureAssetHandle texturehandle_array[_texturehandle_max] = {};
_persist u32 texturehandle_count = 0;


union VTReadbackPixelFormat{
    
    struct{
        u8 texture_id;
        u8 mip;
        u8 x;
        u8 y;  
    };
    
    u32 value;
    
};

struct VTReadbackImageContext : VImageContext{
    u16 w;
    u16 h;
};

//MARK:nvidia is ok with linear if it is not a storage image
_persist VTReadbackImageContext vt_readbackbuffer = {}; //device writes to this
_persist VImageMemoryContext vt_targetreadbackbuffer = {}; // we copy to this for reading 
_persist VTReadbackPixelFormat* vt_readbackpixels = 0;
_persist VTReadbackPixelFormat* threadtexturefetch_array = 0;


_persist VkCommandPool fetch_pool[2];
_persist VkCommandBuffer fetchcmdbuffer_array[2];
_persist u32 fetchcmdbuffer_count = 0;


_persist TextureAssetHandle* evict_texture_handle_array[_texturehandle_max * 2] = {};

_persist u32 evict_texture_handle_count = 0;

#ifdef DEBUG

_persist VTReadbackPixelFormat* debug_pixels = 0;

#endif

void InitAssetAllocator(ptrsize size,VkDeviceSize device_size,
                        u32 phys_w_tiles,u32 phys_h_tiles,VDeviceContext* _restrict vdevice,VSwapchainContext* swapchain){
    
    global_memstate.global_ptr = SSysAlloc(0,size,MEMPROT_READWRITE,0);
    
    auto gmemblock =
        &global_memstate.memoryblock_array[global_memstate.memoryblock_count];
    
    gmemblock->ptr = (s8*)global_memstate.global_ptr;
    gmemblock->size = size;
    gmemblock->higherblock = 0;
    gmemblock->lowerblock = 0;
    
    global_memstate.free_array[global_memstate.free_count] =
        &global_memstate.memoryblock_array[global_memstate.memoryblock_count];
    global_memstate.free_count++;
    global_memstate.memoryblock_count++;
    
    //this part is for device memory
    
    auto typeindex =
        VGetMemoryTypeIndex(*vdevice->phys_info->memoryproperties,0xFFFFFFFF,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    _allocprint("allocator typeindex %d\n",typeindex);
    
    global_gpumemstate.global_ptr =
        VRawDeviceAlloc(vdevice->device,device_size,typeindex);
    
    global_gpumemstate.memoryblock_array[global_gpumemstate.memoryblock_count] =
    {0,device_size,0};
    
    global_gpumemstate.free_array[global_gpumemstate.free_count] =
        &global_gpumemstate.memoryblock_array[global_gpumemstate.memoryblock_count];
    
    global_gpumemstate.free_count++;
    global_gpumemstate.memoryblock_count++;
    
    u32 transferbuffersize = _megabytes(16);
    
    main_transferbuffer = {VCreateTransferBuffer(vdevice,transferbuffersize),0,
        transferbuffersize};
    
    async_transferbuffer = {VCreateTransferBuffer(vdevice,transferbuffersize),0,
        transferbuffersize};
    
    
    
    global_device = vdevice;
    
    
    
    //init phys texture stuff
    {
        
        vt_freepages_count = phys_w_tiles * phys_h_tiles;
        vt_freepages_array = (u16*)alloc(sizeof(u16) * vt_freepages_count);
        
        auto max_index = vt_freepages_count - 1;
        
        for(u32 i = 0; i < vt_freepages_count; i++){
            
            vt_freepages_array[i] = max_index - i;
            
        }
        
        phys_w = phys_w_tiles * _tpage_side;
        phys_h = phys_h_tiles * _tpage_side;
    }
    
#if _usergba
    
    global_texturecache =
        VCreateTextureCache(vdevice,phys_w,phys_h,VK_FORMAT_R8G8B8A8_UNORM);
    
#else
    
    global_texturecache =
        VCreateTextureCache(vdevice,phys_w,phys_h,VK_FORMAT_BC1_RGB_UNORM_BLOCK);
    
#endif
    
    //transition to proper layout
    {
        
        auto queue = VGetQueue(vdevice,VQUEUETYPE_ROOT);
        
        auto pool =
            VCreateCommandPool(vdevice,
                               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                               VGetQueueFamilyIndex(VQUEUETYPE_ROOT));
        
        auto cmdbuffer = VAllocateCommandBuffer(vdevice,pool,
                                                VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        
        VStartCommandBuffer(cmdbuffer,
                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
        VkImageMemoryBarrier shader_barrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,//srcAccessMask
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            global_texturecache.image,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            }
        };
        
        vkCmdPipelineBarrier(cmdbuffer,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,0,0,0,1,&shader_barrier);
        
        VEndCommandBuffer(cmdbuffer);
        VSubmitCommandBuffer(queue,cmdbuffer);
        vkQueueWaitIdle(queue);
        vkDestroyCommandPool(vdevice->device,pool,0);
    }
    
    
#ifdef DEBUG
    
#if (_usergba)
    {
        //NOTE:Clear texturecache - Linux doesn't clear gpu memory by default. this is better for
        // debugging
        
        auto queue = VGetQueue(vdevice,VQUEUETYPE_ROOT);
        
        auto pool =
            VCreateCommandPool(vdevice,
                               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                               VGetQueueFamilyIndex(VQUEUETYPE_ROOT));
        
        auto cmdbuffer = VAllocateCommandBuffer(vdevice,pool,
                                                VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        
        VStartCommandBuffer(cmdbuffer,
                            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        
        VkImageMemoryBarrier transfer_membarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,//srcAccessMask
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            global_texturecache.image,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            },
        };
        
        vkCmdPipelineBarrier(cmdbuffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0,0,0,0,1,&transfer_membarrier);
        
        VkClearColorValue color = {};
        
        VkImageSubresourceRange range = {
            VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1
        };
        
        vkCmdClearColorImage(cmdbuffer,global_texturecache.image,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&color,1,&range);
        
        
        VEndCommandBuffer(cmdbuffer);
        
        VSubmitCommandBuffer(queue,cmdbuffer);
        
        vkQueueWaitIdle(queue);
        
        vkDestroyCommandPool(vdevice->device,pool,0);
    }
#endif
    
#endif
    
    {
        
        auto w = swapchain->width/_fetch_dim_scale_w;
        auto h = swapchain->height/_fetch_dim_scale_h;
        
        auto img = VCreateColorImage(vdevice,w,h,
                                     VK_IMAGE_USAGE_STORAGE_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        
        vt_readbackbuffer.image = img.image;
        vt_readbackbuffer.view = img.view;
        vt_readbackbuffer.memory = img.memory;
        vt_readbackbuffer.w = w;
        vt_readbackbuffer.h = h;
        
#ifdef DEBUG
        
        debug_pixels = (VTReadbackPixelFormat*)alloc(w * h * 4);
#endif
        
        //(FIXME:)MARK: do we really want to make this cached?
        vt_targetreadbackbuffer = VCreateColorImageMemory(vdevice,w,h,
                                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT,false,VMAPPED_NONE,
                                                          VK_IMAGE_TILING_LINEAR);
        
        VMapMemory(global_device,vt_targetreadbackbuffer.memory,
                   0,w * h * sizeof(VTReadbackPixelFormat),(void**)&vt_readbackpixels);
        
        threadtexturefetch_array = (VTReadbackPixelFormat*)alloc(w * h *
                                                                 sizeof(VTReadbackPixelFormat));
        
        
        //clear the image
#if 0
        {
            auto queue = VGetQueue(vdevice,VQUEUETYPE_ROOT);
            
            auto pool =
                VCreateCommandPool(vdevice,
                                   VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                   VGetQueueFamilyIndex(VQUEUETYPE_ROOT));
            
            auto cmdbuffer = VAllocateCommandBuffer(vdevice,pool,
                                                    VK_COMMAND_BUFFER_LEVEL_PRIMARY);
            
            VStartCommandBuffer(cmdbuffer,
                                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            
            VkImageMemoryBarrier transfer_membarrier = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                0,
                0,//srcAccessMask
                VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                vt_targetreadbackbuffer.image,
                {
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0,
                    1,
                    0,
                    1
                },
            };
            
            vkCmdPipelineBarrier(cmdbuffer,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,0,0,0,1,&transfer_membarrier);
            
            VkClearColorValue color = {};
            
            VkImageSubresourceRange range = {
                VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1
            };
            
            vkCmdClearColorImage(cmdbuffer,vt_targetreadbackbuffer.image,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&color,1,&range);
            
            
            VEndCommandBuffer(cmdbuffer);
            
            VSubmitCommandBuffer(queue,cmdbuffer);
            
            
            vkQueueWaitIdle(queue);
            
            vkDestroyCommandPool(vdevice->device,pool,0);
        }
        
#endif
    }
    
    
    
    for(u32 i = 0; i < _arraycount(fetch_pool); i++){
        fetch_pool[i] =
            VCreateCommandPool(vdevice,
                               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                               VGetQueueFamilyIndex(VQUEUETYPE_ROOT));
        
        fetchcmdbuffer_array[i] =
            VAllocateCommandBuffer(vdevice,fetch_pool[i],
                                   VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    }
    
}


void UpdateAllocatorTimeStamp(){
    TIMEBLOCK(Purple);
    global_timestamp += 2;
    ResetTAlloc();
}

void ResetTransferBuffer(){
    main_transferbuffer.offset = 0;
}

void ResetAsyncTransferBuffer(){
    async_transferbuffer.offset = 0;
}

const VTextureContext* GetTextureCache(){
    return &global_texturecache;
}


_ainline void InternalPageToPhysCoord(u32 page,u8* x,u8* y){
    //MARK: Hopefully the compiler is smart enough to fold this into a single div
    *y = page / (phys_w/_tpage_side);
    *x = page % (phys_w/_tpage_side);
}

_ainline u32 InternalPhysCoordToPage(u8 x,u8 y){
    return x + (y * (phys_w/_tpage_side));
}


void CommitTexture(TextureAssetHandle* handle){
    handle->timestamp = global_timestamp;
}



/*
next_active_coord = (active_coord * 2) + qcoord
*/

struct EvictCoord : TCoord{
    u16 page_value;
};

struct EvictList{
    EvictCoord array[_fetch_list_count];
    u32 count;
};

void InternalEvictTextureAllPagesTraverse
(TPageQuadNode* _restrict node,EvictList* list,
 u32 depth,u32 max_mip_level,Coord active_coord){
    
    if(node->page_value == (u32)-1){
        return;
    }
    
    
    Coord n_coord = {(u8)(active_coord.x * 2),(u8)(active_coord.y * 2)};
    
    
    if(node->first){
        
        Coord quad_coord = {0,0};
        
        n_coord.x += quad_coord.x;
        n_coord.y += quad_coord.y;
        
        InternalEvictTextureAllPagesTraverse(node->first,list,depth + 1,max_mip_level,n_coord);
    }
    
    
    if(node->second){
        
        Coord quad_coord = {0,1};
        
        n_coord.x += quad_coord.x;
        n_coord.y += quad_coord.y;
        
        InternalEvictTextureAllPagesTraverse(node->second,list,depth + 1,max_mip_level,n_coord);
    }
    
    
    if(node->third){
        
        Coord quad_coord = {1,0};
        
        n_coord.x += quad_coord.x;
        n_coord.y += quad_coord.y;
        
        InternalEvictTextureAllPagesTraverse(node->third,list,depth + 1,max_mip_level,n_coord);
    }
    
    
    if(node->fourth){
        
        Coord quad_coord = {1,1};
        
        n_coord.x += quad_coord.x;
        n_coord.y += quad_coord.y;
        
        InternalEvictTextureAllPagesTraverse(node->fourth,list,depth + 1,max_mip_level,n_coord);
    }
    
    
    //this make sure we don't evict the lowest mip
    if(depth){
        
        struct PixFormat{
            u8 x;
            u8 y;
            u8 lock;
            u8 pad;
        };
        
        auto f = (PixFormat*)&node->page_value;
        
        list->array[list->count].x = active_coord.x;
        list->array[list->count].y = active_coord.y;
        list->array[list->count].mip = max_mip_level - depth;
        list->array[list->count].page_value = InternalPhysCoordToPage(f->x,f->y);
        
        list->count++;
        
    }
    
}

//TODO: test this
void EvictAllTexturePages(TextureAssetHandle* _restrict handle,EvictList* list){
    
    auto node = &handle->pagetree;
    auto max_mip_level = handle->max_miplevel;
    
    InternalEvictTextureAllPagesTraverse
        (node,list,0,max_mip_level,{});
}

u32 InternalGetAvailablePage(EvictList* list){
    
    //MARK: I think we depend on this not moving
    if(!vt_freepages_count){
        
        TextureAssetHandle handle_array[_texturehandle_max] = {};
        
        memcpy(handle_array,handle_array,sizeof(TextureAssetHandle) * texturehandle_count);
        
        
        qsort(handle_array,texturehandle_count,
              sizeof(TextureAssetHandle),
              [](const void * a, const void* b)->s32 {
              
              auto t_a = (TextureAssetHandle*)a;
              auto t_b = (TextureAssetHandle*)b;
              
              return t_a->timestamp - t_b->timestamp;
              });
        
        for(u32 i = 0; i < texturehandle_count; i++){
            
            auto t = &handle_array[i];
            
            if(global_timestamp != t->timestamp){
                //evict texture
                EvictAllTexturePages(t,list);
            }
            
            if(list->count){
                break;
            }
            
        }
        
    }
    
    _kill("can't get any pages\n",!vt_freepages_count);
    
    vt_freepages_count--;
    u32 page = vt_freepages_array[vt_freepages_count];
    
    
    
    return page;
}

void InternalGetPageCoord(u8* x,u8* y,EvictList* list){
    
    auto page = InternalGetAvailablePage(list);
    InternalPageToPhysCoord(page,x,y);
}

void SetupQuadTree(AQuadNode*curnode,AQuadNode* space,u32 curlevel,u32 maxlevel){
    
    if(curlevel == maxlevel){
        curnode->children_array = 0;
        return;
    }
    
    curnode->children_array = space;
    space += 4;
    
    for(u32 i = 0; i < 4; i++){
        auto child = &curnode->children_array[i];
        SetupQuadTree(child,space,curlevel + 1,maxlevel);
    }
    
}

void SetupQuadTree(DQuadNode*curnode,DQuadNode* space,u32 curlevel,u32 maxlevel){
    
    if(curlevel == maxlevel){
        curnode->first = 0;
        curnode->second = 0;
        curnode->third = 0;
        curnode->fourth = 0;
        return;
    }
    
    curnode->first = space;
    space++;
    SetupQuadTree(curnode->first,space,curlevel + 1,maxlevel);
    
    curnode->second = space;
    space++;
    SetupQuadTree(curnode->second,space,curlevel + 1,maxlevel);
    
    curnode->third = space;
    space++;
    SetupQuadTree(curnode->third,space,curlevel + 1,maxlevel);
    
    curnode->fourth = space;
    space++;
    
    SetupQuadTree(curnode->fourth,space,curlevel + 1,maxlevel);
    
}

void SetupPageTree(TPageQuadNode*curnode,TPageQuadNode** space,u32 curlevel,
                   u32 maxlevel){
    
    *curnode = {};
    
    curnode->page_value = (u32)-1;
    
    if(curlevel == maxlevel){
        return;
    }
    
    curnode->first = *(space);
    *space = *(space) + 1;
    SetupPageTree(curnode->first,space,curlevel + 1,maxlevel);
    
    curnode->second = *(space);
    *space = *(space) + 1;
    SetupPageTree(curnode->second,space,curlevel + 1,maxlevel);
    
    curnode->third = *(space);
    *space = *(space) + 1;
    SetupPageTree(curnode->third,space,curlevel + 1,maxlevel);
    
    curnode->fourth = *(space);
    *space = *(space) + 1;
    SetupPageTree(curnode->fourth,space,curlevel + 1,maxlevel);
    
}

void DebugTree(TPageQuadNode*curnode,u32 counter = 0,u32 index = 0){
    
    if(curnode){
        // printf("%d\n",curnode->page_value);
        printf("%d(%d : %d) > ",curnode->page_value,index,counter);
        counter ++;
        DebugTree(curnode->first,counter,1);
        DebugTree(curnode->second,counter,2);
        DebugTree(curnode->third,counter,3);
        DebugTree(curnode->fourth,counter,4);
    }
    
    else{
        // printf("\nbreak(%d : %d)\n",index,counter);
        // printf("counter %d\n",counter);
        // printf("------------\n");
    }
    
}

//TODO: rename this cos we don't allocate anything anymore
MaterialAssetHandle AllocateAssetMaterial(VDeviceContext* _restrict vdevice){
    
    MaterialAssetHandle handle = {};
    
    return handle;
}


TextureAssetHandle* AllocateAssetTexture(const s8* filepath,
                                         const VDeviceContext* _restrict vdevice,VkCommandBuffer cmdbuffer){
    
    _kill("ran out of texture slots\n",texturehandle_count >= _arraycount(texturehandle_array));
    
    auto asset = &texturehandle_array[texturehandle_count];
    texturehandle_count++;
    
    asset->assetfile = (s8*)filepath;
    
    auto header = GetHeaderInfoTDF(filepath);
    
    asset->bpp = header.bpp;
    asset->w = header.w;
    asset->h = header.h;
    
    //Allocate data for pagetree
    
    auto tsize = 0;
    auto len = header.w >> 7;
    
    for(u32 i = 0; i < header.mips; i++){
        tsize+= len * len;
        len >>= 1;
    }
    
    auto treespace = (TPageQuadNode*)alloc((tsize) * sizeof(TPageQuadNode));
    
    auto cur = &asset->pagetree;
    
    u32 curlevel = 0;
    
    SetupPageTree(cur,&treespace,curlevel,header.mips - 1);
    
    // DebugTree(cur);
    
    asset->pagetable =
        VCreateTexturePageTable(vdevice,header.w,header.h,header.mips);    
    asset->max_miplevel = header.mips - 1;
    
    VkImageSubresourceRange range =     {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        header.mips,
        0,
        1
    };
    
    
    VkImageMemoryBarrier transfer_barrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        0,
        0,//srcAccessMask
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        asset->pagetable.image,
        range
    };
    
    vkCmdPipelineBarrier(cmdbuffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,0,0,0,1,&transfer_barrier);
    
    VkClearColorValue color = {1.0f,1.0f,1.0f,1.0f};
    
    vkCmdClearColorImage(cmdbuffer,asset->pagetable.image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,&color,1,&range);
    
    VkImageMemoryBarrier shader_barrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,//srcAccessMask
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        asset->pagetable.image,
        range
    };
    
    vkCmdPipelineBarrier(cmdbuffer,VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,0,0,0,1,&shader_barrier);
    
    return asset;
}

void UnallocateAssetTexture(TextureAssetHandle* handle){
    _kill("",1);
    unalloc(handle->pagetree.first);
    //TODO: we need to delete other stuff too
}

TCoord InternalToCoordAtMipLevel(TCoord src_coord,u32 dst_mip){
    
    TCoord c = {};
    
    c.x = src_coord.x >> (dst_mip - src_coord.mip);
    c.y = src_coord.y >> (dst_mip - src_coord.mip);
    c.mip = dst_mip;
    
#ifdef DEBUG
    
#if  0
    
    printf("a: %d %d %d\n",c.mip,c.x,c.y);
    
#endif
    
#endif
    
    return c;
}

Coord InternalToQuadCoord(TCoord dst_coord,TCoord prevdst_coord){
    
    Coord c = {
        (u8)(dst_coord.x - (prevdst_coord.x << 1)),
        (u8)(dst_coord.y - (prevdst_coord.y << 1))
    };
    
#ifdef DEBUG
    
#if  0
    printf("q: %d %d\n",c.x,c.y);
#endif
    
#endif
    
    return c;
}

/*
  NOTE: 
  We have 3 sets of coordinates:
  src coord - coord at the src mip level
  dst coord - coord at the dst mip level (derived from src coord)
  quad coord - coord of the tile in the quad (derived from next src coord)
  
   a higher mip can generate coords for a lower mip,but not the other way around
*/
void InternalTraverseMipTree(TPageQuadNode* node,TCoord src_coord,
                             TCoord dst_coord,FetchList* list,EvictList* evict_list){
    
    if(node->page_value == (u32)-1){
        
        _kill("not enough fetch space\n",list->count >=
              _arraycount(list->array));
        
        auto fetch = &list->array[list->count];
        
        fetch->src_coord.mip = dst_coord.mip;
        fetch->src_coord.x = dst_coord.x;
        fetch->src_coord.y = dst_coord.y;
        
#if  0
        printf("f %d %d %d %p :: ",dst_coord.mip,dst_coord.x,dst_coord.y,(void*)node);
        printf("incoord %d %d %d\n",src_coord.mip,src_coord.x,src_coord.y);
#endif
        
        InternalGetPageCoord(&fetch->dst_coord.x,&fetch->dst_coord.y,evict_list);
        list->count++;
        node->page_value = _encode_rgba(fetch->dst_coord.x,fetch->dst_coord.y,0,255);
    }
    
#ifdef DEBUG
    
#if 0
    
    else{
        printf("rejected %d %d %d\n",dst_coord.mip,dst_coord.x,dst_coord.y);
    }
    
#endif
    
#endif
    
    if(dst_coord.mip == src_coord.mip){
        return;
    }
    
    //computing the next dst_coord
    auto next_dst_coord = InternalToCoordAtMipLevel(src_coord,dst_coord.mip - 1);
    
    //computing the coord in quad space
    auto q_coord = InternalToQuadCoord(next_dst_coord,dst_coord);
    
    TPageQuadNode* nextnode = 0;
    
    if(q_coord.y < (1)){
        
        if(q_coord.x < (1)){
            nextnode = node->first;
        }
        else{
            nextnode = node->second;
        }
        
    }
    
    else{
        
        if(q_coord.x < (1)){
            nextnode = node->third;
        }
        else{
            nextnode = node->fourth;
        }
        
    }
    
    
    if(nextnode){
        InternalTraverseMipTree(nextnode,src_coord,next_dst_coord,list,evict_list);
    }
    
}

void InternalGenerateDependentCoords(TextureAssetHandle* asset,u8 mip,u8 x,u8 y,
                                     FetchList* list,EvictList* evict_list){
    
    TCoord src = {};
    
    src.x = x;
    src.y = y;
    src.mip = mip;
    
    auto dst = InternalToCoordAtMipLevel(src,asset->max_miplevel);
    
    auto node = &asset->pagetree;
    
    InternalTraverseMipTree(node,src,dst,list,evict_list);
}



//MARK: start here (Get vt fetch working again and work on vt evict next)
u32 GenTextureFetchList(TextureAssetHandle* asset,VTReadbackPixelFormat* src_coords,
                        u32 count,FetchList* list,EvictList* evict_list){
    
    
    TIMEBLOCK(Purple);
    
    for(u32 i = 0; i < count; i++){
        
        auto a = &src_coords[i];
        
        InternalGenerateDependentCoords(asset,a->mip,a->x,a->y,list,evict_list);
    }
    
    return list->count;
}

void FetchTextureTiles(ThreadFetchBatch* batch,VkCommandBuffer fetch_cmdbuffer){
    
    //MARK:printf
    printf("async alloc %d\n",(u32)async_transferbuffer.offset);
    
    
    if(batch->fetchlist.count){
        
        _kill("over possible fetch tiles array\n",batch->fetchlist.count >= 21845);
        
        VkBufferImageCopy imagecopy_array[21845];
        
        VkBufferImageCopy pagecopy_array[21845];
        
        auto file = FOpenFile(batch->assetfile,F_FLAG_READONLY);
        
        {
            
            TIMEBLOCK(Purple);
            
            u32 tile_size = (u32)(batch->bpp * _tpage_side * _tpage_side);
            u32 total_size = batch->fetchlist.count * tile_size;
            
            auto transferbuffer = async_transferbuffer.buffer;
            auto transferbuffer_offset =
                AllocateTransferBuffer(&async_transferbuffer,
                                       total_size + (sizeof(u32) * batch->fetchlist.count));
            
            {
                
                TIMEBLOCK(DarkBlue);
                
                s8* mappedmemory_ptr;
                
                VMapMemory(global_device->device,transferbuffer.memory,transferbuffer_offset,
                           total_size + (sizeof(u32) * batch->fetchlist.count),(void**)&mappedmemory_ptr);
                
                //Copy image data here
                for(u32 i = 0; i < batch->fetchlist.count; i++){
                    
                    auto cur = mappedmemory_ptr + (tile_size * i);
                    
                    auto fetch_data = &batch->fetchlist.array[i];
                    
                    auto mip = fetch_data->src_coord.mip;
                    auto x = fetch_data->src_coord.x;
                    auto y = fetch_data->src_coord.y;
                    
#if  0
                    if(batch->assetfile == texturehandle_array[0].assetfile)
                        printf("s %s %d %d %d\n",batch->assetfile,mip,x,y);
#endif
                    
                    
                    GetTileDataTDF(file,batch->w,batch->h,x,y,mip,batch->bpp,cur);
                    
                }
                
                //copy page table data
                for(u32 i = 0; i < batch->fetchlist.count; i++){
                    auto cur = ((u32*)(mappedmemory_ptr + total_size)) + i;
                    auto fetch_data = &batch->fetchlist.array[i];
                    
                    auto x = fetch_data->dst_coord.x;
                    auto y = fetch_data->dst_coord.y;
                    
                    *cur = _encode_rgba(x,y,0,255);
                }
                
                vkUnmapMemory(global_device->device,transferbuffer.memory);
            }
            
            {
                TIMEBLOCK(Salmon);
                
                for(u32 i = 0; i < batch->fetchlist.count; i++){
                    
                    //Transfer image data
                    
                    auto data_copy = &imagecopy_array[i];
                    auto page_copy = &pagecopy_array[i];
                    auto fetch_data = &batch->fetchlist.array[i];
                    
                    auto src_mip = (u32)(fetch_data->src_coord.mip);
                    
                    auto dst_x = (u32)(fetch_data->dst_coord.x);
                    auto dst_y = (u32)(fetch_data->dst_coord.y);
                    
                    auto mip_w = batch->w;
                    auto mip_h = batch->h;
                    
                    for(u32 j = 0; j < src_mip; j++){
                        mip_w >>= 1;
                        mip_h >>= 1;
                    }
                    
                    //copies image data
                    data_copy->bufferOffset = transferbuffer_offset + (tile_size * i);
                    
                    data_copy->bufferRowLength = 0;
                    data_copy->bufferImageHeight = 0;
                    data_copy->imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
                    data_copy->imageOffset = {(int32_t)(dst_x * _tpage_side),(int32_t)(dst_y * _tpage_side),0};
                    data_copy->imageExtent = {_tpage_side,_tpage_side,1};
                    
                    _kill("coord exceeds texture bounds\n",fetch_data->src_coord.x >= batch->w ||
                          fetch_data->src_coord.y >= batch->h);
                    
                    //copies page data
                    page_copy->bufferOffset = transferbuffer_offset + total_size + (i * sizeof(u32));
                    page_copy->bufferRowLength = 0;
                    page_copy->bufferImageHeight = 0;
                    page_copy->imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT,src_mip,0,1};
                    page_copy->imageOffset = {(int32_t)(fetch_data->src_coord.x),
                        (int32_t)(fetch_data->src_coord.y),0};
                    page_copy->imageExtent = {1,1,1};
                }
                
            }
            
            vkCmdCopyBufferToImage(fetch_cmdbuffer,transferbuffer.buffer,
                                   global_texturecache.image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   batch->fetchlist.count,imagecopy_array);
            
            vkCmdCopyBufferToImage(fetch_cmdbuffer,transferbuffer.buffer,
                                   batch->pagetable.image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   batch->fetchlist.count,pagecopy_array);
            
            transferbuffer_offset += total_size + (sizeof(u32) * batch->fetchlist.count);
            
        }
        
        FCloseFile(file);
    }
    
}


void PushThreadTextureFetchQueue(ThreadTextureFetchQueue* queue,
                                 TextureAssetHandle* asset,FetchList* list,TSemaphore sem){
    
    _kill("too many entries\n",queue->count >= _FetchqueueSize);
    
#if  0
    {
        static u32 count = 0;
        printf("%d pushed %s\n",count,asset->assetfile);
        count++;
    }
#endif
    
    auto t = &queue->buffer[queue->count];
    
    t->w = asset->w;
    t->h = asset->h;
    t->bpp = asset->bpp;
    t->assetfile = asset->assetfile;
    t->pagetable = asset->pagetable;
    
    t->fetchlist.count = list->count;
    t->total_miplevel = asset->max_miplevel + 1;
    
    memcpy(t->fetchlist.array,list->array,
           sizeof(FetchData) * list->count);
    
    queue->count++;
    
    TSignalSemaphore(sem);
}


//TODO: have a threaded eviction too
void ExecuteThreadTextureFetchQueue(ThreadTextureFetchQueue* queue){
    
    if(queue->index == queue->count){
        return;
    }
    
    u32 tfetch_count = 0;
    
    while(queue->index != queue->count){
        
        auto batch = &queue->buffer[queue->index];
        
        _kill("invalid asset texture\n",!batch->pagetable.image || !batch->assetfile);
        
#if  0
        {
            static u32 count = 0;
            printf("%d fetched %s\n",count,batch->assetfile);
            count++;
        }
        
#endif
        
        VkImageMemoryBarrier transfer_imagebarrier =  {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            batch->pagetable.image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,(batch->total_miplevel),0,1}  
        };
        
        VkImageMemoryBarrier shader_imagebarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            batch->pagetable.image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,(batch->total_miplevel),0,1}
        };
        
        vkCmdPipelineBarrier(queue->cmdbuffer,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,0,0,0,1,
                             &transfer_imagebarrier);
        
        FetchTextureTiles(batch,queue->cmdbuffer);
        
        tfetch_count += batch->fetchlist.count;
        
        vkCmdPipelineBarrier(queue->cmdbuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,0,0,0,1,&shader_imagebarrier);
        
        queue->index ++;
        
    }
    
    
    queue->fetch_count = tfetch_count;
    
    queue->is_done = true;
    
}

void MaterialAddTexture(MaterialAssetHandle* handle,TextureType type,
                        u8 texture_index){
    
    _kill("too many data entries\n",handle->textureid_count >=
          _arraycount(handle->textureid_array));
    
    handle->textureid_array[handle->textureid_count] = texture_index;
    handle->textureid_count++;
}


VImageContext GetVTReadbackBuffer(){
    return vt_readbackbuffer;
}


void UpdateTextureFetchEntries(){
    
    TIMEBLOCK(Green);
    
    VMemoryRangesArray ranges = {};
    
    VPushBackMemoryRanges(&ranges,vt_targetreadbackbuffer.memory,
                          0, VK_WHOLE_SIZE);
    
    VInvalidateMemoryRanges(global_device,&ranges);
}


void DebugRenderFeedbackBuffer(){
    
    u32 total_tiles = vt_readbackbuffer.w * vt_readbackbuffer.h;
    
    for(u32 i = 0; i < total_tiles; i++)
    {
        
        auto a = &vt_readbackpixels[i];
        
        if(a->texture_id && (a->texture_id - 1) >= _arraycount(texturehandle_array)){
            
            printf("hit invalid %d\n",a->texture_id);
            a->value = _encode_rgba(255,0,0,255);
        }
        
        else if(a->texture_id)
        {
            a->value = (u32)-1;
            
        }
        
        else
        {
            a->value = 0;
        }
        
    }
    
    printf("Error: invalid tid generated\n");
    
    WriteBMP(vt_readbackpixels,vt_readbackbuffer.w,vt_readbackbuffer.h,
             "test.bmp");
    
    _kill("",1);
    
}



//we do not do temp page allocations anymore
VkCommandBuffer GenerateTextureFetchRequests(
ThreadTextureFetchQueue* fetchqueue,TSemaphore sem){
    
    TIMEBLOCK(Red);
    
    UpdateTextureFetchEntries();
    
#if 0
    
    {
        
        u32 total_tiles = vt_readbackbuffer.w * vt_readbackbuffer.h;
        u32 count = 0;
        
        static u32 hit_count = 0;
        
        if(hit_count){
            
            for(u32 i = 0; i < total_tiles; i++){
                
                auto a = &vt_readbackpixels[i];
                
                if(a->texture_id && (a->texture_id - 1) >= _arraycount(texturehandle_array)){
                    
                    a->value = _encode_rgba(255,0,0,255);
                }
                
                else if(a->texture_id){
                    a->value = (u32)-1;
                    
                }
                
                else{
                    a->value = _encode_rgba(0,0,255,255);
                }
                
            }
            
            WriteBMP(vt_readbackpixels,vt_readbackbuffer.w,vt_readbackbuffer.h,
                     "test.bmp");
            
            exit(0);
            
        }
        
        hit_count++;
        
    }
    
    
    
#endif  
    
#ifdef DEBUG 
    
    {
        
        memcpy(debug_pixels,vt_readbackpixels,
               vt_readbackbuffer.w * vt_readbackbuffer.h * 4);
    }
    
#endif
    
    
    u32 total_tiles = vt_readbackbuffer.w * vt_readbackbuffer.h;
    u32 count = 0;
    
    for(u32 i = 0; i < total_tiles; i++){
        
        auto a = &vt_readbackpixels[i];
        
        if(a->texture_id){
            
#ifdef DEBUG
            
            if(a->texture_id >= _arraycount(texturehandle_array)){
                DebugRenderFeedbackBuffer();
            }
            
#endif
            
            a->texture_id--;
            threadtexturefetch_array[count] = *a;
            count++;
            
        }
        
    }
    
    qsort(threadtexturefetch_array,count,sizeof(u32),
          [](const void * a, const void* b)->s32 {
          
          auto a_t = ((VTReadbackPixelFormat*)a)->texture_id;
          auto b_t = ((VTReadbackPixelFormat*)b)->texture_id;
          
          return a_t - b_t;
          });
    
    
    struct TAssetEntry{
        TextureAssetHandle* asset;
        u32 offset;
        u32 count;
    };
    
    TAssetEntry entry_array[16] = {};
    u32 entry_count = 0;
    //this locks in all the textures we are gonna use first and narrows down the a list of assets
    {
        
        TextureAssetHandle* active = 0;
        u32 active_index = 0;
        
        for(u32 i = 0; i < count; i++){
            
            auto tid = threadtexturefetch_array[i].texture_id;
            
#ifdef DEBUG
            
            if(tid >= _arraycount(texturehandle_array)){
                
                printf("got invalid tid %d\n",tid);
                
                DebugRenderFeedbackBuffer();
                
                
            }
            
            
#endif
            
            auto cur = &texturehandle_array[tid];
            
            if(active != cur){
                
                _kill("too many entries\n",entry_count >= _arraycount(entry_array));
                
                if(active){
                    entry_array[entry_count] = {active,active_index,i - active_index};
                    entry_count++;
                }
                
                active = cur;
                active_index = i;
                CommitTexture(active);
            }
            
        }
        
        if(active_index != count){
            entry_array[entry_count] = {active,active_index,count - active_index};
            entry_count++;
        }
        
        
    }
    
    VkCommandBuffer resolve_cmdbuffer = fetchqueue->cmdbuffer;
    
    auto is_done = IsThreadTextureFetchQueueDone(fetchqueue);
    auto tfetch_count = fetchqueue->fetch_count;
    
    if(is_done){
        
        auto  commit_cmdbuffer = fetchcmdbuffer_array[fetchcmdbuffer_count];
        
        fetchcmdbuffer_count++;
        
        if(fetchcmdbuffer_count >= _arraycount(fetchcmdbuffer_array)){
            fetchcmdbuffer_count = 0;  
        }
        
        SetCmdBufferThreadTextureFetchQueue(fetchqueue,commit_cmdbuffer);
        
        VStartCommandBuffer(fetchqueue->cmdbuffer,0,0,0,0,0,0,0);
        
        VkImageMemoryBarrier transfer_tcacheimagebarrier =     {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            GetTextureCache()->image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}  
        };
        
        vkCmdPipelineBarrier(fetchqueue->cmdbuffer,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,0,0,0,1,
                             &transfer_tcacheimagebarrier);
        
        Clear(fetchqueue);
    }
    
    //this begins page allocations
    {
        
        for(u32 i = 0; i < entry_count; i++){
            
            auto entry = &entry_array[i];
            FetchList list = {};
            EvictList evict_list = {};
            
            GenTextureFetchList(entry->asset,&threadtexturefetch_array[entry->offset],entry->count,
                                &list,&evict_list);
            
            
            if(list.count){
                
                PushThreadTextureFetchQueue(fetchqueue,entry->asset,&list,sem);	
            }
            
        }
        
    }
    
    if(is_done && resolve_cmdbuffer){
        
        VkImageMemoryBarrier shader_tcacheimagebarrier =     {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            GetTextureCache()->image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}
        };
        
        vkCmdPipelineBarrier(resolve_cmdbuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,0,0,0,0,0,1,
                             &shader_tcacheimagebarrier);
        
        VEndCommandBuffer(resolve_cmdbuffer);
        
        if(tfetch_count){
            return resolve_cmdbuffer;
        }
        
    }
    
    return 0;
}


VkSpecializationInfo VTFragmentShaderSpecConst(){
    
    u32 count = 7;
    auto entry_array = TAlloc(VkSpecializationMapEntry,count);
    auto pdata = TAlloc(f32,count);
    
    pdata[0] = (f32)vt_readbackbuffer.w * _fetch_dim_scale_w;
    pdata[1] = (f32)vt_readbackbuffer.h * _fetch_dim_scale_h;
    pdata[2] = (f32)vt_readbackbuffer.w;
    pdata[3] = (f32)vt_readbackbuffer.h;
    pdata[4] = (f32)_tpage_side;
    pdata[5] = (f32)phys_w;
    pdata[6] = (f32)phys_h;
    
#if 0
    
    printf("%f %f %f %f %f %f %f\n",pdata[0],pdata[1],pdata[2],pdata[3],pdata[4],pdata[5],
           pdata[6]);
#endif
    
    u32 offset = 0;
    
    for(u32 i = 0; i < count; i++){
        entry_array[i] = {i,offset,sizeof(f32)};
        offset += sizeof(f32);
    }
    
    VkSpecializationInfo info = {count,entry_array,offset,pdata};
    return info;
}


void VTStart(VkCommandBuffer cmdbuffer){
    
    VkImageMemoryBarrier clear_barrier[] = {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            GetVTReadbackBuffer().image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}
        }
    };
    
    VkImageSubresourceRange vtreadback_range = {
        VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1
    };
    
    vkCmdPipelineBarrier(cmdbuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0,0,0,0,_arraycount(clear_barrier),&clear_barrier[0]);
    
    VkClearColorValue vt_clearcolor = {};
    
    vkCmdClearColorImage(cmdbuffer,
                         GetVTReadbackBuffer().image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         &vt_clearcolor,
                         1,&vtreadback_range);
    
    VkImageMemoryBarrier layout_barrier[] = {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            GetVTReadbackBuffer().image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}
        }
    };
    
    vkCmdPipelineBarrier(cmdbuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0,0,0,0,_arraycount(layout_barrier),&layout_barrier[0]);
}

void VTEnd(VkCommandBuffer cmdbuffer){
    
    VkImageMemoryBarrier copy_barrier[] = {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            GetVTReadbackBuffer().image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}
        },
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            vt_targetreadbackbuffer.image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}
        }
    };
    
    vkCmdPipelineBarrier(cmdbuffer,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0,0,0,0,_arraycount(copy_barrier),&copy_barrier[0]);
    
    
    VkImageCopy copy_array[] = {
        {
            {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},
            {},
            {VK_IMAGE_ASPECT_COLOR_BIT,0,0,1},
            {},
            {vt_readbackbuffer.w,vt_readbackbuffer.h,1}
        }
    };
    
    vkCmdCopyImage(cmdbuffer,vt_readbackbuffer.image,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   vt_targetreadbackbuffer.image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   1,&copy_array[0]);
    
    VkImageMemoryBarrier  layout_barrier[] = {
        {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_HOST_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            vt_targetreadbackbuffer.image,
            {VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1}
        }    
    };
    
    vkCmdPipelineBarrier(cmdbuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_HOST_BIT,
                         0,
                         0,0,0,0,_arraycount(layout_barrier),&layout_barrier[0]);
}


void VTEvictTextureHandlePages(TextureAssetHandle* handle){
    
    auto index = TGetEntryIndex(&evict_texture_handle_count,_arraycount(evict_texture_handle_array));
    
    evict_texture_handle_array[index] = handle;
}

void VTExecuteEvictTextureHandlePages(VkCommandBuffer cmdbuffer){}
