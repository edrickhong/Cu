#pragma once


#include "gui_draw.h"
#include "aassetmanager.h"

#include "gamecomp.h"

#define _vertexbindingno 0
#define _instbindingno 1

#define _max_skelon_screen 10
#define _max_components 300

struct PushConst{  
    Matrix4b4 viewproj;
    Vector4 camerapos;
};

struct SkelUBO{
    Matrix4b4 world;
    Matrix4b4 bone_array[64];
    u32 texture_array[16];
    u8 padding[128];
}_align(256);

struct Vertex{
    f32 pos[4];
    f32 normal[4];
    f32 texcoord[2];
};

struct LSkelVertex : Vertex{
    u32 boneindex[4];
    f32 weight[4];
};

struct InstData{
    u32 world;
};

enum PipelineType{
    PSKEL = 0,
    PSTATIC= 1,
};

struct SOAOrientationData{
    
    f32 pos_x[300];
    f32 pos_y[300];
    f32 pos_z[300];
    
    Quaternion rot[300];
    
    f32 scale[300];
    
    u8 skip_array[300] = {};
    u32 count = 0;
};

//This is meant to store state game state specific stuff only
struct GameData{
    
    //these are the things that is the game state
    Vector4 camera_pos;
    Vector4 camera_lookdir;
    f32 roty;
    logic running = true;
    
#if _debug
    logic draw_profiler;
#endif
    
    //These are entity registers
    SOAOrientationData orientation;
    void* components;
};


struct SceneContext{
    f32 prev_frametime;//this should be made into ticks
    KeyboardState* keyboardstate;
    MouseState* mousestate;
    
    EntityDrawData* draw_array;
    u32 draw_count;
    
    EntityAnimationData* animationdata_array;
    u32 animationdata_count;
    
    //state management
    Vector4 (*TranslateWorldSpaceToClipSpace)(Vector4);
    Vector4 (*TranslateClipSpaceToWorldSpace)(Vector4);
    void (*SetObjectMaterial)(u32,u32);
    void (*SetActiveCameraOrientation)(Vector4,Vector4);
    void (*SetObjectOrientation)(u32,Vector4,Quaternion,f32);
    void (*AddPointLight)(Vector3,Color,f32);
    
    //MARK: temp until we assets work
    AudioAssetHandle (*AllocateAssetAudio)(const s8*);
    void (*CommitAnimated)(AnimatedAssetHandle*);
    void (*UnallocateAsset)(AssetHandle*);
    
    
    EntityAudioData** audiocontext;
    u32* audiocontext_count;
    
    //asset data - animation should be bound to animation
    ModelAssetHandle modelasset_array[_max_components];
    u32 modelasset_count;
    
    TextureAssetHandle* textureasset_array[16];
    u32 textureasset_count;
    
    MaterialAssetHandle materialasset_array[16];
    u32 materialasset_count;
    
    AnimatedAssetHandle animatedasset_array[_max_components];
    u32 animatedasset_count;
    
#if _debug
    u32 pipelineverthash_array[32];
    u32 pipelineverthash_count;
#endif
};

struct GameInitData{
    SceneContext* context;
    void* memory;
    WWindowContext* window;
    VDeviceContext vdevice;
    VkRenderPass renderpass;
    VkCommandBuffer cmdbuffer;
    VkQueue queue;
};

struct GameReloadData{
    void* memory;
    VDeviceContext vdevice;
    VkRenderPass renderpass;
    WWindowContext window;
    SceneContext* context;
    GUIContext* guicontext;
    AAllocatorContext* allocatorcontext;
};

extern "C"{
    
    _dllexport void GameComponentWrite(void* context);
    
    _dllexport void GameInit(GameInitData*);
    
    _dllexport void GameUpdateRender(SceneContext*);
    
    _dllexport void GameReload(GameReloadData*);
}


//MARK:light stuff (DEBUG) 
//{
//    auto light_ubo = (LightUBO*)pdata->lightupdate_ptr;
//    light_ubo->point_count = 1;
//    light_ubo->point_array[0] = {Vector4{-8.0f,-5.0f,0.0f,1.0f},White,0.2f};
//    
//    VkMappedMemoryRange range = {
//        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
//        0,
//        pdata->light_ubo.memory,
//        0,
//        sizeof(LightUBO)
//    };
//    
//}