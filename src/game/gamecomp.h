#pragma once

typedef u32 ObjectID;
typedef u32 LightID;
typedef u32 AnimationID;
typedef u32 ModelID;
typedef u32 TextureID;
typedef u32 RenderGroupIndex;
typedef u32 MaterialID;

//NOTE: we should not be able to edit core formats. this is because the engine depends not changing
struct REFLCOMPONENT EntityAnimationData{
    ObjectID id;
    AnimationID animdata_id;
    u16 animationindex;
    u16 islooping;
    f32 animationtime;
    f32 speed;
};

struct REFLCOMPONENT EntityDrawData{
    ObjectID id;
    ModelID model;
    MaterialID material;
    RenderGroupIndex group;
};

struct REFLCOMPONENT EntityAudioData{
    ObjectID id;
    AudioAssetHandle audioasset;
    u16 islooping = 0;
    u16 toremove = 0;
};

struct REFLCOMPONENT PointLight{
    ObjectID id;
    f32 R;
    f32 G;
    f32 B;
    f32 intensity;
    
    f32 linear;
    f32 quadratic;
};