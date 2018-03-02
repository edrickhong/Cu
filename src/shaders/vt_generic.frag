#include "math.glsl"
#include "mode.glsl"

//does not support trilinear and aniso

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;

struct PointLight{
    vec4 pos;
    vec4 color;
    float intensity;
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
    PointLight point_array[1024];
    
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


//TODO:can we make this generic?
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

void main(){
    
    uint texture_id = ubo.texture_id[_Diffuse_ID];
    
    vec4 color = VTReadTexture(samplerColor,vec2(phys_w,phys_h),
                               samplerLookup[texture_id],inTexcoord,texture_id);
    
    vec3 eyepos = vec3(pushconst.camerapos.xyz);
    vec3 eyetovertex = normalize(eyepos - inPos);
    
    vec4 factor = vec4(0.0f,0.0f,0.0f,1.0f);
    
    for(uint i = 0; i < light.point_count; i++){
        
        PointLight p_light = light.point_array[i];
        
        vec3 normal = normalize(inNormal);
        vec3 lightpos = vec3(p_light.pos.xyz);
        vec3 lightdir = normalize(lightpos - inPos);
        
        vec3 diffuse = max(dot(normal,lightdir),0.0f) * vec3(1,1,1);
        vec3 reflvec = reflect(-lightdir,normal);
        
        //factor and strength
        vec3 specular = pow(max(dot(eyepos,reflvec),0.0f),8) * 0.0001f * vec3(1,1,1);
        
        vec3 ambientcolor = vec3(p_light.color * p_light.intensity);
        
        factor += vec4((ambientcolor + diffuse + specular),0);
    }
    
    outFragColor = color * factor;
}
