#pragma once
#include "aallocator.h"
#include "stdio.h"

#define _log_string 0

#include "spx_common.h"


struct VertexEntryEx{
    VkFormat format;
    u32 size;
    u32 index;
    
    
#if _log_string
    
    s8 format_string[128];
    
#endif
};


struct VertexLayoutEx{
    VertexEntryEx entry_array[16];
    u16 entry_count = 0;
    u16 size;
};



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

enum GLSLType{
    
    GLSLType_STRUCT = PHashString("struct"),
    
    GLSLType_BOOL = PHashString("bool"),
    GLSLType_INT = PHashString("int"),
    GLSLType_UINT = PHashString("uint"),
    GLSLType_FLOAT = PHashString("float"),
    GLSLType_DOUBLE = PHashString("double"),
    GLSLType_BVEC1 = PHashString("bvec1"),
    GLSLType_BVEC2 = PHashString("bvec2"),
    GLSLType_BVEC3 = PHashString("bvec3"),
    GLSLType_BVEC4 = PHashString("bvec4"),
    
    GLSLType_IVEC1 = PHashString("ivec1"),
    GLSLType_IVEC2 = PHashString("ivec2"),
    GLSLType_IVEC3 = PHashString("ivec3"),
    GLSLType_IVEC4 = PHashString("ivec4"),
    
    GLSLType_UVEC1 = PHashString("uvec1"),
    GLSLType_UVEC2 = PHashString("uvec2"),
    GLSLType_UVEC3 = PHashString("uvec3"),
    GLSLType_UVEC4 = PHashString("uvec4"),
    
    GLSLType_VEC1 = PHashString("vec1"),
    GLSLType_VEC2 = PHashString("vec2"),
    GLSLType_VEC3 = PHashString("vec3"),
    GLSLType_VEC4 = PHashString("vec4"),
    
    GLSLType_DVEC1 = PHashString("dvec1"),
    GLSLType_DVEC2 = PHashString("dvec2"),
    GLSLType_DVEC3 = PHashString("dvec3"),
    GLSLType_DVEC4 = PHashString("dvec4"),
    
    GLSLType_MAT2 = PHashString("mat2"),
    GLSLType_MAT3 = PHashString("mat3"),
    GLSLType_MAT4 = PHashString("mat4"),
    
    //These are opaque types that are only useful in binding
    GLSLType_BUFFER = PHashString("buffer"),
    
    GLSLType_SAMPLER = PHashString("sampler"),
    
    GLSLType_SAMPLER1D = PHashString("sampler1D"),
    GLSLType_SAMPLER2D = PHashString("sampler2D"),
    GLSLType_SAMPLER3D = PHashString("sampler3D"),
    GLSLType_SAMPLERCUBE = PHashString("samplerCube"),
    GLSLType_SAMPLER2DRECT = PHashString("sampler2DRect"),
    GLSLType_SAMPLERBUFFER = PHashString("samplerBuffer"),
    GLSLType_SAMPLER2DMS = PHashString("sampler2DMS"),
    
    GLSLType_SAMPLER1DSHADOW = PHashString("sampler1DShadow"),
    GLSLType_SAMPLER2DSHADOW = PHashString("sampler2DShadow"),
    GLSLType_SAMPLER3DSHADOW = PHashString("sampler3DShadow"),
    GLSLType_SAMPLERCUBESHADOW = PHashString("samplerCubeShadow"),
    GLSLType_SAMPLER2DRECTSHADOW = PHashString("sampler2DRectShadow"),
    
    //TODO: every sampler type has a corresponding texture type
    GLSLType_TEXTURE1D = PHashString("texture1D"),
    GLSLType_TEXTURE2D = PHashString("texture2D"),
    GLSLType_TEXTURE3D = PHashString("texture3D"),
    GLSLType_TEXTURECUBE = PHashString("textureCube"),
    GLSLType_TEXTURE2DRECT = PHashString("texture2DRect"),
    GLSLType_TEXTUREBUFFER = PHashString("textureBuffer"),
    GLSLType_TEXTURE2DMS = PHashString("texture2DMS"),
    
    GLSLType_IMAGE1D = PHashString("image1D"),
    GLSLType_IMAGE2D = PHashString("image2D"),
    GLSLType_IMAGE3D = PHashString("image3D"),
    GLSLType_IMAGECUBE = PHashString("imageCube"),
    GLSLType_IMAGE2DRECT = PHashString("image2DRect"),
    GLSLType_IMAGEBUFFER = PHashString("imageBuffer"),
    GLSLType_IMAGE2DMS = PHashString("image2DMS"),
    
    
    GLSLType_UNIFORM = PHashString("uniform"),
    GLSLType_RGBA8 = PHashString("rgba8"),
    
    
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
    
    
    AttribEx_MATERIAL_DIFFUSE = PHashString("MATERIAL_DIFFUSE"),
    AttribEx_MATERIAL_SPECULAR = PHashString("MATERIAL_SPECULAR"),
    AttribEx_MATERIAL_NORMAL = PHashString("MATERIAL_NORMAL"),
    AttribEx_DEF_MATERIAL = PHashString("DEF_MATERIAL"),
};

VkDescriptorType GetInternalDescAttrib(GLSLType dominant_attrib,
                                       GLSLType secondary_attrib,AttribEx ext_attrib){
    
    VkDescriptorType type = {};
    
    if(dominant_attrib == GLSLType_SAMPLER1D || dominant_attrib == GLSLType_SAMPLER2D ||
       dominant_attrib == GLSLType_SAMPLER3D || dominant_attrib == GLSLType_SAMPLERCUBE ||
       dominant_attrib == GLSLType_SAMPLER2DRECT ||
       dominant_attrib == GLSLType_SAMPLER2DMS ||
       dominant_attrib == GLSLType_SAMPLER1DSHADOW ||
       dominant_attrib == GLSLType_SAMPLER2DSHADOW ||
       dominant_attrib == GLSLType_SAMPLER3DSHADOW ||
       dominant_attrib == GLSLType_SAMPLERCUBESHADOW ||
       dominant_attrib == GLSLType_SAMPLER2DRECTSHADOW){
        
        type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        
    }
    
    if(dominant_attrib == GLSLType_SAMPLER){
        type = VK_DESCRIPTOR_TYPE_SAMPLER;
    }
    
    if(dominant_attrib == GLSLType_TEXTURE1D ||
       dominant_attrib == GLSLType_TEXTURE2D || dominant_attrib == GLSLType_TEXTURE3D ||
       dominant_attrib == GLSLType_TEXTURECUBE ||
       dominant_attrib == GLSLType_TEXTURE2DRECT ||
       dominant_attrib == GLSLType_TEXTURE2DMS){
        type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    }
    
    if(dominant_attrib == GLSLType_IMAGE1D || dominant_attrib == GLSLType_IMAGE2D ||
       dominant_attrib == GLSLType_IMAGE3D || dominant_attrib == GLSLType_IMAGECUBE ||
       dominant_attrib == GLSLType_IMAGE2DRECT ||
       dominant_attrib == GLSLType_IMAGE2DMS){
        type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    
    //MARK:I am unsure about the GLSLType_IMAGEBUFFER part
    if(dominant_attrib == GLSLType_SAMPLERBUFFER ||
       dominant_attrib == GLSLType_IMAGEBUFFER){
        
        if(secondary_attrib == GLSLType_UNIFORM){
            type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;  
        }
        
        else{
            type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;    
        }
        
    }
    
    if(dominant_attrib == GLSLType_UNIFORM){
        
        if(ext_attrib == AttribEx_DYNBUFFER){
            type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        }
        
        else{
            type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
    }
    
    if(dominant_attrib == GLSLType_BUFFER){
        
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


logic IsAttrib(u64 hash){
    
    return
        hash == GLSLType_UNIFORM || hash == GLSLType_BOOL ||
        hash == GLSLType_INT || hash == GLSLType_UINT ||
        hash == GLSLType_FLOAT || hash == GLSLType_DOUBLE ||
        hash == GLSLType_BVEC1 || hash == GLSLType_BVEC2 ||
        hash == GLSLType_BVEC3 || hash == GLSLType_BVEC4 ||
        hash == GLSLType_IVEC1 || hash == GLSLType_IVEC2 ||
        hash == GLSLType_IVEC3 || hash == GLSLType_IVEC4 ||
        hash == GLSLType_UVEC1 || hash == GLSLType_UVEC2 ||
        hash == GLSLType_UVEC3 || hash == GLSLType_UVEC4 ||
        hash == GLSLType_VEC1 || hash == GLSLType_VEC2 ||
        hash == GLSLType_VEC3 || hash == GLSLType_VEC4 ||
        hash == GLSLType_DVEC1 || hash == GLSLType_DVEC2 ||
        hash == GLSLType_DVEC3 || hash == GLSLType_DVEC4 ||
        hash == GLSLType_MAT2 || hash == GLSLType_MAT3 ||
        hash == GLSLType_MAT4 || 
    
        hash == GLSLType_SAMPLER ||
        hash == GLSLType_SAMPLER1D || hash == GLSLType_SAMPLER2D ||
        hash == GLSLType_SAMPLER3D || hash == GLSLType_SAMPLERCUBE ||
        hash == GLSLType_SAMPLER2DRECT || hash == GLSLType_SAMPLERBUFFER ||
        hash == GLSLType_SAMPLER2DMS || hash == GLSLType_SAMPLER1DSHADOW ||
        hash == GLSLType_SAMPLER2DSHADOW || hash == GLSLType_SAMPLER3DSHADOW ||
        hash == GLSLType_SAMPLERCUBESHADOW || hash == GLSLType_SAMPLER2DRECTSHADOW ||
        hash == GLSLType_TEXTURE1D || hash == GLSLType_TEXTURE2D ||
        hash == GLSLType_TEXTURE3D || hash == GLSLType_TEXTURECUBE ||
        hash == GLSLType_TEXTURE2DRECT || hash == GLSLType_TEXTUREBUFFER ||
        hash == GLSLType_TEXTURE2DMS || hash == GLSLType_IMAGE1D ||
        hash == GLSLType_IMAGE2D || hash == GLSLType_IMAGE3D ||
        hash == GLSLType_IMAGECUBE || hash == GLSLType_IMAGE2DRECT ||
        hash == GLSLType_IMAGEBUFFER || hash == GLSLType_IMAGE2DMS ||
        hash == GLSLType_BUFFER || hash == GLSLType_RGBA8
        ;
}

logic IsAttribEx(u64 hash){
    
    AttribEx array[] = {
        
        AttribEx_INSTRATE,
        AttribEx_DYNBUFFER,
        AttribEx_MATERIAL_DIFFUSE,
        AttribEx_MATERIAL_SPECULAR,
        AttribEx_MATERIAL_NORMAL,
        AttribEx_DEF_MATERIAL,
    };
    
    for(u32 i = 0; i < _arraycount(array);i++){
        
        if(hash == array[i]){
            return true;
        }
        
    }
    
    return false;
}



/*
  table - Shader stage
  Vert entries [if any]
  Desc Entries
  PushConst Entries
  
*/

void GetInternalFormatAndSize(GLSLType type,VkFormat* outformat,u32* size){
    
    switch(type){
        
        case GLSLType_UNIFORM: {
            printf("TODO: handle this\n");
        }break;
        
        case GLSLType_BOOL: {
            *outformat = VK_FORMAT_R32_UINT;
            *size = sizeof(u32);
        }break;
        
        case GLSLType_INT: {
            *outformat = VK_FORMAT_R32_SINT;
            *size = sizeof(u32);
        }break;
        
        case GLSLType_UINT: {
            *outformat = VK_FORMAT_R32_UINT;
            *size = sizeof(u32);
        }break;
        
        case GLSLType_FLOAT: {
            *outformat = VK_FORMAT_R32_SFLOAT;
            *size = sizeof(f32);
        }break;
        
        case GLSLType_DOUBLE: {
            *outformat = VK_FORMAT_R64_SFLOAT;
            *size = sizeof(f64);
        }break;
        
        case GLSLType_BVEC1: {
            *outformat = VK_FORMAT_R32_UINT;
            *size = sizeof(u32);    
        }break;
        
        case GLSLType_BVEC2: {
            *outformat = VK_FORMAT_R32G32_UINT;
            *size = sizeof(u32) * 2;
        }break;
        
        case GLSLType_BVEC3: {
            *outformat = VK_FORMAT_R32G32B32_UINT;
            *size = sizeof(u32) * 3;
        }break;
        case GLSLType_BVEC4: {
            *outformat = VK_FORMAT_R32G32B32A32_UINT;
            *size = sizeof(u32) * 4;
        }break;
        
        case GLSLType_IVEC1: {
            *outformat = VK_FORMAT_R32_SINT;
            *size = sizeof(u32);        
        }break;
        
        case GLSLType_IVEC2: {
            
            *outformat = VK_FORMAT_R32G32_SINT;
            *size = sizeof(u32) * 2;  
        }break;
        
        case GLSLType_IVEC3: {
            *outformat = VK_FORMAT_R32G32B32_SINT;
            *size = sizeof(u32) * 3;  
        }break;
        
        case GLSLType_IVEC4: {
            *outformat = VK_FORMAT_R32G32B32A32_SINT;
            *size = sizeof(u32) * 4;
        }break;
        
        case GLSLType_UVEC1: {
            *outformat = VK_FORMAT_R32_UINT;
            *size = sizeof(u32);            
        }break;
        
        case GLSLType_UVEC2: {
            *outformat = VK_FORMAT_R32G32_UINT;
            *size = sizeof(u32) * 2;            
        }break;
        
        case GLSLType_UVEC3: {
            *outformat = VK_FORMAT_R32G32B32_UINT;
            *size = sizeof(u32) * 3;            
        }break;
        
        case GLSLType_UVEC4: {
            *outformat = VK_FORMAT_R32G32B32A32_UINT;
            *size = sizeof(u32) * 4;            
        }break;
        
        case GLSLType_VEC1: {
            *outformat = VK_FORMAT_R32_SFLOAT;
            *size = sizeof(f32) * 1;            
        }break;
        
        case GLSLType_VEC2: {
            *outformat = VK_FORMAT_R32G32_SFLOAT;
            *size = sizeof(f32) * 2;            
        }break;
        
        case GLSLType_VEC3: {
            *outformat = VK_FORMAT_R32G32B32_SFLOAT;
            *size = sizeof(f32) * 3;            
        }break;
        
        case GLSLType_VEC4: {
            *outformat = VK_FORMAT_R32G32B32A32_SFLOAT;
            *size = sizeof(f32) * 4;            
        }break;
        
        
        case GLSLType_DVEC1: {
            *outformat = VK_FORMAT_R64_SFLOAT;
            *size = sizeof(f64);    
        }break;
        
        case GLSLType_DVEC2: {
            *outformat = VK_FORMAT_R64G64_SFLOAT;
            *size = sizeof(f64) * 2;      
        }break;
        
        case GLSLType_DVEC3: {
            *outformat = VK_FORMAT_R64G64B64_SFLOAT;
            *size = sizeof(f64) * 3;          
        }break;
        
        case GLSLType_DVEC4: {
            *outformat = VK_FORMAT_R64G64B64A64_SFLOAT;
            *size = sizeof(f64) * 4;              
        }break;
        
        
        
        case GLSLType_MAT2 : {
            *outformat = VK_FORMAT_R32G32B32A32_SFLOAT;
            *size = sizeof(f32) * 2 * 2;            
        }break;
        
        case GLSLType_MAT3: {
            *outformat = VK_FORMAT_R32G32B32A32_SFLOAT;
            *size = sizeof(f32) * 3 * 3;            
        }break;
        
        case GLSLType_MAT4: {
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

logic IsStruct(EvalChar* eval_buffer,u32 count){
    
    if(PHashString(eval_buffer[0].string) == PHashString("struct")){
        
        return true;
    }
    
    return false;
}


logic IsLayout(EvalChar* eval_buffer,u32 count){
    
    if(PHashString(eval_buffer[0].string) == PHashString("layout")){
        
        return true;
    }
    
    return false;
}

logic IsGLSLType(u64 hash){
    
    GLSLType array[] = {
        
        GLSLType_STRUCT,
        
        GLSLType_BOOL,
        GLSLType_INT,
        GLSLType_UINT,
        GLSLType_FLOAT,
        GLSLType_DOUBLE,
        GLSLType_BVEC1,
        GLSLType_BVEC2,
        GLSLType_BVEC3,
        GLSLType_BVEC4,
        
        GLSLType_IVEC1,
        GLSLType_IVEC2,
        GLSLType_IVEC3,
        GLSLType_IVEC4,
        
        GLSLType_UVEC1,
        GLSLType_UVEC2,
        GLSLType_UVEC3,
        GLSLType_UVEC4,
        
        GLSLType_VEC1,
        GLSLType_VEC2,
        GLSLType_VEC3,
        GLSLType_VEC4,
        
        GLSLType_DVEC1,
        GLSLType_DVEC2,
        GLSLType_DVEC3,
        GLSLType_DVEC4,
        
        GLSLType_MAT2,
        GLSLType_MAT3,
        GLSLType_MAT4,
        
        GLSLType_BUFFER,
        
        GLSLType_SAMPLER,
        
        GLSLType_SAMPLER1D,
        GLSLType_SAMPLER2D,
        GLSLType_SAMPLER3D,
        GLSLType_SAMPLERCUBE,
        GLSLType_SAMPLER2DRECT,
        GLSLType_SAMPLERBUFFER,
        GLSLType_SAMPLER2DMS,
        
        GLSLType_SAMPLER1DSHADOW,
        GLSLType_SAMPLER2DSHADOW,
        GLSLType_SAMPLER3DSHADOW,
        GLSLType_SAMPLERCUBESHADOW,
        GLSLType_SAMPLER2DRECTSHADOW,
        
        GLSLType_TEXTURE1D,
        GLSLType_TEXTURE2D,
        GLSLType_TEXTURE3D,
        GLSLType_TEXTURECUBE,
        GLSLType_TEXTURE2DRECT,
        GLSLType_TEXTUREBUFFER,
        GLSLType_TEXTURE2DMS,
        
        GLSLType_IMAGE1D,
        GLSLType_IMAGE2D,
        GLSLType_IMAGE3D,
        GLSLType_IMAGECUBE,
        GLSLType_IMAGE2DRECT,
        GLSLType_IMAGEBUFFER,
        GLSLType_IMAGE2DMS,
        
        
        GLSLType_UNIFORM,
        GLSLType_RGBA8,
    };
    
    for(u32 i = 0; i < _arraycount(array);i++){
        
        if(hash == array[i]){
            
            return true;
        }
    }
    
    return false;
}

void TagEvalBuffer(EvalChar* eval_buffer,u32 count){
    
    for(u32 i = 0; i < count; i++){
        
        auto c = &eval_buffer[i];
        
        //        printf("%s",c->string);
        
        if(c->hash == PHashString("struct")){
            c->tag = TAG_STRUCT;
            
            //            printf("[struct] ");
        }
        
        else if(c->hash == PHashString("enum")){
            c->tag = TAG_ENUM;
            
            //            printf("[struct] ");
        }
        
        else if(IsGLSLType(c->hash)){
            c->tag = TAG_CTYPE;
            
            //            printf("[type] ");
        }
        
        else if(IsAttribEx(c->hash)){
            c->tag = TAG_KEY;
            
            //            printf("[key] ");
        }
        
        else if(c->hash == PHashString("(")){
            c->tag = TAG_START_ARG;
        }
        
        else if(c->hash == PHashString(")")){
            c->tag = TAG_END_ARG;
        }
        
        else if(c->hash == PHashString("*")){
            c->tag = TAG_INDIR;
        }
        
        else if(c->hash == PHashString("=")){
            c->tag = TAG_ASSIGN;
        }
        
        else if(c->hash == PHashString("\"")){
            c->tag = TAG_DOUBLE_QUOTE;
        }
        
        else if(PIsStringFloat(c->string) || PIsStringInt(c->string)){
            c->tag = TAG_VALUE;
        }
        
        else{
            c->tag = TAG_SYMBOL;
            
            //            printf("[symbol] ");
        }
    }
    
    //    printf("\n\n");
    
}

logic FillEvalBuffer(s8* buffer,u32* a,EvalChar* evaluation_buffer,u32* k,s8* terminator_array,u32 terminator_count){
    
    auto cur = *a;
    
    u32 evaluation_count = *k;
    
    u32 symbol_len = 0;
    s8 symbol_buffer[128] = {};
    
    logic ret = false;
    
    PGetSymbol(&symbol_buffer[0],buffer,&cur,&symbol_len);
    
    if(symbol_len){
        
        //printf("%s\n",&symbol_buffer[0]);
        
        evaluation_buffer[evaluation_count] =
        {PHashString(&symbol_buffer[0])};
        memcpy(&evaluation_buffer[evaluation_count].string[0],&symbol_buffer[0],strlen(&symbol_buffer[0]));
        
        evaluation_count++;
    }
    
    if(buffer[cur] == '('){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString("("),"("};
        
        evaluation_count++;
    }
    
    if(buffer[cur] == ')'){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString(")"),")"};
        
        evaluation_count++;
        
    }
    
    if(buffer[cur] == '*'){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString("*"),"*"};
        
        evaluation_count++;
        
    }
    
    if(buffer[cur] == '='){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString("="),"="};
        
        evaluation_count++;
    }
    
    if(buffer[cur] == '"'){
        
        for(;;){
            
            s8 t[2] = {buffer[cur],0};
            
            evaluation_buffer[evaluation_count] =
                EvalChar{PHashString(&t[0]),buffer[cur]};
            evaluation_count++;
            
            //printf("%c",buffer[cur]);
            
            cur++;
            
            if(buffer[cur] == '"'){
                
                s8 t[2] = {buffer[cur],0};
                
                evaluation_buffer[evaluation_count] =
                    EvalChar{PHashString(&t[0]),buffer[cur]};
                evaluation_count++;
                
                //printf("%c",buffer[cur]);
                
                break;
            }
        }
    }
    
    
    
    for(u32 j = 0; j < terminator_count;j++){
        
        if(buffer[cur] == terminator_array[j]){
            
            TagEvalBuffer(&evaluation_buffer[0],evaluation_count);
            ret = true;
            break;
        }
        
    }
    
    
    if(buffer[cur] == ';' && !ret){
        evaluation_count = 0;
    }
    
    
    
    *k = evaluation_count;
    *a = cur;
    
    return ret;
}

logic FillEvalBuffer(s8* buffer,u32* a,EvalChar* evaluation_buffer,u32* k,s8 terminator){
    
    return FillEvalBuffer(buffer,a,evaluation_buffer,k,&terminator,1);
}

struct GenericTypeDec{
    s8 type_string[128];
    s8 name_string[128];
    u64 name_hash;
    GLSLType type;
};

struct GenericTypeDef : GenericTypeDec{
    u32 dim_array_count;
    u32 dim_array[8];
};

struct GenericStruct : GenericTypeDec{
    u32 members_count = 0;
    GenericTypeDef members_array[256];
};

void _ainline InternalHandleStructFields(GenericStruct* t,GenericStruct* struct_array,u32* struct_count,EvalChar* membereval_array,u32 membereval_count,u32* cur){
    
    
    auto i = *cur;
    
    auto member = &t->members_array[t->members_count];
    t->members_count++;
    
    
    for(u32 j = 0; j < _arraycount(member->dim_array);j++){
        member->dim_array[j] = 1;
    }
    
    
    for(u32 j = 0; j < membereval_count;j++){
        
        auto x = &membereval_array[j];
        
        if(j == 0){
            
            if(x->tag == TAG_SYMBOL){
                
                member->type = GLSLType_STRUCT;
            }
            
            if(x->tag == TAG_CTYPE){
                member->type = (GLSLType)x->hash;
            }
            
            memcpy(&member->type_string[0],&x->string[0],strlen(&x->string[0]));
            
        }
        
        else if(x->tag == TAG_SYMBOL){
            
            member->name_hash = x->hash;
            
            memcpy(&member->name_string[0],&x->string[0],strlen(&x->string[0]));
            
        }
        
        
        if(PIsStringInt(&x->string[0])){
            
            _kill("too many dims\n",member->dim_array_count >= _arraycount(member->dim_array));
            
            member->dim_array[member->dim_array_count] = atoi(&x->string[0]);
            member->dim_array_count++;
            
            
        }
        
    }
    
    *cur = i;
}

void GenerateGenericStruct(EvalChar* eval_buffer,u32 count,s8* buffer,u32* a,GenericStruct* struct_array,u32* struct_count){
    
    auto s_count = *struct_count;
    
    s8 name_buffer[256] = {};
    
    auto t = &struct_array[s_count];
    s_count++;
    
    //fill name here
    for(u32 i = 0; i < count; i++){
        
        if(eval_buffer[i].tag == TAG_SYMBOL){
            
            memcpy(name_buffer,eval_buffer[i].string,strlen(eval_buffer[i].string));
            
            break;
        }
    }
    
    memcpy(&t->name_string[0],&name_buffer[0],strlen(&name_buffer[0]));
    t->name_hash = PHashString(name_buffer);
    
    t->type = GLSLType_STRUCT;
    memcpy(&t->type_string[0],"struct",strlen("struct"));
    
    s8 scope_buffer[1024 * 4] = {};
    
    PExtractScopeC(&scope_buffer[0],buffer,a);
    
    EvalChar membereval_array[256] = {};
    u32 membereval_count = 0;
    
    for(u32 i = 0;;i++){
        
        PSanitizeStringC(&scope_buffer[0],&i);
        
        auto c = scope_buffer[i];
        s8 terminator_array[] = {';','{'};
        
        if(FillEvalBuffer(scope_buffer,&i,&membereval_array[0],&membereval_count,&terminator_array[0],_arraycount(terminator_array))){
            
            
            if(membereval_count){
                
                InternalHandleStructFields(t,struct_array,struct_count,membereval_array,membereval_count,&i);
                
            }
            
            membereval_count = 0;
        }
        
        
        if(!c){
            break;
        }
    }
    
    *struct_count = s_count;
    
    //MARK: print struct here for debugging
}

enum ParsePath{
    PARSEPATH_UNKNOWN = 0,
    PARSEPATH_VERTLAYOUT,
    PARSEPATH_DESCLAYOUT,
    PARSEPATH_PUSHCONSTLAYOUT,
};

ParsePath _ainline InternalGetParseType(EvalChar* eval_buffer,u32 eval_count){
    
    logic in_args_scope = false;
    
    for(u32 i = 0; i < eval_count; i++){
        
        if(eval_buffer[i].hash == PHashString("in")){
            return PARSEPATH_VERTLAYOUT;
        }
        
        if(eval_buffer[i].tag == TAG_START_ARG){
            
            in_args_scope = true;
        }
        
        if(eval_buffer[i].tag == TAG_END_ARG){
            
            in_args_scope = false;
        }
        
        
        //these have to be in args scope
        
        if(in_args_scope){
            
            if(eval_buffer[i].hash == PHashString("binding") || eval_buffer[i].hash == PHashString("set")){
                return PARSEPATH_DESCLAYOUT;
            }
            
            if(eval_buffer[i].hash == PHashString("push_constant")){
                return PARSEPATH_PUSHCONSTLAYOUT;
            }
            
        }
    }
    
    return PARSEPATH_UNKNOWN;
    
}

void InternalParseVertexLayout(EvalChar* eval_buffer,u32 eval_count,VertexLayoutEx* vertexlayout,VertexLayoutEx* instancelayout){
    
    auto layout = vertexlayout;
    u32 location = (u32)-1;
    
    //check if it is instance rate
    for(u32 i = eval_count; i != (u32)-1; i--){
        
        if(eval_buffer[i].hash == PHashString("INSTRATE")){
            layout = instancelayout;
        }
        
        if(eval_buffer[i].tag == TAG_VALUE){
            location = atoi(eval_buffer[i].string);
        }
    }
    
    _kill("location not found\n",location == (u32)-1);
    
    
    for(u32 i = eval_count; i != (u32)-1; i--){
        
        auto tag = eval_buffer[i].tag;
        
        if(tag == TAG_CTYPE){
            
            VkFormat format = {};
            u32 size = 0;
            
            GetInternalFormatAndSize((GLSLType)PHashString(eval_buffer[i].string),&format,&size);
            
            layout->entry_array[layout->entry_count] = {format,size,location};
            layout->size += size;
            
            
#if _log_string
            
            memcpy(layout->entry_array[layout->entry_count].format_string,eval_buffer[i].string,strlen(eval_buffer[i].string));
            
#endif
            
            
            layout->entry_count ++;
            
            
        }
    }
    
}

struct FormatSize{
    VkFormat format;
    u16 size;//size of single element
    u16 dim_array_count;
    u16 dim_array[8];
    
    
#ifdef _log_string
    
    s8 string[128];
    
#endif
    
};


void GenericStructToFormatSize(GenericStruct* basestruct,GenericStruct* struct_array,u32 struct_count,FormatSize* array,u32* a){
    
    auto count = *a;
    
    for(u32 i = 0; i < basestruct->members_count; i++){
        
        auto member = &basestruct->members_array[i];
        
        
        //recursively find struct
        if(member->type == GLSLType_STRUCT){
            
            GenericStruct* s = 0;
            
            for(u32 j = 0; j < struct_count; j++){
                
                auto k = &struct_array[j];
                
                if(k->name_hash == member->name_hash){
                    
                    s = k;
                }
                
                _kill("struct not found\n",s == 0);
                
                
                FormatSize t_format_array[1024] = {};
                u32 t_format_count = 0;
                
                
                GenericStructToFormatSize(s,struct_array,struct_count,t_format_array,&t_format_count);
                
                //we need to add it as many times array elements of the struct
                for(u32 j = 0; j < t_format_count; j++){
                    
                    array[count] = t_format_array[j];
                    count++;
                }
            }
        }
        
        else{
            
            VkFormat format = {};
            u32 size = 0;
            
            GetInternalFormatAndSize(member->type,&format,&size);
            
            array[count].format = format;
            array[count].size = size;
            array[count].dim_array_count = member->dim_array_count;
            
            memcpy(array[count].dim_array,member->dim_array,sizeof(member->dim_array));
            
#ifdef _log_string
            
            memcpy(array[count].string,member->type_string,strlen(member->type_string));
            
#endif
            
            count++;
        }
        
    }
    
    *a = count;
}


void InternalHandlePushConst(s8* buffer,u32* cur,EvalChar* eval_buffer,u32 eval_count,GenericStruct* struct_array,u32 struct_count,PushConstLayout* pushconstlayout){
    
    GenericStruct genericstruct = {};
    u32 g_count = 0;
    
    
    GenerateGenericStruct(&eval_buffer[0],eval_count,buffer,cur,&genericstruct,&g_count);
    
    _kill("cannot be more than 1\n",g_count > 1);
    
#if 0
    
    {
        
        printf("---PUSHCONST START---\n");
        
        for(u32 i = 0; i < genericstruct.members_count; i++){
            
            auto member = &genericstruct.members_array[i];
            
            printf("%s: type %s\n",member->name_string,member->type_string);
        }
        
        printf("---PUSHCONST END---\n");
    }
    
#endif
    
    FormatSize formatsize_array[1024] = {};
    u32 formatsize_count = 0;
    
    GenericStructToFormatSize(&genericstruct,struct_array,struct_count,formatsize_array,&formatsize_count);
    
    _kill("too many entries\n",formatsize_count > _arraycount(formatsize_array));
    
    //TODO: Make this follow std430
    
    
    
    //We use this to make sure all pushconsts follow either std140 or 430
    u32 size_tracker = 0;
    
    for(u32 i = 0; i < formatsize_count; i++){
        
        auto f = &formatsize_array[i];
        
        u32 actual_count = 1;
        
        if(f->dim_array_count){
            
            for(u32 j = 0; j < f->dim_array_count; j++){
                
                actual_count *= f->dim_array[j];
            }
        }
        
        for(u32 j = 0; j < actual_count; j++){
            
            pushconstlayout->entry_array[pushconstlayout->entry_count].format = f->format;
            
            pushconstlayout->size += f->size;
            
            
            //MARK: this is a hack for now
            pushconstlayout->size = _align16(pushconstlayout->size);
            
            
#if _log_string
            
            memcpy(pushconstlayout->entry_array[pushconstlayout->entry_count].string,f->string,sizeof(f->string));
            
#endif
            
            pushconstlayout->entry_count++;
            
        }
        
    }
}

void InternalDebugPrintPushConstLayout(PushConstLayout* pushconstlayout){
    
    
#if _log_string
    
    if(pushconstlayout->size){
        
        printf("\n\n");
        printf("PUSHCONST START (%d)\n",pushconstlayout->size);
        
        for(u32 i = 0; i < pushconstlayout->entry_count; i++){
            
            printf("%d: %s\n",i,pushconstlayout->entry_array[i].string);
        }
        
        printf("PUSHCONST END\n");
        printf("\n\n");
    }
    
#endif
    
    
}

void InternalVkDescriptorTypeToString(VkDescriptorType type,s8* buffer,u32 size){
    
    
    switch(type){
        
        case VK_DESCRIPTOR_TYPE_SAMPLER:{
            
            auto string = "VK_DESCRIPTOR_TYPE_SAMPLER";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
        }break;
        
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:{
            
            auto string = "VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
        }break;
        
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:{
            
            auto string = "VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
            
            
        }break;
        
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:{
            
            auto string = "VK_DESCRIPTOR_TYPE_STORAGE_IMAGE";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
            
        }break;
        
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:{
            
            auto string = "VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
        }break;
        
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:{
            
            auto string = "VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
        }break;
        
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:{
            
            auto string = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
        }break;
        
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:{
            
            auto string = "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
        }break;
        
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:{
            
            auto string = "VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
        }break;
        
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:{
            
            auto string = "VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
        }break;
        
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:{
            
            auto string = "VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT";
            u32 len = strlen( string);
            
            _kill("",len > size);
            
            memcpy(buffer, string,len);
            
        }break;
    }
    
    
    
    
}

void InternalDebugPrintDescLayout(DescLayout* desclayout){
    
    if(desclayout->entry_count){
        
        printf("\n\n");
        printf("DESC START (%d)\n",desclayout->entry_count);
        
        for(u32 i = 0; i < desclayout->entry_count; i++){
            
            auto e = &desclayout->entry_array[i];
            
            s8 buffer[256] = {};
            
            
            InternalVkDescriptorTypeToString(e->type,&buffer[0],_arraycount(buffer));
            
            printf("SET %d BIND %d : %s [%d]\n",e->set,e->bind,&buffer[0],e->total_count);
            
        }
        
        printf("DESC END\n");
        printf("\n\n");
    }
    
}

void InternalDebugPrintVertexLayout(VertexLayoutEx* vertexlayout){
    
#if _log_string
    
    
    if(vertexlayout->size){
        
        printf("\n\n");
        printf("VERTEX START (%d)\n",vertexlayout->size);
        
        for(u32 i = 0; i < vertexlayout->entry_count; i++){
            
            auto e = &vertexlayout->entry_array[i];
            
            printf("%d: %s %d\n",e->index,e->format_string,e->size);
            
        }
        
        
        printf("VERTEX END\n");
        printf("\n\n");
        
    }
    
#endif
    
    
}


void InternalDebugPrintInstanceLayout(VertexLayoutEx* vertexlayout){
    
#if _log_string
    
    
    if(vertexlayout->size){
        
        printf("\n\n");
        printf("INSTANCE START (%d)\n",vertexlayout->size);
        
        for(u32 i = 0; i < vertexlayout->entry_count; i++){
            
            auto e = &vertexlayout->entry_array[i];
            
            printf("%d: %s %d\n",e->index,e->format_string,e->size);
            
        }
        
        
        printf("INSTANCE END\n");
        printf("\n\n");
        
    }
    
#endif
    
}

void InternalDebugPrintVertexLayout(DescLayout* desclayout){}

void GetAttribDominantAndSecondary(GLSLType* type_array,u32 type_count,GLSLType* dominant,GLSLType* secondary){
    
    for(u32 i = 0; i < type_count; i++){
        
        auto attrib = type_array[i];
        
        if(attrib == GLSLType_BUFFER || attrib == GLSLType_SAMPLER ||
           attrib == GLSLType_SAMPLER1D ||
           attrib == GLSLType_SAMPLER2D || attrib == GLSLType_SAMPLER3D ||
           attrib == GLSLType_SAMPLERCUBE || attrib == GLSLType_SAMPLER2DRECT ||
           attrib == GLSLType_SAMPLERBUFFER || attrib == GLSLType_SAMPLER2DMS ||
           attrib == GLSLType_SAMPLER1DSHADOW || attrib == GLSLType_SAMPLER2DSHADOW ||
           attrib == GLSLType_SAMPLER3DSHADOW || attrib == GLSLType_SAMPLERCUBESHADOW ||
           attrib == GLSLType_SAMPLER2DRECTSHADOW || attrib == GLSLType_TEXTURE1D ||
           attrib == GLSLType_TEXTURE2D || attrib == GLSLType_TEXTURE3D ||
           attrib == GLSLType_TEXTURECUBE || attrib == GLSLType_TEXTURE2DRECT ||
           attrib == GLSLType_TEXTUREBUFFER || attrib == GLSLType_TEXTURE2DMS ||
           attrib == GLSLType_IMAGE1D || attrib == GLSLType_IMAGE2D ||
           attrib == GLSLType_IMAGE3D || attrib == GLSLType_IMAGECUBE ||
           attrib == GLSLType_IMAGE2DRECT || attrib == GLSLType_IMAGEBUFFER ||
           attrib == GLSLType_IMAGE2DMS || attrib == GLSLType_UNIFORM){
            
            if((*dominant) == GLSLType_UNIFORM){
                *secondary = GLSLType_UNIFORM;
            }
            
            *dominant = attrib;
        }
    }
    
    _kill("no dominant attrib found\n",!(*dominant));
}

void InternalHandleDesclayout(s8* buffer,u32* cur,EvalChar* eval_buffer,u32 eval_count,GenericStruct* struct_array,u32 struct_count,DescLayout* desclayout){
    
    /*
    There are two kinds of descset patterns
    
    layout (set = 0,binding = 0) uniform UBO DYNBUFFER{
    mat4 world;
    mat4 bone_array[_max_bones];
    uint texture_id[16];
    
}ubo;

layout (set = 1,binding = 0) uniform sampler2D samplerColor;


When parsing this, we don't have to specify every element in a block, just whether it is uniform etc

TODO: handle std430 std140
*/
    
    AttribEx attribex = AttribEx_NONE;
    GLSLType type_array[4] = {};
    u32 type_count = 0;
    
    enum ParseMode{
        PARSE_BINDING = 0,
        PARSE_SET,//specifying the set is optional (assumed 0)
    };
    
    
    ParseMode mode = PARSE_BINDING;
    
    u32 set = 0;
    u32 bind = 0;
    u32 total_count = 1;
    
    logic in_args_scope = false;
    
    for(u32 i = 0; i < eval_count; i++){
        
        auto c = &eval_buffer[i];
        
        if(IsAttrib(c->hash)){
            type_array[type_count] = (GLSLType)c->hash;
            type_count++;
        }
        
        if(IsAttribEx(c->hash)){
            
            attribex = (AttribEx)c->hash;
        }
        
        
        if(c->tag == TAG_START_ARG){
            in_args_scope = true;
        }
        
        if(c->tag == TAG_END_ARG){
            in_args_scope = false;
        }
        
        if(in_args_scope){
            
            if(c->hash == PHashString("set")){
                
                mode = PARSE_SET;
                
            }
            
            if(c->hash == PHashString("binding")){
                mode = PARSE_BINDING;
            }
            
            
            if(c->tag == TAG_VALUE){
                
                if(mode == PARSE_BINDING){
                    bind = atoi(c->string);
                }
                
                else{
                    set = atoi(c->string);
                }
            }
            
            
        }
        
        else if(c->tag == TAG_VALUE){
            total_count *= atoi(c->string);
        }
    }
    
    
    GLSLType dominant_attrib = (GLSLType)0;
    GLSLType secondary_attrib = (GLSLType)0;
    
    GetAttribDominantAndSecondary(type_array,type_count,&dominant_attrib,&secondary_attrib);
    
    auto desctype = GetInternalDescAttrib(dominant_attrib,
                                          secondary_attrib,attribex);
    
    desclayout->entry_array[desclayout->entry_count] = {
        desctype,set,bind,total_count
    };
    desclayout->entry_count++;
    
}



void InternalHandleLayout(s8* buffer,u32* cur,ShaderType type,EvalChar* eval_buffer,u32 eval_count,GenericStruct* struct_array,u32 struct_count,VertexLayoutEx* vertexlayout,VertexLayoutEx* instancelayout,PushConstLayout* pushconstlayout,DescLayout* desclayout){
    
    switch(InternalGetParseType(eval_buffer,eval_count)){
        
        case PARSEPATH_VERTLAYOUT:{
            
            if(type == ShaderType_VERTEX){
                
                InternalParseVertexLayout(eval_buffer,eval_count,vertexlayout,instancelayout);
            }
            
            
            
        }break;
        
        case PARSEPATH_DESCLAYOUT:{
            
            InternalHandleDesclayout(buffer,cur,eval_buffer,eval_count,struct_array,struct_count,desclayout);
            
            
        }break;
        
        case PARSEPATH_PUSHCONSTLAYOUT:{
            
            //TODO: check for specified layout (std140 or std430. default should be std430)
            
            InternalHandlePushConst(buffer,cur,eval_buffer,eval_count,struct_array,struct_count,pushconstlayout);
            
        }break;
        
    }
}

void InternalParseSource(ShaderType type,s8* buffer,u32 size,GenericStruct* struct_array,u32* struct_count,VertexLayoutEx* vertexlayout,VertexLayoutEx* instancelayout,PushConstLayout* pushconstlayout,DescLayout* desclayout){
    
    u32 evaluation_count = 0;
    EvalChar evaluation_buffer[256] = {};
    
    u32 cur = 0;
    
    for(;;){
        
        PSanitizeStringC(buffer,&cur);
        
        s8 terminator_array[] = {'{',';'};
        
        if(FillEvalBuffer(buffer,&cur,&evaluation_buffer[0],&evaluation_count,terminator_array,_arraycount(terminator_array))){
            
#if 0
            
            for(u32 i = 0; i < evaluation_count; i++){
                
                printf("%s ",&evaluation_buffer[i].string[0]);
            }
            
            printf("\n");
            
#endif
            
            
            if(IsStruct(&evaluation_buffer[0],evaluation_count)){
                
                GenerateGenericStruct(&evaluation_buffer[0],evaluation_count,buffer,&cur,struct_array,struct_count);
                
            }
            
            if(IsLayout(&evaluation_buffer[0],evaluation_count)){
                InternalHandleLayout(buffer,&cur,type,&evaluation_buffer[0],evaluation_count,struct_array,*struct_count,vertexlayout,instancelayout,pushconstlayout,desclayout);
                
            }
            
            PSkipBracketBlock(buffer,&cur);
            evaluation_count = 0;
            
        }
        
        cur++;
        
        if(cur >= size){
            break;
        }
    }
    
}


void WriteSPXFile(ShaderType type,s8* outfile_string,PushConstLayout playout,
                  VertexLayout vlayout,
                  VertexLayout instlayout,
                  DescLayout dlayout){
    
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
        
        auto layout = LayoutType_DESC;
        
        FWrite(outfile,&layout,sizeof(layout));
        FWrite(outfile,&dlayout.entry_count,sizeof(dlayout.entry_count));
        
        for(u32 i = 0; i < dlayout.entry_count; i++){
            
            auto entry = dlayout.entry_array[i];
            
            FWrite(outfile,&entry,sizeof(entry));
            
        }
        
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




/*

STD140

The base alignment of the type of an OpTypeStruct member of is defined recursively as follows:

    A scalar of size N has a base alignment of N.
    
    A two-component vector, with components of size N, has a base alignment of 2 N.
    
    A three- or four-component vector, with components of size N, has a base alignment of 4 N.
    
    An array has a base alignment equal to the base alignment of its element type, rounded up to a multiple of 16.
    
    A structure has a base alignment equal to the largest base alignment of any of its members, rounded up to a multiple of 16.
    
    A row-major matrix of C columns has a base alignment equal to the base alignment of a vector of C matrix components.
    
    A column-major matrix has a base alignment equal to the base alignment of the matrix column type.
    
A member is defined to improperly straddle if either of the following are true:

    It is a vector with total size less than or equal to 16 bytes, and has Offset decorations placing its first byte at F and its last byte at L, where floor(F / 16) != floor(L / 16).
    
    It is a vector with total size greater than 16 bytes and has its Offset decorations placing its first byte at a non-integer multiple of 16.
    
Every member of an OpTypeStruct with storage class of Uniform and a decoration of Block (uniform buffers) must be laid out according to the following rules:

    The Offset decoration of a scalar, an array, a structure, or a matrix must be a multiple of its base alignment.
    
    The Offset decoration of a vector must be an integer multiple of the base alignment of its scalar component type, and must not improperly straddle, as defined above.
    
    Any ArrayStride or MatrixStride decoration must be an integer multiple of the base alignment of the array or matrix from above.
    
    The Offset decoration of a member must not place it between the end of a structure or an array and the next multiple of the base alignment of that structure or array.
    
    The numeric order of Offset decorations need not follow member declaration order.
    
    
    STD430
    
    Member variables of an OpTypeStruct with a storage class of PushConstant (push constants), or a storage class of Uniform with a decoration of BufferBlock (storage buffers) , or a storage class of StorageBuffer with a decoration of Block must be laid out as above, except for array and structure base alignment which do not need to be rounded up to a multiple of 16.
    
*/