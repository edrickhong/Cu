#include "lighting.glsl"
#include "math.glsl"
#include "structs.glsl"
#include "functions.glsl"
#include "mode.glsl"

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;

#ifdef  USE_SKEL
layout (location = 3) in uvec4 inBoneIndex;
layout (location = 4) in vec4 inWeight;
#endif

layout (set = 0,binding = 0) uniform UBO DYNBUFFER{
  
  mat4 world;
  mat4 bone_array[64];
  uint texture_id[16];
  
}ubo;

layout(push_constant) uniform PushConsts{		
  mat4 viewproj;
  vec4 camerapos;
  vec4 lightpos;
  vec4 ambient_color;
  float ambient_intensity;
}pushconst;

out gl_PerVertex 
{
  vec4 gl_Position;
};

#define _specpower 32.0f
#define _specintensity 0.5f

void main(){

#ifdef  USE_SKEL

  uvec4 boneindex = inBoneIndex;
  
  mat4 bonetransform = ubo.bone_array[boneindex.x] * inWeight.x;

  bonetransform += ubo.bone_array[boneindex.y] * inWeight.y;
  
  bonetransform += ubo.bone_array[boneindex.z] * inWeight.z;
  
  bonetransform += ubo.bone_array[boneindex.w] * inWeight.w;

#endif

  mat4 world = ubo.world;

  mat4 viewproj = pushconst.viewproj;

#ifdef  USE_SKEL
  
  vec4 vert_worldpos = world * bonetransform * vec4(inPos.xyz,1.0f);
  vec3 normal = normalize(mat3(world) * mat3(bonetransform) * vec3(inNormal.xyz));
  
#else
  
  vec4 vert_worldpos = world * vec4(inPos.xyz,1.0f);
  vec3 normal = normalize(mat3(world) * vec3(inNormal.xyz));
  
#endif

  vec4 vert_screenpos = viewproj * vert_worldpos;
  vec3 eye_pos = vec3(pushconst.camerapos);

  gl_Position = TextureSpaceToScreenSpace(inTexcoord);

  gl_Position.z = TextureCull(vert_worldpos,vert_screenpos,eye_pos,normal);
}
