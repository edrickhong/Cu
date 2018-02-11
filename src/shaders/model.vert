#include "lighting.glsl"
#include "math.glsl"
#include "structs.glsl"
#include "mode.glsl"

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;

#ifdef  USE_SKEL

layout (location = 3) in uvec4 inBoneIndex;
layout (location = 4) in vec4 inWeight;

#endif

struct Light{
  vec3 pos;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct Material{
  vec3 ambient;
  vec3 diffuse; // basically the texture
  vec3 specular;// basically the texture
  float shine;
};


layout (set = 0,binding = 0) uniform UBO DYNBUFFER{
  
  mat4 world;
  mat4 bone_array[64];
  uint texture_id[16];
  
}ubo;

#define _Diffuse_ID 0

layout(push_constant) uniform PushConsts{		
  mat4 viewproj;//TODO: we can separate proj to a specialization const since it never changes
  vec4 camerapos;
  vec4 lightpos;
  vec4 ambient_color;
  float ambient_intensity;
}pushconst;


layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) flat out vec3 outEyePos;
layout (location = 3) flat out vec3 outLightPos;
layout (location = 4) out vec3 outAmbientColor;
layout (location = 5) out vec2 outTexcoord;
layout (location = 6) flat out uint outTextureIndex;

out gl_PerVertex {
  
  vec4 gl_Position;
  
};

#define _specpower 32.0f
#define _specintensity 0.5f

void main(){

#ifdef USE_SKEL
  
  mat4 bonetransform;
  
  uvec4 boneindex = inBoneIndex;
  
  bonetransform = ubo.bone_array[boneindex.x] * inWeight.x;

  bonetransform += ubo.bone_array[boneindex.y] * inWeight.y;
  
  bonetransform += ubo.bone_array[boneindex.z] * inWeight.z;
  
  bonetransform += ubo.bone_array[boneindex.w] * inWeight.w;

#endif

  mat4 world = ubo.world;

  mat4 viewproj = pushconst.viewproj;

  //NOTE: normal will be wrong for non-uniform scaling
  
#ifdef USE_SKEL

  vec4 vertexposition = world * bonetransform * vec4(inPos.xyz,1.0f);
  vec3 normal  = normalize(mat3(world) * mat3(bonetransform) * vec3(inNormal.xyz));

#else

  vec4 vertexposition = world * vec4(inPos.xyz,1.0f);
  vec3 normal  = normalize(mat3(world) * vec3(inNormal.xyz));
  
#endif

  gl_Position = viewproj * vertexposition;
  outPos = vec3(vertexposition);
  outNormal = normal;
  outEyePos = vec3(pushconst.camerapos);
  outLightPos = vec3(pushconst.lightpos);
  outAmbientColor = vec3(pushconst.ambient_color * pushconst.ambient_intensity);
  outTexcoord = inTexcoord;
  outTextureIndex = ubo.texture_id[_Diffuse_ID];
  
}


/*
Types of texture materials

aiTextureType_NONE 	
Dummy value.

No texture, but the value to be used as 'texture semantic' (#aiMaterialProperty::mSemantic) for all material properties not related to textures.

aiTextureType_DIFFUSE 	
The texture is combined with the result of the diffuse lighting equation.

aiTextureType_SPECULAR 	
The texture is combined with the result of the specular lighting equation.

aiTextureType_AMBIENT 	
The texture is combined with the result of the ambient lighting equation.

aiTextureType_EMISSIVE 	
The texture is added to the result of the lighting calculation.

It isn't influenced by incoming light.

aiTextureType_HEIGHT 	
The texture is a height map.

By convention, higher gray-scale values stand for higher elevations from the base height.

aiTextureType_NORMALS 	
The texture is a (tangent space) normal-map.

Again, there are several conventions for tangent-space normal maps. Assimp does (intentionally) not distinguish here.

aiTextureType_SHININESS 	
The texture defines the glossiness of the material.

The glossiness is in fact the exponent of the specular (phong) lighting equation. Usually there is a conversion function defined to map the linear color values in the texture to a suitable exponent. Have fun.

aiTextureType_OPACITY 	
The texture defines per-pixel opacity.

Usually 'white' means opaque and 'black' means 'transparency'. Or quite the opposite. Have fun.

aiTextureType_DISPLACEMENT 	
Displacement texture.

The exact purpose and format is application-dependent.
Higher color values stand for higher vertex displacements.

aiTextureType_LIGHTMAP 	
Lightmap texture (aka Ambient Occlusion)

Both 'Lightmaps' and dedicated 'ambient occlusion maps' are covered by this material property. The texture contains a scaling value for the final color value of a pixel. Its intensity is not affected by incoming light.

aiTextureType_REFLECTION 	
Reflection texture.

Contains the color of a perfect mirror reflection. Rarely used, almost never for real-time applications.

aiTextureType_UNKNOWN 	
Unknown texture.

A texture reference that does not match any of the definitions above is considered to be 'unknown'. It is still imported, but is excluded from any further postprocessing.
*/
