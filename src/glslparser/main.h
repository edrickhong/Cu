#pragma once
#include "aallocator.h"

#include "stdio.h"

logic IsPreprocessor(s8 c){
    return c == '#';
}

logic IsComment(s8 c1,s8 c2){
    return c1 == '/'  && c2 == '/';
}

logic IsStartComment(s8 c1,s8 c2){
    return c1 == '/'  && c2 == '*';
}

logic IsEndComment(s8 c1,s8 c2){
    return c1 == '*'  && c2 == '/';
}

enum ParseState{
    ParseState_NORMAL,
    ParseState_STRING,
    ParseState_COMMENT,
    ParseState_LAYOUT,
    ParseState_SCOPEBLOCK,
    ParseState_SCOPEARRAY,
};

struct ParseStateStack{
    ParseState parse_array[16] = {ParseState_NORMAL};
    u32 parse_count = 1;
};

ParseState GetState(ParseStateStack* pstate){
    return pstate->parse_array[pstate->parse_count - 1];
}

void PushState(ParseStateStack* pstate,ParseState state){
    pstate->parse_array[pstate->parse_count] = state;
    pstate->parse_count++;
}

ParseState PopState(ParseStateStack* pstate){
    auto state = GetState(pstate);
    pstate->parse_count--;
    return state;
}

void SkipBlockComments(s8* string,u32* pos,ParseStateStack* pstate){
    
    auto cur = *pos;
    
    if(IsStartComment(string[cur],string[cur + 1])){
        PushState(pstate,ParseState_COMMENT);
    }
    
    if(GetState(pstate) == ParseState_COMMENT){
        
        for(;;){
            
            s8 c = string[cur];
            
            if(c == 0){
                break;
            }
            
            if(IsEndComment(c,string[cur + 1])){
                cur += 2;
                PopState(pstate);
                break;
            }
            
            cur++;
        }
        
        *pos = cur;
        
    }
}

_persist u32 macro_array[16];
_persist u32 macro_count = 0;

enum ShaderType{
    ShaderType_VERTEX = PHashString("vert"),
    ShaderType_TESSELLATION_CONTROL = PHashString("tesc"),
    ShaderType_TESSELLATION_EVALUATION = PHashString("tese"),
    ShaderType_GEOMETRY = PHashString("geom"),
    ShaderType_FRAGMENT = PHashString("frag"),
    ShaderType_COMPUTE = PHashString("comp"),
};

enum LayoutType : u32{
    LayoutType_IN = PHashString("in"),
    LayoutType_DESC = PHashString("set"),
    LayoutType_BINDING = PHashString("binding"),
    LayoutType_PUSHCONST = PHashString("push_constant"),
    LayoutType_SPV = (u32)-1,
};

enum GLSLTypes{
    
    GLSLTypes_BOOL = PHashString("bool"),
    GLSLTypes_INT = PHashString("int"),
    GLSLTypes_UINT = PHashString("uint"),
    GLSLTypes_FLOAT = PHashString("float"),
    GLSLTypes_DOUBLE = PHashString("double"),
    GLSLTypes_BVEC1 = PHashString("bvec1"),
    GLSLTypes_BVEC2 = PHashString("bvec2"),
    GLSLTypes_BVEC3 = PHashString("bvec3"),
    GLSLTypes_BVEC4 = PHashString("bvec4"),
    
    GLSLTypes_IVEC1 = PHashString("ivec1"),
    GLSLTypes_IVEC2 = PHashString("ivec2"),
    GLSLTypes_IVEC3 = PHashString("ivec3"),
    GLSLTypes_IVEC4 = PHashString("ivec4"),
    
    GLSLTypes_UVEC1 = PHashString("uvec1"),
    GLSLTypes_UVEC2 = PHashString("uvec2"),
    GLSLTypes_UVEC3 = PHashString("uvec3"),
    GLSLTypes_UVEC4 = PHashString("uvec4"),
    
    GLSLTypes_VEC1 = PHashString("vec1"),
    GLSLTypes_VEC2 = PHashString("vec2"),
    GLSLTypes_VEC3 = PHashString("vec3"),
    GLSLTypes_VEC4 = PHashString("vec4"),
    
    GLSLTypes_DVEC1 = PHashString("dvec1"),
    GLSLTypes_DVEC2 = PHashString("dvec2"),
    GLSLTypes_DVEC3 = PHashString("dvec3"),
    GLSLTypes_DVEC4 = PHashString("dvec4"),
    
    GLSLTypes_MAT2 = PHashString("mat2"),
    GLSLTypes_MAT3 = PHashString("mat3"),
    GLSLTypes_MAT4 = PHashString("mat4"),
    
    //These are opaque types that are only useful in binding
    GLSLTypes_BUFFER = PHashString("buffer"),
    
    GLSLTypes_SAMPLER = PHashString("sampler"),
    
    GLSLTypes_SAMPLER1D = PHashString("sampler1D"),
    GLSLTypes_SAMPLER2D = PHashString("sampler2D"),
    GLSLTypes_SAMPLER3D = PHashString("sampler3D"),
    GLSLTypes_SAMPLERCUBE = PHashString("samplerCube"),
    GLSLTypes_SAMPLER2DRECT = PHashString("sampler2DRect"),
    GLSLTypes_SAMPLERBUFFER = PHashString("samplerBuffer"),
    GLSLTypes_SAMPLER2DMS = PHashString("sampler2DMS"),
    
    GLSLTypes_SAMPLER1DSHADOW = PHashString("sampler1DShadow"),
    GLSLTypes_SAMPLER2DSHADOW = PHashString("sampler2DShadow"),
    GLSLTypes_SAMPLER3DSHADOW = PHashString("sampler3DShadow"),
    GLSLTypes_SAMPLERCUBESHADOW = PHashString("samplerCubeShadow"),
    GLSLTypes_SAMPLER2DRECTSHADOW = PHashString("sampler2DRectShadow"),
    
    //TODO: every sampler type has a corresponding texture type
    GLSLTypes_TEXTURE1D = PHashString("texture1D"),
    GLSLTypes_TEXTURE2D = PHashString("texture2D"),
    GLSLTypes_TEXTURE3D = PHashString("texture3D"),
    GLSLTypes_TEXTURECUBE = PHashString("textureCube"),
    GLSLTypes_TEXTURE2DRECT = PHashString("texture2DRect"),
    GLSLTypes_TEXTUREBUFFER = PHashString("textureBuffer"),
    GLSLTypes_TEXTURE2DMS = PHashString("texture2DMS"),
    
    GLSLTypes_IMAGE1D = PHashString("image1D"),
    GLSLTypes_IMAGE2D = PHashString("image2D"),
    GLSLTypes_IMAGE3D = PHashString("image3D"),
    GLSLTypes_IMAGECUBE = PHashString("imageCube"),
    GLSLTypes_IMAGE2DRECT = PHashString("image2DRect"),
    GLSLTypes_IMAGEBUFFER = PHashString("imageBuffer"),
    GLSLTypes_IMAGE2DMS = PHashString("image2DMS"),
    
    
    GLSLTypes_UNIFORM = PHashString("uniform"),
    GLSLTypes_RGBA8 = PHashString("rgba8"),
    
    
    /* We need to handle opaque types
    
       gsampler1DArray	GL_TEXTURE_1D_ARRAY	1D Array Texture
       gsampler2DArray	GL_TEXTURE_2D_ARRAY	2D Array Texture
       gsamplerCubeArray	GL_TEXTURE_CUBE_MAP_ARRAY	Cubemap Array Texture
       gsampler2DMSArray	GL_TEXTURE_2D_MULTISAMPLE_ARRAY	Multisample Array Texture
       sampler1DArrayShadow	GL_TEXTURE_1D_ARRAY
       sampler2DArrayShadow	GL_TEXTURE_2D_ARRAY
       samplerCubeArrayShadow	GL_TEXTURE_CUBE_MAP_ARRAY
       (requires GL 4.0 or ARB_texture_cube_map_array)
       
       gimage1D	GL_TEXTURE_1D
       single layer from
       
       GL_TEXTURE_1D_ARRAY
       gimage2D	GL_TEXTURE_2D
       single layer from:
       
       GL_TEXTURE_2D_ARRAY
       GL_TEXTURE_CUBE_MAP
       GL_TEXTURE_CUBE_MAP_ARRAY
       GL_TEXTURE_3D
       gimage3D	GL_TEXTURE_3D
       gimageCube	GL_TEXTURE_CUBE_MAP
       gimage2DRect	GL_TEXTURE_RECTANGLE
       gimage1DArray	GL_TEXTURE_1D_ARRAY
       gimage2DArray	GL_TEXTURE_2D_ARRAY
       gimageCubeArray	GL_TEXTURE_CUBE_MAP_ARRAY (requires GL 4.0 or ARB_texture_cube_map_array)
       gimageBuffer	GL_TEXTURE_BUFFER
       gimage2DMS	GL_TEXTURE_2D_MULTISAMPLE
       single layer from:
       
       GL_TEXTURE_2D_MULTISAMPLE_ARRAY
       gimage2DMSArray	GL_TEXTURE_2D_MULTISAMPLE_ARRAY
       
       SSBO
    */
};


enum AttribEx{
    AttribEx_NONE,
    AttribEx_INSTRATE = PHashString("INSTRATE"),
    AttribEx_DYNBUFFER = PHashString("DYNBUFFER"),
};

struct GenericLayout{
    LayoutType layout_type;
    u32 loc_array[2];
    GLSLTypes attribtype_array[32];
    u32 attribarraycount_array[32];
    u32 attribtype_count = 0;
    AttribEx attrib_ext;
};

u32 GetDominantAttrib(GenericLayout* layout,GLSLTypes* dominant,GLSLTypes* secondary){
    
    u32 index;
    
    for(u32 i = 0; i < layout->attribtype_count; i++){
        
        auto attrib = layout->attribtype_array[i];
        
        if(attrib == GLSLTypes_BUFFER || attrib == GLSLTypes_SAMPLER ||
           attrib == GLSLTypes_SAMPLER1D ||
           attrib == GLSLTypes_SAMPLER2D || attrib == GLSLTypes_SAMPLER3D ||
           attrib == GLSLTypes_SAMPLERCUBE || attrib == GLSLTypes_SAMPLER2DRECT ||
           attrib == GLSLTypes_SAMPLERBUFFER || attrib == GLSLTypes_SAMPLER2DMS ||
           attrib == GLSLTypes_SAMPLER1DSHADOW || attrib == GLSLTypes_SAMPLER2DSHADOW ||
           attrib == GLSLTypes_SAMPLER3DSHADOW || attrib == GLSLTypes_SAMPLERCUBESHADOW ||
           attrib == GLSLTypes_SAMPLER2DRECTSHADOW || attrib == GLSLTypes_TEXTURE1D ||
           attrib == GLSLTypes_TEXTURE2D || attrib == GLSLTypes_TEXTURE3D ||
           attrib == GLSLTypes_TEXTURECUBE || attrib == GLSLTypes_TEXTURE2DRECT ||
           attrib == GLSLTypes_TEXTUREBUFFER || attrib == GLSLTypes_TEXTURE2DMS ||
           attrib == GLSLTypes_IMAGE1D || attrib == GLSLTypes_IMAGE2D ||
           attrib == GLSLTypes_IMAGE3D || attrib == GLSLTypes_IMAGECUBE ||
           attrib == GLSLTypes_IMAGE2DRECT || attrib == GLSLTypes_IMAGEBUFFER ||
           attrib == GLSLTypes_IMAGE2DMS || attrib == GLSLTypes_UNIFORM){
            
            if((*dominant) == GLSLTypes_UNIFORM){
                *secondary = GLSLTypes_UNIFORM;
            }
            
            *dominant = attrib;
            index = i;
        }
        
    }
    
    _kill("no dominant attrib found\n",!(*dominant));
    
    return index;
}

VkDescriptorType GetInternalDescAttrib(GLSLTypes dominant_attrib,
                                       GLSLTypes secondary_attrib,AttribEx ext_attrib){
    
    VkDescriptorType type = {};
    
    if(dominant_attrib == GLSLTypes_SAMPLER1D || dominant_attrib == GLSLTypes_SAMPLER2D ||
       dominant_attrib == GLSLTypes_SAMPLER3D || dominant_attrib == GLSLTypes_SAMPLERCUBE ||
       dominant_attrib == GLSLTypes_SAMPLER2DRECT ||
       dominant_attrib == GLSLTypes_SAMPLER2DMS ||
       dominant_attrib == GLSLTypes_SAMPLER1DSHADOW ||
       dominant_attrib == GLSLTypes_SAMPLER2DSHADOW ||
       dominant_attrib == GLSLTypes_SAMPLER3DSHADOW ||
       dominant_attrib == GLSLTypes_SAMPLERCUBESHADOW ||
       dominant_attrib == GLSLTypes_SAMPLER2DRECTSHADOW){
        
        type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        
    }
    
    if(dominant_attrib == GLSLTypes_SAMPLER){
        type = VK_DESCRIPTOR_TYPE_SAMPLER;
    }
    
    if(dominant_attrib == GLSLTypes_TEXTURE1D ||
       dominant_attrib == GLSLTypes_TEXTURE2D || dominant_attrib == GLSLTypes_TEXTURE3D ||
       dominant_attrib == GLSLTypes_TEXTURECUBE ||
       dominant_attrib == GLSLTypes_TEXTURE2DRECT ||
       dominant_attrib == GLSLTypes_TEXTURE2DMS){
        type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    }
    
    if(dominant_attrib == GLSLTypes_IMAGE1D || dominant_attrib == GLSLTypes_IMAGE2D ||
       dominant_attrib == GLSLTypes_IMAGE3D || dominant_attrib == GLSLTypes_IMAGECUBE ||
       dominant_attrib == GLSLTypes_IMAGE2DRECT ||
       dominant_attrib == GLSLTypes_IMAGE2DMS){
        type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    
    //MARK:I am unsure about the GLSLTypes_IMAGEBUFFER part
    if(dominant_attrib == GLSLTypes_SAMPLERBUFFER ||
       dominant_attrib == GLSLTypes_IMAGEBUFFER){
        
        if(secondary_attrib == GLSLTypes_UNIFORM){
            type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;  
        }
        
        else{
            type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;    
        }
        
    }
    
    if(dominant_attrib == GLSLTypes_UNIFORM){
        
        if(ext_attrib == AttribEx_DYNBUFFER){
            type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        }
        
        else{
            type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
    }
    
    if(dominant_attrib == GLSLTypes_BUFFER){
        
        if(ext_attrib == AttribEx_DYNBUFFER){
            type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        }
        
        else{
            type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        }
    }
    //   VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
    
    return type;
}


ShaderType GetShaderType(const s8* file){
    
    s8 dst[5] = {};
    
    PGetFileExtension(dst,file,0);
    
    return  (ShaderType)PHashString(dst);
}


VkShaderStageFlagBits GetInternalShaderType(ShaderType type){
    
    switch(type){
        
        case ShaderType_VERTEX: {
            return VK_SHADER_STAGE_VERTEX_BIT;
        }break;
        
        case ShaderType_TESSELLATION_CONTROL: {
            return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }break;
        
        case ShaderType_TESSELLATION_EVALUATION: {
            return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }break;
        
        case ShaderType_GEOMETRY: {
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        }break;
        
        case ShaderType_FRAGMENT: {
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }break;
        
        case ShaderType_COMPUTE: {
            return VK_SHADER_STAGE_COMPUTE_BIT;
        }break;
        
    }
    
    return {};
}


logic IsAttrib(u32 hash){
    
    return
        hash == GLSLTypes_UNIFORM || hash == GLSLTypes_BOOL ||
        hash == GLSLTypes_INT || hash == GLSLTypes_UINT ||
        hash == GLSLTypes_FLOAT || hash == GLSLTypes_DOUBLE ||
        hash == GLSLTypes_BVEC1 || hash == GLSLTypes_BVEC2 ||
        hash == GLSLTypes_BVEC3 || hash == GLSLTypes_BVEC4 ||
        hash == GLSLTypes_IVEC1 || hash == GLSLTypes_IVEC2 ||
        hash == GLSLTypes_IVEC3 || hash == GLSLTypes_IVEC4 ||
        hash == GLSLTypes_UVEC1 || hash == GLSLTypes_UVEC2 ||
        hash == GLSLTypes_UVEC3 || hash == GLSLTypes_UVEC4 ||
        hash == GLSLTypes_VEC1 || hash == GLSLTypes_VEC2 ||
        hash == GLSLTypes_VEC3 || hash == GLSLTypes_VEC4 ||
        hash == GLSLTypes_DVEC1 || hash == GLSLTypes_DVEC2 ||
        hash == GLSLTypes_DVEC3 || hash == GLSLTypes_DVEC4 ||
        hash == GLSLTypes_MAT2 || hash == GLSLTypes_MAT3 ||
        hash == GLSLTypes_MAT4 || 
    
        hash == GLSLTypes_SAMPLER ||
        hash == GLSLTypes_SAMPLER1D || hash == GLSLTypes_SAMPLER2D ||
        hash == GLSLTypes_SAMPLER3D || hash == GLSLTypes_SAMPLERCUBE ||
        hash == GLSLTypes_SAMPLER2DRECT || hash == GLSLTypes_SAMPLERBUFFER ||
        hash == GLSLTypes_SAMPLER2DMS || hash == GLSLTypes_SAMPLER1DSHADOW ||
        hash == GLSLTypes_SAMPLER2DSHADOW || hash == GLSLTypes_SAMPLER3DSHADOW ||
        hash == GLSLTypes_SAMPLERCUBESHADOW || hash == GLSLTypes_SAMPLER2DRECTSHADOW ||
        hash == GLSLTypes_TEXTURE1D || hash == GLSLTypes_TEXTURE2D ||
        hash == GLSLTypes_TEXTURE3D || hash == GLSLTypes_TEXTURECUBE ||
        hash == GLSLTypes_TEXTURE2DRECT || hash == GLSLTypes_TEXTUREBUFFER ||
        hash == GLSLTypes_TEXTURE2DMS || hash == GLSLTypes_IMAGE1D ||
        hash == GLSLTypes_IMAGE2D || hash == GLSLTypes_IMAGE3D ||
        hash == GLSLTypes_IMAGECUBE || hash == GLSLTypes_IMAGE2DRECT ||
        hash == GLSLTypes_IMAGEBUFFER || hash == GLSLTypes_IMAGE2DMS ||
        hash == GLSLTypes_BUFFER || hash == GLSLTypes_RGBA8
        ;
}

logic IsAttribEx(u32 hash){
    return hash == AttribEx_INSTRATE || hash == AttribEx_DYNBUFFER;
}

/*
  TODO: we do not capture arrays yet
  We still need more types - storage image, textures ssbo etc
*/
void ProcessGenericLayout(s8* string,u32 hash,GenericLayout* layout,
                          u32 enable_parse_vert){
    
    auto tlayout = (LayoutType)hash;
    
    //set layout
    if((tlayout == LayoutType_IN && enable_parse_vert) || tlayout == LayoutType_DESC ||
       tlayout == LayoutType_PUSHCONST || tlayout == LayoutType_BINDING){
        // printf("isvalid\n");
        layout->layout_type = tlayout;
    }
    
    //attrib
    if(IsAttrib(hash)){
        // printf("matched %s : %d\n",string,hash);
        layout->attribtype_array[layout->attribtype_count] = (GLSLTypes)hash;
        layout->attribtype_count++;
    }
    
    if(IsAttribEx(hash)){
        layout->attrib_ext = (AttribEx)hash;
    }
    
    //set bind points
    if(PIsNumeric(string[0])){
        
        if(layout->layout_type == LayoutType_DESC){
            layout->loc_array[0] = atoi(string);
        }
        
        else if(layout->layout_type == LayoutType_BINDING){
            layout->loc_array[1] = atoi(string);
        }
        
        else{
            //potentially a vertex input
            layout->loc_array[0] = atoi(string); 
        }
    }
    
}

_persist logic skip_until_endif = false;

void TokenizeLine(s8* string,ParseStateStack* pstate,
                  GenericLayout* layout_array,u32* layout_count,u32 enable_parse_vert){
    
    u32 len = strlen(string);
    u32 cur = 0;
    
    auto cur_layout = &layout_array[(*layout_count) - 1];
    
    
    for(;;){
        
        if(cur >= len || IsComment(string[cur],string[cur + 1])){
            break;
        }
        
        if(IsPreprocessor(string[cur])){
            
            PSkipWhiteSpace(string,&cur);
            SkipBlockComments(string,&cur,pstate);
            
            s8 dst[512] = {};
            
            PGetSymbol(dst,string,&cur,0);
            
            if(PHashString(dst) == PHashString("#ifdef")){
                
                PSkipWhiteSpace(string,&cur);
                SkipBlockComments(string,&cur,pstate);
                
                s8 dst[512] = {};
                
                PGetSymbol(dst,string,&cur,0);
                
                auto macro_hash = PHashString(dst);
                
                logic is_defined = false;
                
                for(u32 i = 0; i < macro_count; i++){
                    
                    if(macro_hash == macro_array[i]){
                        is_defined = true;
                        break;
                    }
                    
                }
                
                if(!is_defined){
                    skip_until_endif = true;
                }
                
            }
            
            else if(PHashString(dst) == PHashString("#endif")){
                skip_until_endif = false;
            }
            
            break;
        }
        
        if(skip_until_endif){
            break;
        }
        
        
        PSkipWhiteSpace(string,&cur);
        SkipBlockComments(string,&cur,pstate);
        
        s8 dst[512] = {};
        
        PGetSymbol(dst,string,&cur,0);
        
        if(strlen(dst)){
            
            auto hash = PHashString(dst);
            
            if(hash == PHashString("layout")){
                
                PushState(pstate,ParseState_LAYOUT);
                cur_layout = &layout_array[(*layout_count)];
                (*layout_count)++;
                
                for(u32 k = 0; k < _arraycount(cur_layout->attribarraycount_array); k++){
                    cur_layout->attribarraycount_array[k] = 1;
                }
                
                // printf("%p : ",(void*)cur_layout);
                // printf("start layout - %s\n",string);
            }
            
            if(GetState(pstate) == ParseState_LAYOUT || GetState(pstate) == ParseState_SCOPEBLOCK){
                // printf("%p : ",(void*)cur_layout);
                // printf("capture %s : hash %d\n",dst,hash);	
                ProcessGenericLayout(dst,hash,cur_layout,enable_parse_vert);
            }
            
            if(GetState(pstate) == ParseState_SCOPEARRAY){
                cur_layout->attribarraycount_array[cur_layout->attribtype_count - 1] = atoi(dst);	
            }
            
        }
        
        
        if(GetState(pstate) == ParseState_LAYOUT && string[cur] == '{'){
            PushState(pstate,ParseState_SCOPEBLOCK);
            // printf("got %c\n",string[cur]);
        }
        
        if(GetState(pstate) == ParseState_SCOPEBLOCK && string[cur] == '}'){
            PopState(pstate);
            // printf("end %c\n",string[cur]);
        }
        
        
        if((GetState(pstate) == ParseState_LAYOUT ||
            GetState(pstate) == ParseState_SCOPEBLOCK) && string[cur] == '['){
            PushState(pstate,ParseState_SCOPEARRAY);
            // printf("got %c\n",string[cur]);
        }
        
        if(GetState(pstate) == ParseState_SCOPEARRAY && string[cur] == ']'){
            PopState(pstate);
            // printf("end %c\n",string[cur]);
        }
        
        if(GetState(pstate) == ParseState_LAYOUT && string[cur] == ';'){
            PopState(pstate);
            
            if(cur_layout->layout_type == 0){
                (*cur_layout) = {};
                (*layout_count)--;
            }
            // printf("end layout\n");
        }
        
        cur++;
        
    }
    
}

/*
  table - Shader stage
  Vert entries [if any]
  Desc Entries
  PushConst Entries
  
*/

void GetInternalFormatAndSize(GLSLTypes type,VkFormat* outformat,u32* size){
    
    switch(type){
        
        case GLSLTypes_UNIFORM: {
            printf("TODO: handle this\n");
        }break;
        
        case GLSLTypes_BOOL: {
            *outformat = VK_FORMAT_R32_UINT;
            *size = sizeof(u32);
        }break;
        
        case GLSLTypes_INT: {
            *outformat = VK_FORMAT_R32_SINT;
            *size = sizeof(u32);
        }break;
        
        case GLSLTypes_UINT: {
            *outformat = VK_FORMAT_R32_UINT;
            *size = sizeof(u32);
        }break;
        
        case GLSLTypes_FLOAT: {
            *outformat = VK_FORMAT_R32_SFLOAT;
            *size = sizeof(f32);
        }break;
        
        case GLSLTypes_DOUBLE: {
            *outformat = VK_FORMAT_R64_SFLOAT;
            *size = sizeof(f64);
        }break;
        
        case GLSLTypes_BVEC1: {
            *outformat = VK_FORMAT_R32_UINT;
            *size = sizeof(u32);    
        }break;
        
        case GLSLTypes_BVEC2: {
            *outformat = VK_FORMAT_R32G32_UINT;
            *size = sizeof(u32) * 2;
        }break;
        
        case GLSLTypes_BVEC3: {
            *outformat = VK_FORMAT_R32G32B32_UINT;
            *size = sizeof(u32) * 3;
        }break;
        case GLSLTypes_BVEC4: {
            *outformat = VK_FORMAT_R32G32B32A32_UINT;
            *size = sizeof(u32) * 4;
        }break;
        
        case GLSLTypes_IVEC1: {
            *outformat = VK_FORMAT_R32_SINT;
            *size = sizeof(u32);        
        }break;
        
        case GLSLTypes_IVEC2: {
            
            *outformat = VK_FORMAT_R32G32_SINT;
            *size = sizeof(u32) * 2;  
        }break;
        
        case GLSLTypes_IVEC3: {
            *outformat = VK_FORMAT_R32G32B32_SINT;
            *size = sizeof(u32) * 3;  
        }break;
        
        case GLSLTypes_IVEC4: {
            *outformat = VK_FORMAT_R32G32B32A32_SINT;
            *size = sizeof(u32) * 4;
        }break;
        
        case GLSLTypes_UVEC1: {
            *outformat = VK_FORMAT_R32_UINT;
            *size = sizeof(u32);            
        }break;
        
        case GLSLTypes_UVEC2: {
            *outformat = VK_FORMAT_R32G32_UINT;
            *size = sizeof(u32) * 2;            
        }break;
        
        case GLSLTypes_UVEC3: {
            *outformat = VK_FORMAT_R32G32B32_UINT;
            *size = sizeof(u32) * 3;            
        }break;
        
        case GLSLTypes_UVEC4: {
            *outformat = VK_FORMAT_R32G32B32A32_UINT;
            *size = sizeof(u32) * 4;            
        }break;
        
        case GLSLTypes_VEC1: {
            *outformat = VK_FORMAT_R32_SFLOAT;
            *size = sizeof(f32) * 1;            
        }break;
        
        case GLSLTypes_VEC2: {
            *outformat = VK_FORMAT_R32G32_SFLOAT;
            *size = sizeof(f32) * 2;            
        }break;
        
        case GLSLTypes_VEC3: {
            *outformat = VK_FORMAT_R32G32B32_SFLOAT;
            *size = sizeof(f32) * 3;            
        }break;
        
        case GLSLTypes_VEC4: {
            *outformat = VK_FORMAT_R32G32B32A32_SFLOAT;
            *size = sizeof(f32) * 4;            
        }break;
        
        
        case GLSLTypes_DVEC1: {
            *outformat = VK_FORMAT_R64_SFLOAT;
            *size = sizeof(f64);    
        }break;
        
        case GLSLTypes_DVEC2: {
            *outformat = VK_FORMAT_R64G64_SFLOAT;
            *size = sizeof(f64) * 2;      
        }break;
        
        case GLSLTypes_DVEC3: {
            *outformat = VK_FORMAT_R64G64B64_SFLOAT;
            *size = sizeof(f64) * 3;          
        }break;
        
        case GLSLTypes_DVEC4: {
            *outformat = VK_FORMAT_R64G64B64A64_SFLOAT;
            *size = sizeof(f64) * 4;              
        }break;
        
        
        
        case GLSLTypes_MAT2 : {
            *outformat = VK_FORMAT_R32G32B32A32_SFLOAT;
            *size = sizeof(f32) * 2 * 2;            
        }break;
        
        case GLSLTypes_MAT3: {
            *outformat = VK_FORMAT_R32G32B32A32_SFLOAT;
            *size = sizeof(f32) * 3 * 3;            
        }break;
        
        case GLSLTypes_MAT4: {
            *outformat = VK_FORMAT_R32G32B32A32_SFLOAT;
            *size = sizeof(f32) * 4 * 4;
        }break;
        
#if 1
        
        default:{
            printf("unhandled type: %d\n",(u32)type);
        }break;
        
#endif
        
    }
    
}

void GenerateShaderTable(s8* infile_string,s8* outfile_string){
    
    auto file = FOpenFile(infile_string,F_FLAG_READONLY);
    
    ptrsize size;
    u32 cur = 0;
    auto buffer = FReadFileToBuffer(file,&size);
    
    FCloseFile(file);
    
    auto type = GetShaderType(infile_string);
    
    logic enable_parse_vert = type == ShaderType_VERTEX;
    
    ParseStateStack pstate;
    
    GenericLayout genericlayout_array[1024] = {};
    u32 genericlayout_count = 0;
    
    //parse the glsl source
    for(;;){
        
        PSkipWhiteSpace(buffer,&cur);
        
        s8 dst[1024] = {};
        
        PGetLine(dst,buffer,&cur,0);
        
        TokenizeLine(dst,&pstate,genericlayout_array,&genericlayout_count,enable_parse_vert);
        
        if(cur >= size){
            break;
        }
        
    }
    
    unalloc(buffer);
    
    struct VertexEntry{
        VkFormat format;
        u32 size;
    };
    
    struct DescEntry{
        VkDescriptorType type;
        u32 set;
        u32 bind;
        u32 array_count;
    };
    
    struct PushConstEntry{
        VkFormat format;
    };
    
    struct VertexLayout{
        VertexEntry entry_array[16];
        u16 entry_count = 0;
        u16 size;
    };
    
    struct DescLayout{
        DescEntry entry_array[16];
        u32 entry_count = 0;
    };
    
    struct PushConstLayout{
        PushConstEntry entry_array[16];
        u16 entry_count = 0;
        u16 size;
    };
    
    VertexLayout vlayout = {};
    VertexLayout instlayout = {};
    DescLayout dlayout = {};
    PushConstLayout playout = {};
    
    qsort(&genericlayout_array[0],genericlayout_count,sizeof(genericlayout_array[0]),
          [](const void * a, const void* b)->s32 {
          
          auto a_layout = (GenericLayout*)a;
          auto b_layout = (GenericLayout*)b;
          
          return a_layout->loc_array[0] - b_layout->loc_array[0];
          });
    
    
    for(u32 i = 0; i < genericlayout_count; i++){
        
        const auto layout = &genericlayout_array[i];
        
        if(enable_parse_vert && layout->layout_type == LayoutType_IN){
            
            //save verts
            for(u32 j = 0; j < layout->attribtype_count;j++){
                
                VkFormat format = {};
                u32 size = 0;
                
                GetInternalFormatAndSize(layout->attribtype_array[j],&format,&size);
                
                if(layout->attrib_ext == AttribEx_NONE){
                    
                    vlayout.entry_array[vlayout.entry_count] = {format,size};
                    vlayout.entry_count++;
                    
                    vlayout.size += size; 
                }
                
                else{
                    instlayout.entry_array[instlayout.entry_count] = {format,size};
                    instlayout.entry_count++;
                    
                    instlayout.size += size; 
                }
                
            }
            
        }
        
        if(layout->layout_type == LayoutType_DESC || layout->layout_type == LayoutType_BINDING){
            
            GLSLTypes dominant_attrib = {};
            GLSLTypes secondary_attrib = {};
            
            auto index = GetDominantAttrib(layout,&dominant_attrib,&secondary_attrib);
            
            auto internaltype =
                GetInternalDescAttrib(dominant_attrib,secondary_attrib,layout->attrib_ext);
            
            // VkDescriptorType type;
            // u32 set;
            // u32 bind;
            // u32 array_count;
            
            dlayout.entry_array[dlayout.entry_count] = {internaltype,layout->loc_array[0],
                layout->loc_array[1],layout->attribarraycount_array[index]};
            dlayout.entry_count++;
        }
        
        if(layout->layout_type == LayoutType_PUSHCONST){
            
            for(u32 j = 0; j < layout->attribtype_count;j++){
                
                VkFormat format = {};
                u32 size = 0;
                
                if(layout->attribtype_array[j] == GLSLTypes_UNIFORM){
                    continue;
                }
                
                GetInternalFormatAndSize(layout->attribtype_array[j],&format,&size);
                playout.entry_array[playout.entry_count] = {format};
                playout.entry_count++;
                
                playout.size += size;
                
                playout.size = _align16(playout.size);
                
            }
            
        }
        
    }
    
    //Write file
    auto spvfile = FOpenFile(outfile_string,F_FLAG_READONLY);
    
    ptrsize spvsize = 0;
    auto spvdata = FReadFileToBuffer(spvfile,&spvsize);
    
    auto len = strlen(outfile_string);
    
    outfile_string[len - 3] = 's';
    outfile_string[len - 2] = 'p';
    outfile_string[len - 1] = 'x';
    
    auto outfile = FOpenFile(outfile_string,F_FLAG_READWRITE | F_FLAG_CREATE |
                             F_FLAG_TRUNCATE);
    
#define _encode(a,b,c,d) (u32)  (((u32)(a << 0)) | ((u32)(b << 8)) | ((u32)(c << 16)) | ((u32)(d << 24)))
    
    u32 headertag = _encode('S','P','X',' ');
    FWrite(outfile,&headertag,sizeof(headertag));
    
    //write shadertype
    auto internal_type = GetInternalShaderType(type);
    FWrite(outfile,&internal_type,sizeof(internal_type));
    
    if(vlayout.size){
        
        auto layout = LayoutType_IN;
        
        FWrite(outfile,&layout,sizeof(layout));
        FWrite(outfile,&vlayout.size,sizeof(vlayout.size));
        FWrite(outfile,&vlayout.entry_count,sizeof(vlayout.entry_count));
        
        // printf("Writing vert %d %d - %d\n",vlayout.size,vlayout.entry_count,(u32)layout);
        
        for(u32 i = 0; i < vlayout.entry_count; i++){
            
            auto entry = vlayout.entry_array[i];
            
            FWrite(outfile,&entry,sizeof(entry));
            
            // printf("wrote vert %d\n",entry.size);
            
        }
        
        if(instlayout.size){
            
            auto layout = LayoutType_IN;
            
            FWrite(outfile,&layout,sizeof(layout));
            FWrite(outfile,&instlayout.size,sizeof(instlayout.size));
            FWrite(outfile,&instlayout.entry_count,sizeof(instlayout.entry_count));
            
            // printf("Writing inst %d %d\n",instlayout.size,instlayout.entry_count);
            
            for(u32 i = 0; i < instlayout.entry_count; i++){
                
                auto entry = instlayout.entry_array[i];
                
                FWrite(outfile,&entry,sizeof(entry));
                // printf("wrote vert %d\n",entry.size);
                
            }
            
        }
        
    }
    
    if(dlayout.entry_count){
        
#if 1
        
        auto layout = LayoutType_DESC;
        
        FWrite(outfile,&layout,sizeof(layout));
        FWrite(outfile,&dlayout.entry_count,sizeof(dlayout.entry_count));
        
        for(u32 i = 0; i < dlayout.entry_count; i++){
            
            auto entry = dlayout.entry_array[i];
            
            FWrite(outfile,&entry,sizeof(entry));
            
        }
        
#endif
        
    }
    
    if(playout.size){
        
        auto layout = LayoutType_PUSHCONST;
        
        FWrite(outfile,&layout,sizeof(layout));
        FWrite(outfile,&playout.size,sizeof(playout.size));
        FWrite(outfile,&playout.entry_count,sizeof(playout.entry_count));
        
        // printf("Writing pushconst %d %d\n",playout.size,playout.entry_count);
        
        for(u32 i = 0; i < playout.entry_count; i++){
            
            auto entry = playout.entry_array[i];
            
            FWrite(outfile,&entry,sizeof(entry));
            
            // printf("wrote pushconst\n");
            
        }
        
    }
    
    u32 spvtag = -1;
    FWrite(outfile,&spvtag,sizeof(spvtag));
    FWrite(outfile,&spvsize,sizeof(spvsize));
    FWrite(outfile,spvdata,spvsize);
    
    // printf("shader size %llu\n",spvsize);  
    
    FCloseFile(spvfile);
    unalloc(spvdata);
}
