#pragma once

//TODO: commits should happen when pushing to render list and recorded as a secondary command buffer

#include "mode.h"
#include "ttype.h"

#include "aassettools.h"

#include "vvulkanx.h"

#include "aaudio.h"

#define TIMESTAMP u32

#define _fetch_list_count 1365


//MARK:the largest possible fetch size is actually the screen size
#define _FetchqueueSize 2048
#define _texturehandle_max 16

//this is the fixed size of an audio asset
#if 0
#define _fixed_audio_size (u32)(_48ms2frames(640)  * 4)

#else

#define _frames2bytes_s16(frames) (frames * sizeof(s16) * 2)
#define _frames2bytes_f32(frames) (frames * sizeof(f32) * 2)
#define _bytes2frames_s16(bytes) (bytes >> 2)

#define _fixed_audio_frames (u32)(_48ms2frames(640))
#define _fixed_audio_size (_fixed_audio_frames  * sizeof(f32) * 2)
#define _fixed_audio_read_size (_fixed_audio_frames * sizeof(s16) * 2)
#define _fixed_audio_r_offset (_fixed_audio_size >> 1)

#endif



#define _debugallocator 0

#ifdef DEBUGallocator

#define _allocprint(string, ...)  _dprint(string, __VA_ARGS__)

#else

#define _allocprint(string, ...)

#endif

struct AssetHandle{
    s8 assetfile[256] = {};//we will be using this as a hash
    u32 id = 0;//index into the table
    void* ptr = 0;
};

struct ModelAssetHandle{
    s8 assetfile[256] = {};//we will be using this as a hash
    u32 id = 0;//index into the table
    void* ptr = 0;
    
    
    u32 vert_fileoffset = 0;
    u32 index_fileoffset = 0;
    u32 gpuid = 0;
    VkDeviceSize gpuptr = 0;
    VBufferContext vertexbuffer = {};
    VBufferContext indexbuffer = {};
    VBufferContext instancebuffer = {};
    
    //#ifdef DEBUG
    u32 vert_component = 0;
    u32 animation_id = (u32)-1;
    //#endif
};


struct AnimatedAssetHandle{
    s8 assetfile[256] = {};//we will be using this as a hash
    u32 id = 0;//index into the table
    void* ptr = 0;
    /*
      id refers to the boneid while ptr refers to the bone ptr
      
      TODO: Save the animbone size
     */
    
    u32 animbonesize = 0;
    
    u32 anim_fileoffset = 0;
    u32 bone_fileoffset = 0;
    
    ALinearBone* rootbone;
    AAnimationSet* animationset_array;
    u16 bone_count;
    u16 animationset_count;
};

struct AudioAssetHandle{
    s8 assetfile[256] = {};//we will be using this as a hash
    u32 id = 0;//index into the table
    void* ptr = 0;
    u32 file_size = 0;
    u32 file_offset = 0;
    u32 avail_frames; // the amount of valid data in the buffer
    u16 compression_type = 0;//change this to u32. might as well
};

//we will always either have 4 or 0 children

//This is better for iterating across child nodes
struct AQuadNode{
    AQuadNode* children_array;
};

//This is better for iterating across levels
struct DQuadNode{
    DQuadNode* first;
    DQuadNode* second;
    DQuadNode* third;
    DQuadNode* fourth;
};

struct Coord{
    u8 x;
    u8 y;
};

struct TCoord : Coord{
    u8 mip;// miplevel = total mips - 1 - mipoffset
};

struct FetchData{
    TCoord src_coord; // src to get from file
    Coord dst_coord; // dst into cache texture
    u32 start_mip; // this is for generating dependency coords only
};

//TODO: clean up the vt system. there's no reason ThreadTextureFetchQueue has to be exposed

struct FetchList{
    FetchData array[_fetch_list_count] = {}; //4k by 4k texture
    u32 count = 0;
};

struct ThreadFetchBatch{
    u16 w;
    u16 h;
    f32 bpp;
    s8* assetfile;
    VTextureContext pagetable;
    FetchList fetchlist;
    u32 total_miplevel;
};

struct TPageQuadNode{
    
    TPageQuadNode* first;
    TPageQuadNode* second;
    TPageQuadNode* third;
    TPageQuadNode* fourth;
    
    // NOTE:-1 means page is not in memory.
    // -2 means this level of the tree is not used. Do we want to use this or a completely different system
    //for textures with no mips?
    
    u32 page_value;
};


struct TextureAssetHandle{
    
    s8 assetfile[256] = {};//we will be using this as a hash
    u16 w;
    u16 h;
    f32 bpp; //bytes per pixel
    u16 max_miplevel;
    u16 timestamp;
    
    TPageQuadNode pagetree;
    VTextureContext pagetable;
};

enum TextureType{
    TextureType_None = 0,
    TextureType_Diffuse,
    TextureType_Specular,
    TextureType_Normal,
};

//just an id of textures. we should make materials match pipelines
//TODO: redo this: we should ,mark up in glsl what the material format is a generate a material from
//there
struct MaterialAssetHandle{
    u8 textureid_array[4];
    u32 textureid_count;
};

MaterialAssetHandle* AddAssetMaterial();

void MaterialAddTexture(MaterialAssetHandle* handle,TextureType type,
                        u8 texture_index);

void InitAssetAllocator(ptrsize size,VkDeviceSize device_size,
                        u32 phys_w_tiles,u32 phys_h_tiles,VDeviceContext* _restrict vdevice,VSwapchainContext* swapchain);

AudioAssetHandle AllocateAssetAudio(const s8* filepath);

void AllocateAssetAnimated(const s8* filepath,
                           const VDeviceContext* _restrict vdevice,
                           VkQueue queue,VkCommandBuffer commandbuffer,
                           u32 vertexbinding_no,AnimatedAssetHandle* animbone,
                           ModelAssetHandle* vertindex);

void AllocateAssetModel(const s8* filepath,
                        const VDeviceContext* _restrict vdevice,VkQueue queue,
                        VkCommandBuffer commandbuffer,u32 vertexbinding_no,ModelAssetHandle* vertindex);

void CommitTexture(TextureAssetHandle* handle);

void CommitModel(ModelAssetHandle* handle,VkCommandBuffer cmdbuffer);

void CommitAnimated(AnimatedAssetHandle* handle);

void CommitAudio(AudioAssetHandle* handle);

void UnallocateAsset(AssetHandle* handle);

void GPUUnallocateAsset(ModelAssetHandle* handle);

void GetAssetData(AssetHandle* handle);

void UpdateAllocatorTimeStamp();

void ResetTransferBuffer();

void ResetAsyncTransferBuffer();

const VTextureContext* GetTextureCache();

void UnallocateAssetTexture(TextureAssetHandle* handle);

TextureAssetHandle* AllocateAssetTexture(const s8* filepath,
                                         const VDeviceContext* _restrict vdevice,VkCommandBuffer cmdbuffer);

struct IVector4{
    u32 x;
    u32 y;
    u32 z;
    u32 w;
};

u32 GenTextureFetchList(TextureAssetHandle* asset,IVector4* src_coords,u32 count,
                        VkCommandBuffer tpage_cmdbuffer);

void FetchTextureTiles(ThreadFetchBatch* batch,VkCommandBuffer fetch_cmdbuffer);

#ifdef DEBUG

//TODO: Test this
void TestDefrag();
#endif


struct ThreadTextureFetchQueue{
    ThreadFetchBatch* buffer = 0;
    TextureAssetHandle* evict_array[_texturehandle_max];
    u32 evict_count = 0;
    u16 count = 0;
    u16 index = 0;
    u16 is_done = true;
    u16 fetch_count = 0;
    volatile  VkCommandBuffer cmdbuffer;
};

VkCommandBuffer _ainline
GetCmdBufferThreadTextureFetchQueue(ThreadTextureFetchQueue* queue){
    return queue->cmdbuffer;
}

void _ainline
SetCmdBufferThreadTextureFetchQueue(ThreadTextureFetchQueue* queue,
                                    VkCommandBuffer cmdbuffer){
    queue->cmdbuffer = cmdbuffer;
}

void _ainline Clear(ThreadTextureFetchQueue* queue){
    queue->count = 0;
    queue->index = 0;
    queue->is_done = false;
    queue->fetch_count = 0;
}

b32 _ainline IsThreadTextureFetchQueueDone(ThreadTextureFetchQueue* queue){
    
    if((queue->index != queue->count) && queue->is_done){
        queue->is_done = false;  
    }
    
    return (queue->index == queue->count && queue->is_done);
}


//NOTE: it is expected that only one thread will enter at a time
void ThreadExecuteVTSystem(ThreadTextureFetchQueue* queue);


void BuildFetchCommandBuffer(VDeviceContext* _restrict vdevice,
                             ThreadTextureFetchQueue* fetchqueue,TSemaphore sem,
                             VkCommandBuffer t_cmdbuffer,MaterialAssetHandle** asset_array,
                             u32 count);

VImageContext GetVTReadbackBuffer();

VkSpecializationInfo VTFragmentShaderSpecConst();


VkCommandBuffer GenerateTextureFetchRequests(
ThreadTextureFetchQueue* fetchqueue,TSemaphore sem);

void VTStart(VkCommandBuffer cmdbuffer);
void VTEnd(VkCommandBuffer cmdbuffer);

void ReadAudioAssetData(AudioAssetHandle* _restrict handle,b32 islooping,u32* _restrict is_done,u32 submit_frames,f32 factor);

void GetAudioAssetDataPointers(AudioAssetHandle* _restrict handle,f32** _restrict left,f32** _restrict right);



//TODO: move this??
void WriteMaterialFile(const s8* filepath,MaterialAssetHandle* handle);
void ReadMaterialFile(const s8* filepath);