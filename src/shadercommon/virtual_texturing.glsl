

struct VT_TextureLevelContext{
    vec2 vt_dimpages;
    vec2 texture_local_coord;
    vec4 p_data;//x:x, y:y ,z: 1.0f if page is invalid , w:
};

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

bool VT_IsContextValid(VT_TextureLevelContext context){
    return context.p_data.z != 0.0f;
}

VT_TextureLevelContext VT_GetContext(sampler2D vt_texture,vec2 vt_coord,int mip_level){
    
    VT_TextureLevelContext context;
    
    context.vt_dimpages = vec2(textureSize(vt_texture,mip_level).xy);
    
    
    context.texture_local_coord = vt_coord - (fract(vt_coord * context.vt_dimpages)/context.vt_dimpages);
    
    context.p_data = textureLod(vt_texture,context.texture_local_coord,mip_level);
    
    return context;
}

bool VT_ToExecuteWritePos(vec2 write_pos,vec2 framebuffer_dim_rcp,vec2 feedback_dim){
    
    vec2 subpixel_size = feedback_dim * framebuffer_dim_rcp;
    
    return fract(write_pos.x) < subpixel_size.x && fract(write_pos.y) < subpixel_size.y;
}

vec2 VT_GenerateWritePos(vec2 framebuffer_dim_rcp,vec2 feedback_dim,vec2 fragcoord){
    
    
    vec2 normalized_pixel_pos = fragcoord * framebuffer_dim_rcp;
    vec2 write_pos = normalized_pixel_pos * feedback_dim;
    
    return write_pos;
}

vec4 VT_GenerateFetchRequestData(VT_TextureLevelContext context,uint texture_id,int mip_level){
    
    vec2 tcoord = context.texture_local_coord * context.vt_dimpages;
    
    
    vec4 fetch_data =
        vec4(float(texture_id + 1),float(mip_level),tcoord.x,tcoord.y) *
        vec4(1.0f/255.0f,1.0f/255.0f,1.0f/255.0f,1.0f/255.0f);
    
    return fetch_data;
}

VT_TextureLevelContext VT_HandleInvalidContext(VT_TextureLevelContext context,sampler2D vt_texture,vec2 vt_coord,int mip_level,int total_mip_levels){
    
    for(mip_level = mip_level + 1; mip_level < total_mip_levels; mip_level++){
        
        context = VT_GetContext(vt_texture,vt_coord,mip_level);
        
        if(VT_IsContextValid(context)){
            break;
        }
    }
    
    return context;
}

vec4 VT_ReadPhysTexture(VT_TextureLevelContext context,vec2 vt_coord,vec2 phys_dimpages_recp,sampler2D phys_texture){
    
    vec2 page_pos = floor(context.p_data.xy * 255.0f + 0.5f);
    
    vec2 page_offset = fract(vt_coord.xy * context.vt_dimpages);
    
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
    
    return texture(phys_texture,page_pos.xy);
}