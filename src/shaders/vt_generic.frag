#include "math.glsl"
#include "mode.glsl"
#include "virtual_texturing.glsl"

//does not support trilinear and aniso

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;

layout (set = 0,binding = 0) uniform UBO DYNBUFFER{
    mat4 world;
    mat4 bone_array[64];
    uint texture_id[16];
    
}ubo;



layout (set = 1,binding = 0) uniform sampler2D samplerColor;//we should separate this
layout (set = 1,binding = 1) uniform sampler2D samplerLookup[16];//we should separate this

layout (set = 1,binding = 2, rgba8) uniform restrict writeonly image2D vt_feedback;

struct PointLight{
    vec4 pos;
    vec4 color;
    
    float radius;
};

struct DirectionalLight{
    vec4 dir;
    vec4 color;
};

struct SpotLight{
    
    vec4 pos;
    vec4 dir;
    vec4 color;
    
    float cos_angle;
    float hard_cos_angle;
    float radius;
};


layout (set = 1,binding = 3) uniform LIGHT_UBO{
    
    uint dir_count;
    uint point_count;
    uint spot_count;
    
    DirectionalLight dir_array[1024];
    PointLight point_array[1024];
    SpotLight spot_array[1024];
    
    vec4 ambient_color;
    
}light;

layout(origin_upper_left) in vec4 gl_FragCoord;

struct FetchCoord{
    uvec2 texture_detail;// x: texture_id(actual id = tid + 1) y: mip
    vec2 texcoord; // x and y coord of the texture
};

layout(push_constant) uniform PushConsts{		
    mat4 viewproj;
    vec4 camerapos;
}pushconst;



layout (location = 0) out vec4 outFragColor;

//layout (constant_id = 0) const float framebuffer_w = 1440.0f;
//layout (constant_id = 1) const float framebuffer_h = 1080.0f;
//layout (constant_id = 2) const float feedback_w = 160.0f;
//layout (constant_id = 3) const float feedback_h = 120.0f;

layout (constant_id = 0) const float framebuffer_w = 1280.0f;
layout (constant_id = 1) const float framebuffer_h = 720.0f;
layout (constant_id = 2) const float feedback_w = 1280.0f;
layout (constant_id = 3) const float feedback_h = 720.0f;

layout (constant_id = 4) const float page_size = 128.0f;
layout (constant_id = 5) const float phys_w = 16384.0f;
layout (constant_id = 6) const float phys_h = 8192.0f;







vec4 VTInternalReadTexture(sampler2D phys_texture,vec2 phys_dim,
                           sampler2D vt_texture,vec2 vt_coord,uint texture_id){
    
    vec2 phys_dimpages_recp = page_size / phys_dim;
    
    int total_mip_levels = textureQueryLevels(vt_texture);
    int mip_level = clamp(int(GetMipLevel(vt_coord,textureSize(vt_texture,0) * page_size)),0,
                          total_mip_levels - 1);
    
    VT_TextureLevelContext context = VT_GetContext(vt_texture,vt_coord,mip_level);
    
    if(!VT_IsContextValid(context)){
        
        vec2 fbdim_rcp = vec2(1.0f/framebuffer_w,1.0f/framebuffer_h);
        vec2 feedback_dim = vec2(feedback_w,feedback_h);
        
        vec2 write_pos = VT_GenerateWritePos(fbdim_rcp,feedback_dim,gl_FragCoord.xy);
        
        if(VT_ToExecuteWritePos(write_pos,fbdim_rcp,feedback_dim)){
            
            imageStore(vt_feedback,ivec2(write_pos.xy),VT_GenerateFetchRequestData(context,texture_id,mip_level));
        }
        
        context = VT_HandleInvalidContext(context,vt_texture,vt_coord,mip_level,total_mip_levels);
    }
    
    return VT_ReadPhysTexture(context,vt_coord,phys_dimpages_recp,phys_texture);
}

#define _Diffuse_ID 0

//TODO: figure out the effective range of the lights
float CalculateAttenuation(float dist,float constant,float linear,float quadratic){
    
    return (1.0f)/(constant + (linear * dist) + (quadratic * (dist * dist)));
}

float CalculateSphericalLightAttenuation(float dist,float radius){
    
    // f = 1/(((d/r) + 1)^2)
    //or f = 1/(1 + ((2/r) * d) + ((1/(r^2)) * d^2))
    // which is similar to the formula where constant = 1,linear = (2/r) and quadratic = (1/r^2)
    
    float linear = 2.0f/radius;
    float quadratic = 1/(radius * radius);
    
    
    return CalculateAttenuation(dist,1.0f,linear,quadratic);
}

float Diffuse(vec3 normal,vec3 lightdir){
    return max(dot(normal,lightdir),0.0f);
}

float Specular(vec3 normal,vec3 lightdir,vec3 eyepos,float factor,float intensity){
    
    vec3 reflvec = reflect(-lightdir,normal);
    
    
    return pow(max(dot(eyepos,reflvec),0.0f),factor) * intensity;
}



void main(){
    
    uint texture_id = ubo.texture_id[_Diffuse_ID];
    
    vec4 color = VTInternalReadTexture(samplerColor,vec2(phys_w,phys_h),
                                       samplerLookup[texture_id],inTexcoord,texture_id);
    
    vec3 eyepos = vec3(pushconst.camerapos.xyz);
    vec3 eyetovertex = normalize(eyepos - inPos);
    
    vec3 normal = normalize(inNormal);
    
    vec3 ambient = vec3(light.ambient_color.xyz);
    
    vec4 factor = vec4(0.0f,0.0f,0.0f,1.0f);
    
    for(uint i = 0; i < light.dir_count; i++){
        
        DirectionalLight d_light = light.dir_array[i];
        vec3 lightdir = -normalize(d_light.dir.xyz);
        
        vec3 diffuse = Diffuse(normal,lightdir) * d_light.color.xyz;
        
        vec3 specular = Specular(normal,lightdir,eyepos,8,0.0001f) * d_light.color.xyz;
        
        factor += vec4((diffuse + specular),0);
    }
    
    for(uint i = 0; i < light.point_count; i++){
        
        PointLight p_light = light.point_array[i];
        
        vec3 lightpos = vec3(p_light.pos.xyz);
        vec3 lightdir = normalize(lightpos - inPos);
        
        vec3 diffuse = Diffuse(normal,lightdir) * p_light.color.xyz;
        
        vec3 specular = Specular(normal,lightdir,eyepos,8,0.0001f) * p_light.color.xyz;
        
        float attenuation = CalculateSphericalLightAttenuation(length(inPos - lightpos),p_light.radius);
        
        factor += vec4((diffuse + specular),0) * attenuation;
    }
    
    if(light.point_count == 0){
        outFragColor = vec4(1);
        
        return;
    }
    
    for(uint i = 0; i < light.spot_count; i++){
        
        SpotLight s_light = light.spot_array[i];
        
        vec3 lightpos = vec3(s_light.pos.xyz);
        vec3 frag_to_light = normalize(lightpos - inPos);
        
        vec3 lightdir = normalize(-s_light.dir.xyz);
        
        float t = dot(lightdir,frag_to_light);
        
        if(t > s_light.cos_angle){
            
            vec3 diffuse = Diffuse(normal,lightdir) * s_light.color.xyz;
            vec3 specular = Specular(normal,lightdir,eyepos,8,0.0001f) * s_light.color.xyz;
            
            float attenuation = CalculateSphericalLightAttenuation(length(inPos - lightpos),s_light.radius);
            
            float intensity = clamp((t - s_light.hard_cos_angle)/(s_light.hard_cos_angle - s_light.cos_angle),0.0f,1.0f);
            
            factor += vec4((diffuse + specular),0) * attenuation * intensity;
        }
    }
    
    factor.xyz += ambient;
    
    outFragColor = color * factor;
}
