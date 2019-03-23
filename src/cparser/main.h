#pragma once
#include "mode.h"
#include "ttype.h"

#include "ttimer.h"

#include "aallocator.h"

#include "stdio.h"
#include "stdlib.h"

#include "ffileio.h"
#include "pparse.h"
#include "ctype.h"

#define _testing 0

#if _testing

#define FWrite(file,buffer,len) printf("%s",(s8*)(buffer));

#endif

//TODO:
//Remove ref_metadatacomp_index

enum CType{
    
    CType_UNKNOWN = 0,
    
    CType_U8 = PHashString("u8"),
    CType_U16 = PHashString("u16"),
    CType_U32 = PHashString("u32"),
    CType_U64 = PHashString("u64"),
    
    CType_S8 = PHashString("s8"),
    CType_S16 = PHashString("s16"),
    CType_S32 = PHashString("s32"),
    CType_S64 = PHashString("s64"),
    
    CType_LOGIC = PHashString("b32"),
    
    CType_F32 = PHashString("f32"),
    CType_F64 = PHashString("f64"),
    
    CType_PTRSIZE = PHashString("ptrsize"),
    
    CType_VOID = PHashString("void"),
    
    CType_STRUCT = PHashString("struct"),
    CType_ENUM = PHashString("enum"),
    CType_UNION = PHashString("union"),//Do we support unions?
};

b32 IsIntType(u32 type){
    
    return type == CType_U8 ||
        type == CType_U16 ||
        type == CType_U32 ||
        type == CType_U64 ||
        type == CType_S8||
        type == CType_S16||
        type == CType_S32||
        type == CType_S64;
}

b32 IsFloatType(u32 type){
    return type == CType_F32 ||
        type == CType_F64;
}

b32 IsCType(u64 hash){
    
    CType array[] = {
        
        CType_U8,
        CType_U16,
        CType_U32,
        CType_U64,
        
        CType_S8 ,
        CType_S16,
        CType_S32,
        CType_S64,
        
        CType_LOGIC,
        
        CType_F32,
        CType_F64,
        
        CType_PTRSIZE,
        
        CType_VOID,
        
        CType_STRUCT,
        CType_ENUM,
        CType_UNION,
    };
    
    for(u32 i = 0; i < _arraycount(array); i++){
        
        if(hash == array[i]){
            return true;
        }
    }
    
    return false;
}

enum ParserKeyWord{
    PARSERKEYWORD_REFL = PHashString("REFL"),
    PARSERKEYWORD_COMPONENT = PHashString("REFLCOMPONENT"),
};

struct GenericTypeDec{
    
    s8 type_string[128];
    s8 name_string[128];
    u64 name_hash;
    CType type;
};

struct GenericTypeDef : GenericTypeDec{
    
    u8 indir_count;
    u8 dim_count;
    u8 default_count;//MARK: -1 is {}
    
    
    u16 dim_array[8];
};

struct GenericEnumEntry{
    u64 name_hash;
    s8 name_string[128];
};

struct GenericStruct : GenericTypeDec{
    ParserKeyWord pkey;
    u32 members_count = 0;
    GenericTypeDef members_array[256];
};


struct GenericEnum{
    u64 name_hash;
    s8 name_string[128];
    
    GenericEnumEntry members_array[256];
    u32 members_count;
};

struct GenericFunction{
    u64 name_hash;
    s8 name_string[128];
    
    GenericTypeDef ret;
    
    GenericTypeDef args_array[256];
    u32 args_count;
};



b32 IsParserKeyword(u64 hash){
    
    ParserKeyWord array[] = {
        PARSERKEYWORD_REFL,
        PARSERKEYWORD_COMPONENT,
    };
    
    for(u32 i = 0; i < _arraycount(array); i++){
        
        if(hash == array[i]){
            return true;
        }
    }
    
    return false;
}

enum REFLSTRUCTTYPE{
    REFLSTRUCTTYPE_NONE = 0,
    REFLSTRUCTTYPE_NAMED_STRUCT = 1,
    REFLSTRUCTTYPE_TYPEDEF_STRUCT = 2,
};

REFLSTRUCTTYPE IsReflStruct(EvalChar* eval_buffer,u32 count){
    
    b32 is_struct = false;
    b32 has_keyword = false;
    b32 is_typedef = false;
    
    for(u32 i = 0; i < count; i++){
        
        if(!is_struct){
            is_struct = eval_buffer[i].tag == TAG_STRUCT;
        }
        
        if(!has_keyword){
            has_keyword = eval_buffer[i].tag == TAG_KEY;
        }
        
        if(!is_typedef){
            
            is_typedef = eval_buffer[i].hash == PHashString("typedef");
        }
        
        if(has_keyword && is_struct){
            break;
        }
    }
    
    if(is_struct && has_keyword && is_typedef){
        return REFLSTRUCTTYPE_TYPEDEF_STRUCT;
    }
    
    if(is_struct && has_keyword){
        return REFLSTRUCTTYPE_NAMED_STRUCT;
    }
    
    return REFLSTRUCTTYPE_NONE;
}

b32 IsReflEnum(EvalChar* eval_buffer,u32 count){
    
    b32 is_enum = eval_buffer[0].tag == TAG_ENUM;
    
    b32 has_keyword = false;
    
    for(u32 i = 1; i < count; i++){
        
        has_keyword = eval_buffer[i].tag == TAG_KEY;
        
        if(has_keyword){
            break;
        }
    }
    
    return is_enum && has_keyword;
}

b32 IsReflFunc(EvalChar* eval_buffer,u32 count){
    
    b32 has_return_type = eval_buffer[0].tag == TAG_CTYPE;
    
    b32 has_arg_brackets = false;
    
    b32 has_key = false;
    
    for(u32 i = 1; i < count - 1; i++){
        
        auto a = eval_buffer[i];
        auto b = eval_buffer[i + 1];
        
        if(a.tag == TAG_KEY){
            has_key = true;
        }
        
        if(a.tag == TAG_SYMBOL && b.tag == TAG_START_ARG){
            
            for(u32 j = i + 2; j < count; j++){
                
                auto k = eval_buffer[j];
                
                if(k.tag == TAG_END_ARG){
                    has_arg_brackets = true;
                    break;
                }
            }
            
            if(has_arg_brackets){
                break;
            }
        }
    }
    
    return has_return_type && has_arg_brackets && has_key;
}

void InternalBufferGetString(s8* default_string,EvalChar* membereval_array,u32 membereval_count,u32* j){
    
    auto string = default_string;
    
    u32 i = (*j) + 1;
    
    for(;;i++){
        
        auto e = &membereval_array[i];
        
        if(e->tag == TAG_DOUBLE_QUOTE){
            break;
        }
        
        *string = e->string[0];
        string++;
    }
    
    *j = i;
}

void DebugPrintGenericEnum(GenericEnum* e){
    
    printf("Parsed enum %s that has %d members\n",e->name_string,e->members_count);
    
    for(u32 i = 0; i < e->members_count;i++){
        
        printf("%s\n",e->members_array[i].name_string);
    }
}

void DebugPrintGenericStruct(GenericStruct* s){
    
    printf("Parsed struct %s that has %d members\n",s->name_string,s->members_count);
    
    for(u32 i = 0; i < s->members_count; i++){
        
        auto f = &s->members_array[i];
        
        printf("%s",f->type_string);
        
        for(u32 k = 0;k < f->indir_count;k++){
            printf("*");
        }
        
        printf(" ");
        
        printf("%s",f->name_string);
        
        for(u32 k = 0; k < f->dim_count;k++){
            
            printf("[%d]",f->dim_array[k]);
        }
        
        
        
        printf("\n");
    }
}


void GetArgsData(s8** argv,u32 argc,s8*** source_array,u32* source_count,s8** component_src,s8** meta_src,s8** component_h,s8** meta_h){
    
    u32 i = 0;
    
    for(; i < argc; i++){
        
        auto hash = PHashString(argv[i]);
        
        if(hash == PHashString("-component-source") || hash == PHashString("-meta-source") || hash == PHashString("-component-header") || hash == PHashString("-meta-header")){
            break;
        }
    }
    
    *source_count = i;
    
    if((*source_count)){
        *source_array = &argv[0];
    }
    
#if 0
    
    for(u32 k = i; k < argc; k++){
        printf("%s\n",argv[k]);
    }
    
#endif
    
    for(; i < argc; i++){
        
        if(PHashString(argv[i]) == PHashString("-component-source")){
            i++;
            *component_src = argv[i];
        }
        
        if(PHashString(argv[i]) == PHashString("-meta-source")){
            i++;
            *meta_src = argv[i];
        }
        
        
        if(PHashString(argv[i]) == PHashString("-meta-header")){
            i++;
            *meta_h = argv[i];
        }
        
        
        if(PHashString(argv[i]) == PHashString("-component-header")){
            i++;
            *component_h = argv[i];
        }
        
    }
    
    
    
}

const s8* InternalGetMainFile(const s8* string){
    
    auto len = strlen(string);
    
    for(u32 i = len - 1; i != (u32)-1; i--){
        
        if(string[i] == '/' || string[i] == '\\'){
            return &string[i + 1];
        }
    }
    
    return 0;
}


void InternalGetInheritancdTypeString(s8* dst_string,s8* src_string){
    
    
    u32 len = strlen(src_string);
    
    
    for(u32 i = 0; i <  len; i++){
        
        dst_string[i] = src_string[i];
        
        
        
        if(src_string[i] == '_' && src_string[i + 1] == '_'){
            
            dst_string[i] = ':';
            
            i++;
            
            dst_string[i] = ':';
        }
    }
    
}

void InternalWriteStructs(FileHandle src_file,FileHandle header_file,GenericStruct* struct_array,u32 struct_count){
    
    //structs
    
    for(u32 i = 0; i < struct_count; i++){
        
        auto s = &struct_array[i];
        
        
        if(header_file){
            
            s8 buffer[256] = {};
            
            sprintf(buffer,"\n\nextern MetaStructEntry %s_META_STRUCT[%d];\n",s->name_string,s->members_count);
            
            FWrite(header_file,(void*)buffer,strlen(buffer));
            
        }
        
        
        {
            
            s8 buffer[256] = {};
            
            sprintf(buffer,"\n\nMetaStructEntry %s_META_STRUCT[] = {\n",s->name_string);
            
            FWrite(src_file,(void*)buffer,strlen(buffer));
            
        }
        
        s8 struct_name_buffer[256] = {};
        
        InternalGetInheritancdTypeString(struct_name_buffer,s->name_string);
        
        
        
        {
            
            for(u32 j = 0; j < s->members_count;j++){
                
                
                auto m = &s->members_array[j];
                
                s8 buffer[256] = {};
                
                u32 element_count = 1;
                
                for(u32 i = 0; i < m->dim_count; i++){
                    element_count *= m->dim_array[i];
                }
                
                
                
                sprintf(buffer,"{(u32)%d,(u32)%d,\"%s\",\"%s\",(u32)sizeof(%s::%s),(u32)((u64)((&((%s*)0)->%s))),%d,(u32)-1},\n"
                        ,(u32)PHashString(m->type_string),(u32)m->name_hash,m->type_string
                        ,m->name_string,
                        
                        struct_name_buffer,m->name_string,
                        
                        struct_name_buffer,m->name_string
                        
                        ,element_count);
                
                FWrite(src_file,(void*)buffer,strlen(buffer));
            }
            
            
            auto end_struct = "};";
            FWrite(src_file,(void*)end_struct,strlen(end_struct));
            
        }
    }
    
    
    {
        
        if(header_file){
            
            auto count = struct_count;
            
            if(!count){
                count = 1;
            }
            
            s8 buffer[256] = {};
            
            sprintf(buffer,"\n\n\nextern MetaDataStructEntry META_STRUCT_ARRAY[%d];\n",count);
            
            FWrite(header_file,(void*)buffer,strlen(buffer));
            
        }
        
        auto metacomp_string = "\n\n\nMetaDataStructEntry META_STRUCT_ARRAY[] = {\n";
        
        FWrite(src_file,(void*)metacomp_string,strlen(metacomp_string));
        
        if(struct_count){
            
            for(u32 i = 0; i < struct_count; i++){
                
                auto s = &struct_array[i];
                
                s8 outbuffer[2048] = {};
                
                s8 struct_name_buffer[256] = {};
                
                InternalGetInheritancdTypeString(struct_name_buffer,s->name_string);
                
                sprintf(outbuffer,
                        "{sizeof(%s),\"%s\",(u32)%d,&%s_META_STRUCT[0],_arraycount(%s_META_STRUCT)},\n",
                        struct_name_buffer,s->name_string,
                        (u32)s->name_hash,s->name_string,
                        s->name_string);
                
                FWrite(src_file,(void*)outbuffer,strlen(outbuffer));
            }
            
        }
        
        else{
            
            auto buffer = "{},\n";
            FWrite(src_file,(void*)buffer,strlen(buffer));
        }
        
        metacomp_string = "\n};\n";
        
        FWrite(src_file,(void*)metacomp_string,strlen(metacomp_string));
    }
    
}

void InternalMakeParentString(s8* dst_name,const s8* src_name){
    
    u32 len = strlen(src_name);
    
    memcpy(dst_name,src_name,len);
    
    for(u32 i = 0; i < len; i++){
        
        if(dst_name[i] == '_' && dst_name[i + 1] == '_'){
            
            dst_name[i] = ':';
            dst_name[i + 1] = ':';
        }
    }
    
}


void InternalWriteEnums(FileHandle src_file,FileHandle header_file,GenericEnum* enum_array,u32 enum_count){
    
    //enums
    
    {
        
        
        for(u32 i = 0; i < enum_count;i++){
            
            auto e = &enum_array[i];
            
            s8 converted_buffer[256] = {};
            
            InternalMakeParentString(converted_buffer,e->name_string);
            
            
            if(header_file){
                
                s8 buffer[256] = {};
                
                sprintf(buffer,"\n\nextern MetaEnumEntry %s_META_ENUM[%d];\n",e->name_string,e->members_count);
                
                FWrite(header_file,(void*)buffer,strlen(buffer));
                
            }
            
            
            
            {
                s8 buffer[256] = {};
                
                sprintf(buffer,"\n\nMetaEnumEntry %s_META_ENUM[] = {\n",e->name_string);
                
                FWrite(src_file,(void*)buffer,strlen(buffer));
            }
            
            
            for(u32 j = 0; j < e->members_count;j++){
                
                auto m = &e->members_array[j];
                
                {
                    s8 buffer[256] = {};
                    
                    
                    
                    sprintf(buffer,"{%d,\"%s\",(u64)%s::%s},\n",
                            (u32)m->name_hash,m->name_string,converted_buffer,m->name_string);
                    
                    FWrite(src_file,(void*)buffer,strlen(buffer));
                }
            }
            
            auto end_struct = "};";
            FWrite(src_file,(void*)end_struct,strlen(end_struct));
        }
        
        {
            
            if(header_file){
                
                auto count = enum_count;
                
                if(!count){
                    count = 1;
                }
                
                s8 buffer[256] = {};
                
                sprintf(buffer,"\n\n\nextern MetaEnumData META_ENUM_ARRAY[%d];\n",count);
                
                FWrite(header_file,(void*)buffer,strlen(buffer));
                
            }
            
            
            auto metacomp_string = "\n\n\nMetaEnumData META_ENUM_ARRAY[] = {\n";
            
            FWrite(src_file,(void*)metacomp_string,strlen(metacomp_string));
            
            if(enum_count){
                
                for(u32 i = 0; i < enum_count;i++){
                    
                    auto e = &enum_array[i];
                    
                    s8 converted_buffer[256] = {};
                    
                    InternalMakeParentString(converted_buffer,e->name_string);
                    
                    {
                        s8 buffer[256] = {};
                        
                        sprintf(buffer,"{%d,\"%s\",sizeof(%s::%s),_arraycount(%s_META_ENUM),&%s_META_ENUM[0]},\n",
                                (u32)e->name_hash,e->name_string,converted_buffer,e->members_array[0].name_string,e->name_string,e->name_string);
                        
                        FWrite(src_file,(void*)buffer,strlen(buffer));
                    }
                    
                }
                
            }
            
            else{
                
                auto buffer = "{},\n";
                FWrite(src_file,(void*)buffer,strlen(buffer));
                
            }
            
            
            
            auto end_struct = "};";
            FWrite(src_file,(void*)end_struct,strlen(end_struct));
            
        }
    }
    
}

void ConstructTypeStringSize(s8* dst_string,GenericTypeDef* type){
    
    
    if(type->type == CType_VOID && !type->indir_count){
        dst_string[0] = '0';
        dst_string[1] = 0;
        return;
    }
    
    s8 type_buffer[256] = {};
    u32 len = strlen(type->type_string);
    
    memcpy(type_buffer,type->type_string,len);
    
    for(u32 i = 0; i < type->indir_count; i++){
        type_buffer[len + i] = '*';
    }
    
    sprintf(dst_string,"(u32)sizeof(%s)",type_buffer);
    
}

void InternalWriteFunctions(FileHandle src_file,FileHandle header_file,GenericFunction* function_array,u32 function_count){
    
    if(header_file){
        
        auto count = function_count;
        
        if(!count){
            count = 1;
        }
        
        s8 buffer[256] = {};
        
        sprintf(buffer,"\n\nextern MetaFunctionData META_FUNCTION_ARRAY[%d];\n",count);
        
        FWrite(header_file,(void*)buffer,strlen(buffer));
        
    }
    
    auto start_string = "\n\nMetaFunctionData META_FUNCTION_ARRAY[] = {\n";
    
    FWrite(src_file,(void*)start_string,strlen(start_string));
    
    if(!function_count){
        
        auto empty_block = "{}\n";
        
        FWrite(src_file,(void*)empty_block,strlen(empty_block));
    }
    
    
    {
        
        for(u32 i = 0; i < function_count;i++){
            
            auto f = &function_array[i];
            
            {
                s8 buffer[256] = {};
                s8 typesize_buffer[256] = {};
                
                ConstructTypeStringSize(typesize_buffer,&f->ret);
                
                sprintf(buffer,
                        "{(void*)%s,\"%s\",%d,{\"%s\",\"%s\",%d,%s,%d,%d},{",
                        f->name_string,
                        f->name_string,
                        (u32)f->name_hash,
                        
                        
                        f->ret.type_string,
                        f->ret.name_string,
                        (u32)f->ret.name_hash,
                        
                        typesize_buffer,
                        
                        IsIntType(PHashString(f->ret.type_string)),
                        f->ret.indir_count
                        );
                
                FWrite(src_file,(void*)buffer,strlen(buffer));
                
            }
            
            for(u32 j = 0; j < f->args_count; j++){
                
                auto a = &f->args_array[j];
                
                s8 buffer[256] = {};
                s8 typesize_buffer[256] = {};
                
                ConstructTypeStringSize(typesize_buffer,a);
                
                sprintf(buffer,
                        "{\"%s\",\"%s\",%d,%s,%d,%d},",
                        a->type_string,
                        a->name_string,
                        (u32)a->name_hash,
                        
                        typesize_buffer,IsIntType(PHashString(a->type_string)),
                        a->indir_count);
                
                FWrite(src_file,(void*)buffer,strlen(buffer));
            }
            
            {
                s8 buffer[256] = {};
                
                sprintf(buffer,"},%d",f->args_count);
                
                FWrite(src_file,(void*)buffer,strlen(buffer));
            }
            
            auto end_struct = "},\n";
            FWrite(src_file,(void*)end_struct,strlen(end_struct));
            
        }
    }
    
    auto end_string = "};";
    
    FWrite(src_file,(void*)end_string,strlen(end_string));
    
    
}


void WriteMetaFile(const s8* src_file_string,const s8* header_file_string,GenericStruct* struct_array,u32 struct_count,GenericEnum* enum_array,u32 enum_count,GenericFunction* function_array,u32 function_count){
    
#if !(_testing)
    
    auto src_file = FOpenFile(src_file_string,F_FLAG_READWRITE |
                              F_FLAG_TRUNCATE | F_FLAG_CREATE);
    
    FileHandle header_file = 0;
    
    if(header_file_string){
        header_file = FOpenFile(header_file_string,F_FLAG_READWRITE |
                                F_FLAG_TRUNCATE | F_FLAG_CREATE);
    }
    
#endif
    
    {
        
        auto header = R"FOO(
        /*This file is generated by the preprocessor*/ 
        #pragma once
        
        
        #include"pparse.h"
        #include "aassetmanager.h"
        
        struct MetaStructEntry{
        u32 type_hash;
        u32 name_hash;
        s8 type_string[128];
        s8 name_string[128];
        u32 size;
        u32 offset;
        u32 arraycount;
        // for referencing other components to access.
        u32 ref_metadatacomp_index; 
        };
        
        struct MetaDataStructEntry{
        u32 size;
        s8 name_string[128];
        u32 name_hash;
        MetaStructEntry* member_array;
        u32 member_count;
        };
        
        struct MetaEnumEntry{
        
        u64 name_hash;
        s8 name_string[128];
        u64 value;
        
        };
        
        struct MetaEnumData{
        
        u64 name_hash;
        s8 name_string[128];
        
        u32 size;
        u32 entry_count;
        MetaEnumEntry* entry_array;
        };
        
        struct MetaTypeVar{
        
        s8 type_string[128];
        s8 name_string[128];
        u64 name_hash;
        
        u32 size;
        u16 is_int;
        u16 indir_count;
        };
        
        struct MetaFunctionData{
        
        void* function_call;
        
        s8 name_string[128];
        u64 name_hash;
        
        MetaTypeVar ret;
        
        MetaTypeVar args_array[128];
        u32 args_count;
        };
        
        enum CType{
        CType_U8 = PHashString("u8"),
        CType_U16 = PHashString("u16"),
        CType_U32 = PHashString("u32"),
        CType_U64 = PHashString("u64"),
        CType_S8 = PHashString("s8"),
        CType_S16 = PHashString("s16"),
        CType_S32 = PHashString("s32"),
        CType_S64 = PHashString("s64"),
        CType_LOGIC = PHashString("b32"),
        CType_F32 = PHashString("f32"),
        CType_F64 = PHashString("f64"),
        CType_PTRSIZE = PHashString("ptrsize"),
        CType_VOID = PHashString("void"),
        CType_STRUCT,
        };
        
        )FOO";
        
        if(header_file){
            
            FWrite(header_file,(void*)header,strlen(header));
            
            s8 buffer[1024] = {};
            
            sprintf(buffer,
                    R"FOO(
                    
                    #include "%s"
                    
                    
                    )FOO",header_file_string);
            
            FWrite(src_file,(void*)buffer,strlen(buffer));
        }
        
        else{
            
            FWrite(src_file,(void*)header,strlen(header));
            
        }
        
        
        
    }
    
    //header declarations
    
    
    InternalWriteStructs(src_file,header_file,struct_array,struct_count);
    
    InternalWriteEnums(src_file,header_file,enum_array,enum_count);
    
    InternalWriteFunctions(src_file,header_file,function_array,function_count);
    
    if(header_file){
        
        auto functiondecl_string = R"FOO(
        
        MetaEnumData* MetaGetEnumByNameHash(u32 hash);
        
        MetaEnumData* MetaGetEnumByName(const s8* name);
        
        u64 MetaGetEnumValueByNameHash(MetaEnumEntry* array,u32 count,u32 hash);
        
        u64 MetaGetEnumValueByName(MetaEnumEntry* array,u32 count,const s8* name);
        
        void MetaGetEnumNamesByValue(u64 value,MetaEnumEntry* array,u32 count,const s8** names_array,u32* names_count);
        
        MetaDataStructEntry* MetaGetStructByNameHash(u32 hash);
        
        MetaDataStructEntry* MetaGetStructByName(const s8* name);
        
        MetaDataStructEntry* MetaGetStructByName(const s8* name);
        
        u32 MetaGetTypeByNameHash(u32 hash,MetaStructEntry* array,
    u32 array_count);
    
        u32 MetaGetTypeByName(const s8* name,MetaStructEntry* array,
    u32 array_count);
    
    b32 MetaGetValueByNameHash(void* obj,u32 index,void* outdata,u32 hash,MetaStructEntry* array,
    u32 array_count);
    
    b32 MetaGetValueByName(void* obj,u32 index,void* outdata,const s8* name,MetaStructEntry* array,
    u32 array_count);
    
    b32 MetaSetValueByNameHash(void* obj,u32 index,void* value,u32 hash,MetaStructEntry* array,
    u32 array_count);
    
    b32 MetaSetValueByName(void* obj,u32 index,void* value,const s8* name,MetaStructEntry* array,
    u32 array_count);
    
    b32 MetaIsCType(u32 hash);
    
    b32 MetaIsCType(const s8* string);
    
    u32 MetaStringToType(const s8* string);
    
    b32 IsIntType(u32 type);
    
    b32 IsFloatType(u32 type);
    
    MetaFunctionData* MetaGetFunctionByNameHash(u32 hash);
    
    MetaFunctionData* MetaGetFunctionByName(const s8* name);
    
    m64 MetaCallFunction(MetaFunctionData* function,u64* value_array);
    
    u64 MetaCallFunction(u32 hash,u64* value_array);
    
    u64 MetaCallFunction(const s8* name,u64* value_array);
    
)FOO";
        
        FWrite(header_file,(void*)functiondecl_string,strlen(functiondecl_string));
    }
    
    
    
    auto function_string = R"FOO(
    
    MetaEnumData* MetaGetEnumByNameHash(u32 hash){
    
    for(u32 i = 0; i < _arraycount(META_ENUM_ARRAY);i++){
    
    auto entry = &META_ENUM_ARRAY[i];
    
    if(hash == entry->name_hash){
    return entry;
    }
    
    }
    
    return 0;
    }
    
    MetaEnumData* MetaGetEnumByName(const s8* name){
    
    return MetaGetEnumByNameHash(PHashString(name));
    }
    
    u64 MetaGetEnumValueByNameHash(MetaEnumEntry* array,u32 count,u32 hash){
    
    for(u32 i = 0; i < count; i++){
    
    auto k = &array[i];
    
    if(k->name_hash == hash){
    
    return k->value;
    }
    
    }
    
    return (u32)-1;
    }
    
    u64 MetaGetEnumValueByName(MetaEnumEntry* array,u32 count,const s8* name){
    return MetaGetEnumValueByNameHash(array,count,PHashString(name));
    }
    
    void MetaGetEnumNamesByValue(u64 value,MetaEnumEntry* array,u32 count,const s8** names_array,u32* names_count){
    
    u32 c = 0;
    
    for(u32 i = 0; i < count; i++){
    
    auto k = &array[i];
    
    if(k->value == value){
    
    if(names_array){
    names_array[c] = k->name_string;
    }
    
    c++;
    }
    
    }
    
    *names_count = c;
    
    }
    
    MetaDataStructEntry* MetaGetStructByNameHash(u32 hash){
    
    for(u32 i = 0; i < _arraycount(META_STRUCT_ARRAY); i++){
    auto entry = &META_STRUCT_ARRAY[i];
    
    if(hash == entry->name_hash){
    return entry;
    }
    
    }
    
    return 0;
    }
    
    MetaDataStructEntry* MetaGetStructByName(const s8* name){
    return MetaGetStructByNameHash(PHashString(name));
    }
    
    u32 MetaGetTypeByNameHash(u32 hash,MetaStructEntry* array,
    u32 array_count){
    
    for(u32 i = 0; i < array_count; i++){
    MetaStructEntry* entry = &array[i];
    if(entry->name_hash == hash){
    return entry->type_hash;
    }
    }
    return (u32)-1;
    }
    
    
    u32 MetaGetTypeByName(const s8* name,MetaStructEntry* array,
    u32 array_count){
    
    return MetaGetTypeByNameHash(PHashString(name),array,array_count);
    }
    
    
    b32 MetaGetValueByNameHash(void* obj,u32 index,void* outdata,u32 hash,MetaStructEntry* array,
    u32 array_count){
    
    for(u32 i = 0; i < array_count; i++){
    MetaStructEntry* entry = &array[i];
    if(entry->name_hash == hash){
    
    _kill("index exceeds arraycount\n",index >= entry->arraycount);
    
    auto data = (s8*)obj;
    memcpy(outdata,data + entry->offset + (entry->size * index),entry->size);
    return true;
    }
    }
    
    return false;
    }
    
    b32 MetaGetValueByName(void* obj,u32 index,void* outdata,const s8* name,MetaStructEntry* array,
    u32 array_count){
    
    return MetaGetValueByNameHash(obj,index,outdata,PHashString(name),array,array_count);
    }
    
    b32 MetaSetValueByNameHash(void* obj,u32 index,void* value,u32 hash,MetaStructEntry* array,
    u32 array_count){
    
    for(u32 i = 0; i < array_count; i++){
    
    MetaStructEntry* entry = &array[i];
    
    if(entry->name_hash == hash){
    
    _kill("index exceeds arraycount\n",index >= entry->arraycount);
    
    
    auto data = (s8*)obj;
    memcpy(data + entry->offset + (entry->size * index),value,entry->size);
    return true;
    }
    }
    
    return false;
    }
    
    b32 MetaSetValueByName(void* obj,u32 index,void* value,const s8* name,MetaStructEntry* array,
    u32 array_count){
    
    return MetaSetValueByNameHash(obj,index,value,PHashString(name),array,array_count);
    }
    
    b32 MetaIsCType(u32 hash){
    return hash == CType_U8 || hash == CType_U16 || hash == CType_U32  || hash == CType_U64 ||
    hash == CType_S8 || hash == CType_S16 || hash == CType_S32 || hash == CType_S64 ||
    hash == CType_LOGIC || hash == CType_F32 || hash == CType_F64 || hash == CType_PTRSIZE ||
    hash == CType_VOID;
    }
    
    b32 MetaIsCType(const s8* string){
    return MetaIsCType(PHashString(string));
    }
    
    u32 MetaStringToType(const s8* string){
    return PHashString(string);
    }
    
    b32 IsIntType(u32 type){
    
    return type == CType_U8 ||
    type == CType_U16 ||
    type == CType_U32 ||
    type == CType_U64 ||
    type == CType_S8||
    type == CType_S16||
    type == CType_S32||
    type == CType_S64;
    }
    
    b32 IsFloatType(u32 type){
    return type == CType_F32 ||
    type == CType_F64;
    }
    
    
    MetaFunctionData* MetaGetFunctionByNameHash(u32 hash){
    
    for(u32 i = 0; i < _arraycount(META_FUNCTION_ARRAY);i++){
    
    auto f = &META_FUNCTION_ARRAY[i];
    
    if(f->name_hash == hash){
    return f;
}

}

return 0;
}

    MetaFunctionData* MetaGetFunctionByName(const s8* name){
    
    return MetaGetFunctionByNameHash(PHashString(name));
}

#ifdef _WIN32

/*

RCX, RDX, R8, R9 for the first four integer or pointer arguments

XMM0, XMM1, XMM2, XMM3

Integer return values (similar to x86) are returned in RAX if 64 bits or less

Floating point return values are returned in XMM0

*/

extern "C" void InternalFillArgsAndCall(void* call,u64* values,u64* iret,f64* fret);

#endif

m64 MetaCallFunction(MetaFunctionData* function,u64* value_array){


    /*
Linux ABI

The first six integer or pointer arguments are passed in registers RDI, RSI, RDX, RCX, R8, R9 

XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6 and XMM7 are used for certain floating point arguments

Integral return values up to 64 bits in size are stored in RAX while values up to 128 bit are stored in RAX and RDX. Floating-point return values are similarly stored in XMM0 and XMM1

*/

struct Registers{

        u32 int_reg_count;
        u32 float_reg_count;
        
        union{
        
            struct{
            
                u64 RDI;
                u64 RSI;
                u64 RDX;
                u64 RCX;
                
            };
            
            u64 int_reg_array[4];
            
        };
        
        union{
        
            struct{
                double XMM0;
                double XMM1;
                double XMM2;
                double XMM3;
            };
            
            u64 float_reg_array[4];
        };
        
        
    };
    
    Registers registers = {};
    
#ifdef _WIN32

for(u32 i = 0; i < function->args_count;i++){

        auto a = &function->args_array[i];
        auto v = value_array[i];
        
        if(a->is_int){
            registers.int_reg_array[i] = v;
        }
        
        else{
            registers.float_reg_array[i] = v;
        }
    }
    
#else

for(u32 i = 0; i < function->args_count;i++){

        auto a = &function->args_array[i];
        auto v = value_array[i];
        
        if(a->is_int){
            registers.int_reg_array[registers.int_reg_count] = v;
            registers.int_reg_count++;
        }
        
        else{
            registers.float_reg_array[registers.float_reg_count] = v;
            registers.float_reg_count++;
        }
    }
    
#endif

    u64 ret_value = 0;
    f64 fret_value = 0;
    
#ifdef _WIN32

InternalFillArgsAndCall(function->function_call,registers.int_reg_array,&ret_value,&fret_value);

#else   

//TODO: we can condense this down if we want to
    __asm__ volatile (
        "movq %[a1],%%rdi\n"
        "movq %[a2],%%rsi\n"
        "movq %[a3],%%rdx\n"
        "movq %[a4],%%rcx\n"
        "movss %[a7],%%xmm0\n"
        "movss %[a8],%%xmm1\n"
        "movss %[a9],%%xmm2\n"
        "movss %[a10],%%xmm3\n"
        
        "callq *%[c]\n"
        "movq %%rax,%[r1]\n"
        "movss %%xmm0,%[r2]\n"
: [r1] "=g"(ret_value),[r2] "=g"(fret_value)
: [a1] "g" (registers.RDI), [a2] "g" (registers.RSI),[a3] "g" (registers.RDX),
   [a4] "g" (registers.RCX), [a7] "g" (registers.XMM0), [a8] "g" (registers.XMM1), 
[a9] "g" (registers.XMM2), [a10] "g" (registers.XMM3), 
[c] "g" (function->function_call)
        );
        
#endif

m64 return_m = {};

if(function->ret.is_int){
return_m.u = ret_value;
}

else{
return_m.f = fret_value;
}

    return return_m;
    
}

u64 MetaCallFunction(u32 hash,u64* value_array){
return MetaCallFunction(MetaGetFunctionByNameHash(hash),value_array);
}

u64 MetaCallFunction(const s8* name,u64* value_array){
return MetaCallFunction(PHashString(name),value_array);
}


    )FOO";
    
    FWrite(src_file,(void*)function_string,strlen(function_string));
    
#if !(_testing)
    
    FCloseFile(src_file);
    
#endif
    
}

void WriteComponentMetaData(const s8* src_file_string,const s8* header_file_string,GenericStruct* struct_array,u32 struct_count,const s8* inc_file){
    
#if !(_testing)
    
    auto src_file = FOpenFile(src_file_string,F_FLAG_READWRITE |
                              F_FLAG_TRUNCATE | F_FLAG_CREATE);
    
    FileHandle header_file = 0;
    
    if(header_file_string){
        header_file = FOpenFile(header_file_string,F_FLAG_READWRITE |
                                F_FLAG_TRUNCATE | F_FLAG_CREATE);
    }
    
#endif
    
    _kill("meta src_file not found\n",!inc_file);
    
    {
        
        inc_file = InternalGetMainFile(inc_file);
        
        s8 header[1024 * 4] = {};
        
        sprintf(header,
                R"FOO(
                
                /*This file is generated by the preprocessor*/ 
                #pragma once
                
                #include "%s"
                
                struct MetaDataCompEntry{
                u32 array; //offset to component array
                u32 count;//offset to component count
                u32 size;
                s8 name_string[128];
                u32 name_hash;
                MetaStructEntry* member_array;
                u32 member_count;
                };
                
                struct MetaDataCompOut{
                s8* array; //array ptr
                u32* count;// ptr to array count
                u32 size;
                const s8* name_string;
                u32 name_hash;
                MetaStructEntry* member_array;
                u32 member_count;
                };
                
                )FOO",
                inc_file);
        
        
        if(header_file){
            
            FWrite(header_file,(void*)header,strlen(header));
            
            s8 buffer[1024] = {};
            
            sprintf(buffer,
                    R"FOO(
                    
                    #include "%s"
                    
                    
                    )FOO",header_file_string);
            
            FWrite(src_file,(void*)buffer,strlen(buffer));
        }
        
        else{
            
            FWrite(src_file,(void*)header,strlen(header));
            
        }
        
    }
    
    
    {
        
        
        auto comp_struct_string =  "\n\n#define _component_count 200\n\nstruct ComponentStruct{";
        
        FWrite(header_file,(void*)comp_struct_string,strlen(comp_struct_string));
        
        for(u32 i = 0; i < struct_count;i++){
            
            auto s = &struct_array[i];
            
            s8 outbuffer[256] = {};
            
            s8 lowercasebuffer[256] = {};
            
            u32 len = strlen(s->name_string);
            
            for(u32 j = 0; j < len; j++){
                lowercasebuffer[j] = tolower(s->name_string[j]);
            }
            
            sprintf(outbuffer,"\n\n\n%s %s_array[_component_count];\nu32 %s_count = 0;\n",
                    s->name_string,lowercasebuffer,lowercasebuffer);
            
            FWrite(header_file,(void*)outbuffer,strlen(outbuffer));
            
        }
        
        comp_struct_string =  "\n};\n\n";
        
        FWrite(header_file,(void*)comp_struct_string,strlen(comp_struct_string));
    }
    
    {
        
        if(header_file){
            
            s8 buffer[1024] = {};
            
            sprintf(buffer,
                    R"FOO(
                    
                    extern MetaDataCompEntry METACOMP_ARRAY[%d];
                    
                    
                    )FOO",struct_count);
            
            FWrite(header_file,(void*)buffer,strlen(buffer));
        }
        
        auto metacomp_string = "\n\n\nMetaDataCompEntry METACOMP_ARRAY[] = {\n";
        
        FWrite(src_file,(void*)metacomp_string,strlen(metacomp_string));
        
        for(u32 i = 0; i < struct_count; i++){
            
            auto s = &struct_array[i];
            
            u32 len = strlen(s->name_string);
            
            s8 outbuffer[2048] = {};
            s8 lowercasebuffer[256] = {};
            
            for(u32 j = 0; j < len; j++){
                lowercasebuffer[j] = tolower(s->name_string[j]);
            }
            
            sprintf(outbuffer,
                    "{offsetof(ComponentStruct,%s_array),offsetof(ComponentStruct,%s_count),sizeof(ComponentStruct::%s_array[0]),\"%s\",(u32)%d,&%s_META_STRUCT[0],_arraycount(%s_META_STRUCT)},\n",
                    lowercasebuffer,lowercasebuffer,lowercasebuffer,s->name_string,
                    (u32)s->name_hash,s->name_string,
                    s->name_string);
            
            FWrite(src_file,(void*)outbuffer,strlen(outbuffer));
        }
        
        metacomp_string = "\n};\n";
        
        FWrite(src_file,(void*)metacomp_string,strlen(metacomp_string));
    }
    
    auto function_string = R"FOO(
    
    MetaDataCompOut GetComponentData(ComponentStruct* components,
    MetaDataCompEntry metacomp){
    
    auto data = (s8*)components;
    
    MetaDataCompOut output = 
    {(data + metacomp.array),(u32*)(data + metacomp.count),metacomp.size,
    metacomp.name_string,metacomp.name_hash,metacomp.member_array,
    metacomp.member_count};
    
    return output;
    }
    
    MetaDataCompEntry* MetaGetCompByNameHash(u32 hash){
    
    for(u32 i = 0; i < _arraycount(METACOMP_ARRAY); i++){
    auto entry = &METACOMP_ARRAY[i];
    
    if(hash == entry->name_hash){
    return entry;
    }
    
    }
    
    return 0;
    }
    
    MetaDataCompEntry* MetaGetCompByName(const s8* name){
    return MetaGetCompByNameHash(PHashString(name));
    }
    
    void MetaDumpComponents(ComponentStruct* compstruct){
    
    auto metacomp_array = &METACOMP_ARRAY[0];
    u32 metacomp_count = _arraycount(METACOMP_ARRAY);
    
    for(u32 i = 0; i < metacomp_count; i++){
    
    printf("--COMPTYPE--\n");
    
    auto entry = metacomp_array[i];
    
    auto outdata = GetComponentData(compstruct,entry);
    
    auto array = outdata.array;
    auto count = *outdata.count;
    
    for(u32 j = 0; j < count; j++){
    
    printf("--COMP--\n");
    
    auto comp_entry_data = array + (j * outdata.size);
    
    for(u32 k = 0; k < outdata.member_count; k++){
    
    auto comp_meta_entry = outdata.member_array[k];
    
    printf("%s %s : ",comp_meta_entry.type_string,comp_meta_entry.name_string);
    
    for(u32 k = 0; k < comp_meta_entry.arraycount; k++){
    
    s8 buffer[256] = {};
    
    MetaGetValueByName(comp_entry_data,k,&buffer[0],comp_meta_entry.name_string,
    outdata.member_array,outdata.member_count);
    
    if(
    comp_meta_entry.type_hash == CType_U8 ||
    comp_meta_entry.type_hash == CType_U16 ||
    comp_meta_entry.type_hash == CType_U32 ||
    comp_meta_entry.type_hash == CType_U64 ||
    comp_meta_entry.type_hash == CType_S8||
    comp_meta_entry.type_hash == CType_S16||
    comp_meta_entry.type_hash == CType_S32||
    comp_meta_entry.type_hash == CType_S64
    ){
    printf("%d\n",*((u32*)(&buffer[0])));
    }
    
    else if(comp_meta_entry.type_hash == CType_F32 ||
    comp_meta_entry.type_hash == CType_F64){
    printf("%f\n",*((f32*)(&buffer[0])));
    }
    
    else{
    printf("%d\n",*((u32*)(&buffer[0])));
    }
    
    }
    
    }
    
    }
    
    }
    
    }
    
    void MetaDump(){
    
    for(u32 i = 0; i < _arraycount(METACOMP_ARRAY); i++){
    
    auto meta_comp = &METACOMP_ARRAY[i];
    
    printf("\nCOMP: %s\n",meta_comp->name_string);
    
    for(u32 j = 0; j < meta_comp->member_count; j++){
    
    auto field_entry = &meta_comp->member_array[j];
    
    printf("%s %s\n",field_entry->type_string,field_entry->name_string);
    }
    
    }
    
    }
    
    )FOO";
    
    if(header_file){
        
        
        auto functiondecl_string = R"FOO(
        
    MetaDataCompOut GetComponentData(ComponentStruct* components,
    MetaDataCompEntry metacomp);
    
MetaDataCompEntry* MetaGetCompByNameHash(u32 hash);

MetaDataCompEntry* MetaGetCompByName(const s8* name);

    void MetaDumpComponents(ComponentStruct* compstruct);
    
    void MetaDump();
    
    )FOO";
        
        FWrite(header_file,(void*)functiondecl_string,strlen(functiondecl_string));
        
    }
    
    FWrite(src_file,(void*)function_string,strlen(function_string));
    
#if !(_testing)
    
    FCloseFile(src_file);
    
#endif
}

b32 IsValidName(s8* name_buffer){
    
    auto len = strlen(name_buffer);
    
    for(u32 j = 0; j < len; j++){
        
        if(name_buffer[j] == '_' && name_buffer[j + 1] == '_'){
            return false;
        }
    }
    
    return true;
}

b32 ValidateAndFillNameBufferStruct(EvalChar* eval_buffer,u32 count,const s8* parent_name,s8* name_buffer,ParserKeyWord* key = 0){
    
    for(u32 i = 0; i < count; i++){
        
        auto c = &eval_buffer[i];
        
        if(c->tag == TAG_SYMBOL){
            
            if(parent_name){
                sprintf(&name_buffer[0],"%s__%s",parent_name,&c->string[0]);
            }
            
            else{
                memcpy(&name_buffer[0],&c->string[0],strlen(&c->string[0]));
                
                return !IsValidName(name_buffer);
            }
            
            
        }
        
        if(c->tag == TAG_KEY && key){
            *key = (ParserKeyWord)c->hash;
        }
        
    }
    
    return false;
}

b32 ValidateNameBufferStruct(EvalChar* eval_buffer,u32 count,const s8* parent_name,s8* name_buffer,ParserKeyWord* key = 0){
    
    _kill("name buffer must be prefilled\n",!strlen(name_buffer));
    
    
    for(u32 i = 0; i < count; i++){
        
        auto c = &eval_buffer[i];
        
        if(c->tag == TAG_KEY && key){
            *key = (ParserKeyWord)c->hash;
        }
        
    }
    
    
    if(parent_name){
        sprintf(&name_buffer[0],"%s__%s",parent_name,name_buffer);
    }
    
    else{
        
        return !IsValidName(name_buffer);
    }
    
    return false;
    
}
