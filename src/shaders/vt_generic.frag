#include "math.glsl"
#include "mode.glsl"

//does not support trilinear and aniso

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;

struct PointLight{
    vec4 pos;
    vec4 color;
    
    float radius;
};

struct DirectionalLight{
    vec4 dir;
    vec4 color;
};

layout (set = 0,binding = 0) uniform UBO DYNBUFFER{
    mat4 world;
    mat4 bone_array[64];
    uint texture_id[16];
    
}ubo;



layout (set = 1,binding = 0) uniform sampler2D samplerColor;//we should separate this
layout (set = 1,binding = 1) uniform sampler2D samplerLookup[16];//we should separate this

layout (set = 1,binding = 2, rgba8) uniform restrict writeonly image2D vt_feedback;


layout (set = 1,binding = 3) uniform LIGHT_UBO{
    uint point_count;
    uint dir_count;
    PointLight point_array[1024];
    DirectionalLight dir_array[1024];
    
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

layout (constant_id = 0) const float framebuffer_w = 1440.0f;
layout (constant_id = 1) const float framebuffer_h = 1080.0f;
layout (constant_id = 2) const float feedback_w = 160.0f;
layout (constant_id = 3) const float feedback_h = 120.0f;

layout (constant_id = 4) const float page_size = 128.0f;
layout (constant_id = 5) const float phys_w = 16384.0f;
layout (constant_id = 6) const float phys_h = 8192.0f;

#define _max_int 255.0f





float GetMipLevel(vec2 uv,vec2 dim){
    
    /*
      explanation for dxdf(k)/dydf(k) in glsl:
      
      as the frag shader renders a triangle, it sweeps the aabb of the triangle in a 2x2 pixel grid fashion
      (executes 2x2 pixels per invocation). when an invocation reaches the dfdx function, it calculates
      the difference between the value of k that is passed to that instance of dxdf:
      
      2x2 pixel grid each w unique values of k:
      
      1  2
      0  4
      
      the dfdx for pixel (0,0) is 2 - 1 which is 1
      the dfdy for pixel (0,0) is 0 - 1 which is -1
      
      NOTE: this is taken from stb's implementation
      //TODO: try to fully understand this
    */
    
    vec2 uv_scaled = uv * dim;
    
    vec2 dx_scaled = dFdx(uv_scaled);
    vec2 dy_scaled = dFdy(uv_scaled);
    
    vec2 dtex = dx_scaled * dx_scaled + dy_scaled * dy_scaled;
    float min_delta = max(dtex.x,dtex.y);
    float miplevel = max(0.5 * log2(min_delta), 0.0);
    
    return miplevel;
}

vec4 VTGetPhysCoord(sampler2D vt_texture,vec2 vt_coord){
    return vec4(0);
}

bool VTToGenerateFetchRequest(vec4 vt){
    return vt.z == 0.0f;
}

bool VTToExecuteWrite(vec2 write_pos,vec2 framebuffer_dim_rcp,vec2 feedback_dim){
    
    vec2 subpixel_size = feedback_dim * framebuffer_dim_rcp;
    
    return fract(write_pos.x) < subpixel_size.x && fract(write_pos.y) < subpixel_size.y;
}

vec2 VTGenerateWritePos(vec2 framebuffer_dim_rcp,vec2 feedback_dim,vec2 fragcoord){
    
    
    vec2 normalized_pixel_pos = fragcoord * framebuffer_dim_rcp;
    vec2 write_pos = normalized_pixel_pos * feedback_dim;
    
    return write_pos;
}

vec4 VTGenerateFetchData(uint texture_id,vec2 fetchcoord,uint mip_level,vec2 vt_dimpagesf){
    
    vec2 tcoord = fetchcoord * vt_dimpagesf;
    
    
    vec4 fetch_data =
        vec4(float(texture_id + 1),float(mip_level),tcoord.x,tcoord.y) *
        vec4(1.0f/255.0f,1.0f/255.0f,1.0f/255.0f,1.0f/255.0f);
    
    return fetch_data;
}

//TODO: Separate this
vec4 VTReadTexture(sampler2D phys_texture,vec2 phys_dim,
                   sampler2D vt_texture,vec2 vt_coord,uint texture_id){
    
    vec2 phys_dimpages_recp = page_size / vec2(phys_w,phys_h);
    
    int total_mip_levels = textureQueryLevels(vt_texture);
    int mip_level = clamp(int(GetMipLevel(vt_coord,textureSize(vt_texture,0) * page_size)),0,
                          total_mip_levels - 1);
    
    vec2 vt_dimpagesf;
    vec4 p_data;
    
    {
        
        ivec2 vt_dimpages = textureSize(vt_texture,mip_level);
        
        vt_dimpagesf = vec2(vt_dimpages.x,vt_dimpages.y);
        
        
        vec2 fetchcoord = vt_coord - (fract(vt_coord * vt_dimpagesf)/vt_dimpagesf);
        
        //x:x, y:y ,z: 1.0f if page is invalid , w:
        p_data = textureLod(vt_texture,fetchcoord,mip_level);
        
        if(p_data.z == 0.0f){
            
            //write to fetch buffer
            
            vec2 fbdim_rcp = vec2(1.0f/framebuffer_w,1.0f/framebuffer_h);
            vec2 image_feedback_dim = vec2(feedback_w,feedback_h);
            vec2 subpixel_size = image_feedback_dim * fbdim_rcp;
            vec2 normalized_pixel_pos = gl_FragCoord.xy * fbdim_rcp;
            vec2 write_pos = normalized_pixel_pos * image_feedback_dim;
            
            if(fract(write_pos.x) < subpixel_size.x && fract(write_pos.y) < subpixel_size.y){
                
                vec2 tcoord = fetchcoord * vt_dimpagesf;
                
                
                vec4 fetch_data =
                    vec4(float(texture_id + 1),float(mip_level),tcoord.x,tcoord.y) *
                    vec4(1.0f/255.0f,1.0f/255.0f,1.0f/255.0f,1.0f/255.0f);
                
                ivec2 write_pos_int = ivec2(write_pos.x,write_pos.y);
                
                imageStore(vt_feedback,write_pos_int,fetch_data);
                
#if  0
                
                vec4 m_color = vec4(0,0,0,0);
                
                if(mip_level == 0){
                    m_color = vec4(1,0,0,1);
                }
                
                if(mip_level == 1){
                    m_color = vec4(0,1,0,1);
                }
                
                if(mip_level == 2){
                    m_color = vec4(0,0,1,1);
                }
                
                if(mip_level == 3){
                    m_color = vec4(1,0,1,1);
                }
                
                return m_color;
                
#endif
                
            }
            
#if  1
            
            //try to get a valid texture coord
            
            for(mip_level = mip_level + 1; mip_level < total_mip_levels; mip_level++){
                
                vt_dimpages = textureSize(vt_texture,mip_level);
                
                vt_dimpagesf = vec2(vt_dimpages.x,vt_dimpages.y);
                
                fetchcoord = vt_coord - (fract(vt_coord * vt_dimpagesf)/vt_dimpagesf);
                
                p_data = texture(vt_texture,fetchcoord);
                
                //FIXME: the regions we break at are wrong
                if(p_data.z != 0.0f){
                    break;
                }
                
            }
            
#endif
            
        }
        
    }
    
    //actual texture data
    
    vec2 page_pos = floor(p_data.xy * 255.0f + 0.5f);
    
    vec2 page_offset = fract(vt_coord.xy * vt_dimpagesf);
    
#define k 1.0f/128.0f
    
#if 1
    
    //This produces better results but might be slower
    page_offset.x = clamp(page_offset.x,k,(1.0f - k));
    page_offset.y = clamp(page_offset.y,k,(1.0f - k));
    
#else
    
    page_offset.x = InterpolateValue(k,(1.0f - k),page_offset.x);
    page_offset.y = InterpolateValue(k,(1.0f - k),page_offset.y);
    
#endif
    
    page_pos = (page_pos + page_offset) * phys_dimpages_recp;
    
    vec4 color = texture(phys_texture,page_pos.xy);
    
#if  0
    
    if(mip_level == 0){
        color = vec4(1,0,0,1);
    }
    
    if(mip_level == 1){
        color = vec4(0,1,0,1);
    }
    
    if(mip_level == 2){
        color = vec4(0,0,1,1);
    }
    
    if(mip_level == 3){
        color = vec4(1,0,1,1);
    }
    
#endif
    
    return color;
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
    
    vec4 color = VTReadTexture(samplerColor,vec2(phys_w,phys_h),
                               samplerLookup[texture_id],inTexcoord,texture_id);
    
    vec3 eyepos = vec3(pushconst.camerapos.xyz);
    vec3 eyetovertex = normalize(eyepos - inPos);
    
    vec3 normal = normalize(inNormal);
    
    //MARK: these are fixed for now
    vec3 ambient_color = vec3(1.0f,1.0f,1.0f);
    float ambient_intensity = 0.1f;
    
    vec3 ambient = ambient_color * ambient_intensity;
    
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
    
    factor.xyz += ambient;
    
    outFragColor = color * factor;
}
