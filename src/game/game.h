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
    f32 world[16];
    f32 bone_array[64 * 16];
    u32 texture_array[16];
    
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

#define _max_objects 512

struct SOAOrientationData{
    
    f32 pos_x[_max_objects];
    f32 pos_y[_max_objects];
    f32 pos_z[_max_objects];
    
    Quaternion rot[_max_objects];
    
    f32 scale[_max_objects];
    
    u8 skip_array[_max_objects] = {};
    
    s8 obj_name[_max_objects][256] = {};
    u32 count = 0;
};

//This is meant to store state game state specific stuff only
struct GameData{
    
    //these are the things that is the game state
    Vector4 camera_pos;
    Vector4 camera_lookdir;
    f32 roty;
    logic running = true;
    
    //These are entity registers
    SOAOrientationData orientation;
    void* components;
    
#if _debug
    //GUI state variables
    
    
    Vector2 prev_mpos;
    u32 widget_type = 0;
    u32 obj_id = 0;
    u32 dirlight_id = 0;
    
    logic draw_profiler;
    logic show_object_list = false;
    logic show_object_editor = false;
    logic show_dir_light_editor = false;
    logic show_ambient_light_editor = false;
    logic write_orientation = true;
    
    GUIVec2 pos_control = {-1.0f,1.0f};
    GUIDim2 dim_control = {GUIDEFAULT_W * 2.8f,GUIDEFAULT_H * 0.22f};
    
    GUIVec2 pos_obj_list = {-0.16f,GUIDEFAULT_Y};
    GUIDim2 dim_obj_list = {GUIDEFAULT_W * 2.2f,GUIDEFAULT_H};
    
    GUIVec2 pos_obj_editor = {0.4f,GUIDEFAULT_Y};
    GUIDim2 dim_obj_editor = {GUIDEFAULT_W * 2.2f,GUIDEFAULT_H * 2.5f};
    
    GUIVec2 pos_dirlight = {-1.0f,1.0f};
    GUIVec2 pos_ambient = {-1.0f,1.0f};
    
    s8 o_buffer[4][128] = {};
    
    Quaternion dir_light_rot[1024];
    Color dir_light_color[1024];
    f32 dir_light_intensity[1024];
    
    Color ambient_color;
    f32 ambient_intensity;
    
#endif
    
};

struct DirLight{
    Vector4 dir;
    Color color;
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
    void (*AddSpotLight)(Vector3,Vector3,Color,f32,f32,f32);
    
    void (*GetDirLightList)(DirLight**,u32**);
    void (*SetAmbientColor)(Color,f32);
    
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