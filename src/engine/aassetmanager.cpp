#include "aassetmanager.h"
#include "ssys.h"
#include "audio_util.h"

/*
TODO:

Fresh start, we should diff this against the working implementation and patch in eviction.
TODO: use a ringbuffer for transferbuffers
*/

_global VDeviceContext* global_device = {};

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
    VkDeviceSize global_ptr_offset;
    
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


_global MemoryState global_memstate = {};
_global GPUMemState global_gpumemstate = {};

_global TIMESTAMP global_timestamp = 0xFFFFFFFF;

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

#define _DIsInListFn(slot_type,block_type) b32 IsInList##block_type(slot_type* list,u32 count, \
block_type* block){ \
    for(u32 i = 0; i < count; i++){ \
        if(block == list[i].block){ \
            return 1; \
        } \
    } \
    return 0; \
} \
b32 IsInList##block_type(block_type** list,u32 count,block_type* block){ \
    for(u32 i = 0; i < count; i++){ \
        if(block == list[i]){ \
            return 1; \
        } \
    } \
    return 0; \
}

#define _DMergeBlockFn(type) b32 MergeBlock##type(type* a,type* b){	\
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
b32 LinkAllocateBlockList##block_type(slot_type* list,u32* count,u32 size,slot_type* slot){	\
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
_intern void Defrag(){
    
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
    
    vkBindBufferMemory(vdevice->device,a->buffer,global_gpumemstate.global_ptr,a->ptr + global_gpumemstate.global_ptr_offset);
    vkBindBufferMemory(vdevice->device,b->buffer,global_gpumemstate.global_ptr,b->ptr + global_gpumemstate.global_ptr_offset);
    
}

//TODO: I don't trust this 
_intern void GPUDefrag(const  VDeviceContext* _restrict vdevice,
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

_intern void AllocateAsset(AssetHandle* handle,u32 size){
    
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
        
        Defrag();//TODO: Step through this - We have to replace this too
        
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
    
    memset(handle->ptr,0,allocsize);
}

_intern void GPUAllocateAsset(ModelAssetHandle* handle,u32 size){
    
    TIMEBLOCK(Silver);
    
    GPUMemorySlot slot = {};
    
    auto allocsize = size;
    
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
                           global_gpumemstate.global_ptr,handle->gpuptr + global_gpumemstate.global_ptr_offset);
        
        vkBindBufferMemory(global_device->device,handle->indexbuffer.buffer,
                           global_gpumemstate.global_ptr,
                           handle->gpuptr + handle->vertexbuffer.size + global_gpumemstate.global_ptr_offset);
        
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
    memcpy(handle.assetfile,filepath,strlen(filepath));
    
    u32 size = _fixed_audio_size;
    
    AllocateAsset((AssetHandle*)&handle,size);
    
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
    
    memcpy(animbone->assetfile,filepath,strlen(filepath));
    memcpy(vertindex->assetfile,filepath,strlen(filepath));
    
    u32 vertindex_size = 0;
    u32 animbone_size = 0;
    
    LoadMDF(filepath,0,0,&vertindex_size,&animbone_size);
    
    vertindex->ptr = TAlloc(s8,vertindex_size);
    
    AllocateAsset((AssetHandle*)animbone,animbone_size);
    
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
    
    auto vertsize = mdf.vertex_size;
    auto indexsize = mdf.index_size;
    
    GPUAllocateAsset(vertindex,vertsize + indexsize);
    
    vertindex->vertexbuffer =
        VCreateStaticVertexBuffer(vdevice,mdf.vertex_size,global_gpumemstate.global_ptr,
                                  vertindex->gpuptr + global_gpumemstate.global_ptr_offset,vertexbinding_no);
    
    vertindex->indexbuffer =
        VCreateStaticIndexBufferX(vdevice,global_gpumemstate.global_ptr,
                                  vertindex->gpuptr + mdf.vertex_size + global_gpumemstate.global_ptr_offset,
                                  mdf.index_size);
    
    
    auto ptr = VGetTransferBufferPtr(mdf.vertex_size + mdf.index_size);
    
    memcpy(ptr,mdf.vertex_data,mdf.vertex_size);
    memcpy(ptr + mdf.vertex_size,mdf.index_data,mdf.index_size);
    
    {
        VBufferCopy copy = {};
        
        VPushBackCopyBuffer(ptr,&copy,0,mdf.vertex_size);
        VCmdCopyBuffer(commandbuffer,vertindex->vertexbuffer.buffer,&copy);
    }
    
    {
        VBufferCopy copy = {};
        
        VPushBackCopyBuffer(ptr + mdf.vertex_size,&copy,0,mdf.index_size);
        VCmdCopyBuffer(commandbuffer,vertindex->indexbuffer.buffer,&copy);
    }
    
    
    vertindex->ptr = 0;
}


void AllocateAssetModel(const s8* filepath,
                        const VDeviceContext* _restrict vdevice,VkQueue queue,
                        VkCommandBuffer commandbuffer,u32 vertexbinding_no,ModelAssetHandle* vertindex){
    
    *vertindex = {};
    vertindex->animation_id = (u32)-1;
    memcpy(vertindex->assetfile,filepath,strlen(filepath));
    u32 vertindex_size = 0;
    
    LoadMDF(filepath,0,0,&vertindex_size,0);
    
    vertindex->ptr = TAlloc(s8,vertindex_size);
    
    auto mdf = LoadMDF(filepath,vertindex->ptr,vertindex->ptr,0,0);
    
    _kill("model loaded is not static\n",mdf.vertex_component != 3);
    
    vertindex->vert_component = mdf.vertex_component;
    
    vertindex->vert_fileoffset = mdf.vertexdata_offset;
    vertindex->index_fileoffset = mdf.indexdata_offset;
    
    auto vertsize = mdf.vertex_size;
    auto indexsize = mdf.index_size;
    
    GPUAllocateAsset(vertindex,vertsize + indexsize);
    
    vertindex->vertexbuffer =
        VCreateStaticVertexBuffer(vdevice,mdf.vertex_size,global_gpumemstate.global_ptr,
                                  vertindex->gpuptr + global_gpumemstate.global_ptr_offset,vertexbinding_no);
    
    vertindex->indexbuffer =
        VCreateStaticIndexBufferX(vdevice,global_gpumemstate.global_ptr,vertindex->gpuptr + mdf.vertex_size + global_gpumemstate.global_ptr_offset,mdf.index_size);
    
    
    auto ptr = VGetTransferBufferPtr(mdf.vertex_size + mdf.index_size);
    
    memcpy(ptr,mdf.vertex_data,mdf.vertex_size);
    memcpy(ptr + mdf.vertex_size,mdf.index_data,mdf.index_size);
    
    {
        VBufferCopy copy = {};
        
        VPushBackCopyBuffer(ptr,&copy,0,mdf.vertex_size);
        VCmdCopyBuffer(commandbuffer,vertindex->vertexbuffer.buffer,&copy);
    }
    
    {
        VBufferCopy copy = {};
        
        VPushBackCopyBuffer(ptr + mdf.vertex_size,&copy,0,mdf.index_size);
        VCmdCopyBuffer(commandbuffer,vertindex->indexbuffer.buffer,&copy);
    }
    
    vertindex->ptr = 0;
}

void CommitModel(ModelAssetHandle* handle,VkCommandBuffer cmdbuffer){
    
    if(!GpuCheckAsset(handle)){
        
        _breakpoint();
        
        
        if(!CheckAsset((AssetHandle*)handle)){
            
            AllocateAsset((AssetHandle*)handle,
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
        
        
        GPUAllocateAsset(handle,(handle->vertexbuffer.size + handle->indexbuffer.size));
        
        //bind and transfer vertex and index buffer
        
        vkBindBufferMemory(global_device->device,handle->vertexbuffer.buffer,
                           global_gpumemstate.global_ptr,handle->gpuptr +  + global_gpumemstate.global_ptr_offset);
        
        vkBindBufferMemory(global_device->device,handle->indexbuffer.buffer,
                           global_gpumemstate.global_ptr,handle->gpuptr + handle->vertexbuffer.size + global_gpumemstate.global_ptr_offset);
        
        //copy data to transfer buffer
        
        s8* ptr = VGetTransferBufferPtr(handle->vertexbuffer.size + handle->indexbuffer.size);
        
        memcpy(ptr,vert,handle->vertexbuffer.size);
        memcpy(ptr + handle->vertexbuffer.size,ind,handle->indexbuffer.size);
        
        {
            VBufferCopy copy = {};
            
            VPushBackCopyBuffer(ptr,&copy,0,handle->vertexbuffer.size);
            VCmdCopyBuffer(cmdbuffer,handle->vertexbuffer.buffer,&copy);
        }
        
        {
            VBufferCopy copy = {};
            
            VPushBackCopyBuffer(ptr + handle->vertexbuffer.size,&copy,0,handle->indexbuffer.size);
            VCmdCopyBuffer(cmdbuffer,handle->indexbuffer.buffer,&copy);
        }
        
        UnallocateAsset((AssetHandle*)handle);
    }
    
}

void CommitAnimated(AnimatedAssetHandle* handle){
    
    if(!CheckAsset((AssetHandle*)handle)){
        //TODO: Untested but should work
        AllocateAsset((AssetHandle*)handle,handle->animbonesize);
        
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
        
        u32 size = _fixed_audio_size;
        
        AllocateAsset((AssetHandle*)handle,size);
        
        handle->file_offset -= _frames2bytes_s16(handle->avail_frames);
        handle->avail_frames = 0;
    }
    
}

void UpdateAllocatorTimeStamp(){
    TIMEBLOCK(Purple);
    global_timestamp += 2;
    ResetTAlloc();
}


void CommitTexture(TextureAssetHandle* handle){
    handle->timestamp = global_timestamp;
}




void UnallocateAssetTexture(TextureAssetHandle* handle){
    _kill("",1);
    unalloc(handle->pagetree.first);
    //TODO: we need to delete other stuff too
}



MaterialAssetHandle* AddAssetMaterial(){
    auto mat = &pdata->scenecontext.materialasset_array[pdata->scenecontext.materialasset_count];
    pdata->scenecontext.materialasset_count++;
    return mat;
}



void MaterialAddTexture(MaterialAssetHandle* handle,TextureType type,
                        u8 texture_index){
    
    _kill("too many data entries\n",handle->textureid_count >=
          _arraycount(handle->textureid_array));
    
    handle->textureid_array[handle->textureid_count] = texture_index;
    handle->textureid_count++;
}




//vt vars and structs
#include "vt_vars.hpp"


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


_intern u32 GetTotalNodes(u32 total_mips){
    
    u32 total = 0;
    
    for(u32 i = 0; i < total_mips; i++){
        
        u32 len = 1 << i;
        
        total += len * len;
    }
    
    return total;
};

//separated it becase vt is a whole other system
#include "vt.cpp"



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
    
    
    
    
    {
        VDeviceMemoryBlockAlloc(device_size,&global_gpumemstate.global_ptr,&global_gpumemstate.global_ptr_offset);
    }
    
    
    
    global_gpumemstate.memoryblock_array[global_gpumemstate.memoryblock_count] =
    {0,device_size,0};
    
    global_gpumemstate.free_array[global_gpumemstate.free_count] =
        &global_gpumemstate.memoryblock_array[global_gpumemstate.memoryblock_count];
    
    global_gpumemstate.free_count++;
    global_gpumemstate.memoryblock_count++;
    
    global_device = vdevice;
    
    
    
    InitVTSubsystem(phys_w_tiles,phys_h_tiles,vdevice,swapchain);
    
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


const VTextureContext* GetTextureCache(){
    return &global_texturecache;
}

TextureAssetHandle* AllocateAssetTexture(const s8* filepath,
                                         const VDeviceContext* _restrict vdevice,VkCommandBuffer cmdbuffer){
    
    _kill("ran out of texture slots\n",texturehandle_count >= _arraycount(texturehandle_array));
    
    auto asset = &texturehandle_array[texturehandle_count];
    texturehandle_count++;
    
    memcpy(asset->assetfile,filepath,strlen(filepath));
    
    auto header = GetHeaderInfoTDF(filepath);
    
    asset->bpp = header.bpp;
    asset->w = header.w;
    asset->h = header.h;
    
    //Allocate data for pagetree
    auto total_nodes = GetTotalNodes(header.mips) - 1;
    
    auto treespace = (TPageQuadNode*)alloc((total_nodes) * sizeof(TPageQuadNode));
    
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


VImageContext GetVTReadbackBuffer(){
    return vt_readbackbuffer;
}

void ReadAudioAssetData(AudioAssetHandle* _restrict handle,b32 islooping,u32* _restrict is_done,u32 submit_frames,f32 factor){
    
    CommitAudio(handle);
    
    
    if(submit_frames > handle->avail_frames){
        
        auto cur_frames = handle->avail_frames;
        auto inv_factor = (1.0f/factor);
        auto required_bytes = 
            (u32)((f32)(_fixed_audio_read_size - _frames2bytes_s16(handle->avail_frames)) * inv_factor + 0.5f);
        
        u32 buffer_size = required_bytes << 1;//cos we will expand to f32
        auto buffer = TAlloc(s8,buffer_size);
        
        u32 offset = 0;
        
        u32 readsize = required_bytes;
        
        if((handle->file_offset + readsize) > handle->file_size){
            
            u32 remaining = handle->file_size - handle->file_offset;
            offset = remaining;
            
            
            ADFGetData(handle->assetfile,buffer,&handle->file_offset,remaining);
            handle->avail_frames += _bytes2frames_s16(remaining);
            
            if(islooping){
                readsize -= remaining;
                handle->file_offset = 0;
            }
            
            else{
                
                u32 zerosize = required_bytes - _frames2bytes_s16(handle->avail_frames);
                memset((buffer + offset),0,zerosize);
                readsize = 0;
                (*is_done) = true;
            }
            
        }
        
        ADFGetData(handle->assetfile,buffer + offset,&handle->file_offset,readsize);
        handle->avail_frames = _fixed_audio_frames;
        
        auto tdst = (s8*)buffer;
        u32 half_size = buffer_size >> 1;
        auto l = (f32*)tdst;
        auto r = (f32*)(tdst + half_size);
        u32 samples = (required_bytes) >> 1;
        
        //uninterleave
        {
            auto src = (s16*)buffer;
            u32 count = 0;
            
            for(u32 i = 0; i < samples; i += 8){
                
                DeInterleavedSamples samples = {};
                Convert_S16_TO_F32((void*)&samples,src + i,8);
                
                Deinterleave_2((f32*)&samples.l_channel,(f32*)&samples.r_channel,(f32*)&samples);
                
                
                
                _mm_store_ps(l + count,samples.l_channel);
                _mm_store_ps(r + count,samples.r_channel);
                count +=4;
            }
        }
        
        //rescale
        {
            samples >>=1;//half the samples in each buffer
            auto dst = (s8*)handle->ptr;
            auto dst_l = (f32*)(dst + (_frames2bytes_f32(cur_frames) >> 1));
            auto dst_r = (f32*)(dst + (_frames2bytes_f32(cur_frames) >> 1) + _fixed_audio_r_offset);
            
            _kill("overwrite will happen\n",(samples * factor) > (dst_r - dst_l));
            
            Convert_Factor(dst_l,l,samples,inv_factor);
            Convert_Factor(dst_r,r,samples,inv_factor);
        }
        
    }
}

void GetAudioAssetDataPointers(AudioAssetHandle* _restrict handle,f32** _restrict left,f32** _restrict right){
    (*left) = (f32*)handle->ptr;
    (*right) = (f32*)(((s8*)handle->ptr) + _fixed_audio_r_offset);
}

void WriteMaterialFile(const s8* filepath,MaterialAssetHandle* handle){
    
    auto file = FOpenFile(filepath,F_FLAG_WRITEONLY | F_FLAG_CREATE | F_FLAG_TRUNCATE);
    
    u32 encode = _encode('M','A','T',' ');
    
    FWrite(file,&encode,sizeof(encode));
    FWrite(file,&handle->textureid_count,sizeof(handle->textureid_count));
    
    for(u32 i = 0; i < handle->textureid_count; i++){
        
        auto id = handle->textureid_array[i];
        
        auto string = texturehandle_array[id].assetfile;
        
        FWrite(file,string,sizeof(TextureAssetHandle::assetfile));
    }
    
    FCloseFile(file);
}

void ReadMaterialFile(const s8* filepath){
    
    auto mat = AddAssetMaterial();
    
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    
    u32 encode = 0;
    u32 count = 0;
    
    FRead(file,&encode,sizeof(encode));
    FRead(file,&count,sizeof(count));
    
    for(u32 i = 0; i < count; i++){
        
        s8 buffer[sizeof(TextureAssetHandle::assetfile)] = {};
        
        FRead(file,buffer,sizeof(TextureAssetHandle::assetfile));
        
        u8 texture_index = (u8)-1;
        auto hash = PHashString(buffer);
        
        for(u32 j = 0; j < texturehandle_count; j++){
            if(PHashString(texturehandle_array[j].assetfile) == hash){
                texture_index = (u8)j;
            }
        }
        
        _kill("texture not file\n",texture_index == (u8)-1);
        
        MaterialAddTexture(mat,TextureType_Diffuse,texture_index);
    }
    
    FCloseFile(file);
}