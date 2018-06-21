#pragma once
#include "aallocator.h"

#include "stdio.h"
#include "stdlib.h"

#include "ttype.h"
#include "mode.h"

#include "ffileio.h"
#include "pparse.h"
#include "ctype.h"

#define _testing 0

#if _testing

#define FWrite(file,buffer,len) printf("%s",(s8*)(buffer));

#endif

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


enum CType{
    
    CType_U8 = PHashString("u8"),
    CType_U16 = PHashString("u16"),
    CType_U32 = PHashString("u32"),
    CType_U64 = PHashString("u64"),
    
    CType_S8 = PHashString("s8"),
    CType_S16 = PHashString("s16"),
    CType_S32 = PHashString("s32"),
    CType_S64 = PHashString("s64"),
    
    CType_LOGIC = PHashString("logic"),
    
    CType_F32 = PHashString("f32"),
    CType_F64 = PHashString("f64"),
    
    CType_PTRSIZE = PHashString("ptrsize"),
    
    CType_VOID = PHashString("void"),
    
    CType_STRUCT = PHashString("struct"),
    CType_ENUM = PHashString("enum"),
    CType_UNION = PHashString("union"),//Do we support unions?
};

logic IsIntType(u32 type){
    
    return type == CType_U8 ||
        type == CType_U16 ||
        type == CType_U32 ||
        type == CType_U64 ||
        type == CType_S8||
        type == CType_S16||
        type == CType_S32||
        type == CType_S64;
}

logic IsFloatType(u32 type){
    return type == CType_F32 ||
        type == CType_F64;
}


void IgnoreWhiteSpace(s8* buffer,u32* cur){
    
    auto k = *cur;
    
    while(PIsWhiteSpace(buffer[k])){
        k++;
    }
    
    *cur = k;
}

void IgnorePreprocessorAndComments(s8* buffer,u32* cur){
    
    auto k = *cur;
    
    
    
    auto is_block = IsStartComment(buffer[k],buffer[k + 1]);
    
    while(is_block){
        
        is_block = IsEndComment(buffer[k],buffer[k + 1]);
        k++;
        
        if(!is_block){
            
            k+=2;
            IgnoreWhiteSpace(buffer,cur);
        }
        
    }
    
    *cur = k;
}



void SanitizeString(s8* buffer,u32* k){
    
    auto cur = *k;
    
    IgnoreWhiteSpace(buffer,&cur);
    
    for(;;){
        
        logic reparse = false;
        
        if(IsComment(buffer[cur],buffer[cur + 1])){
            PSkipLine(buffer,&cur);
            reparse = true;
        }
        
        
        if(IsPreprocessor(buffer[cur])){
            PSkipLine(buffer,&cur);
            reparse = true;
        }
        
        
        auto keep_parsing = IsStartComment(buffer[cur],buffer[cur + 1]);
        
        while(keep_parsing){
            
            keep_parsing = !IsEndComment(buffer[cur],buffer[cur + 1]);
            cur++;
            
            if(!keep_parsing){
                cur += 2;
                reparse = true;
            }
        }
        
        
        if(!reparse){
            break;
        }
        
        
    }
    
    *k = cur;
}

logic IsCType(u64 hash){
    
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

struct GenericTypeDec{
    
    s8 type_string[128];
    s8 name_string[128];
    u64 name_hash;
    CType type;
};

struct GenericTypeDef : GenericTypeDec{
    
    u8 indir_count;
    u8 dim_array_count;
    u8 default_count;//MARK: -1 is {}
    
    
    u16 dim_array[8];
    
    union{
        s8 default_string[256];
        u64 default_array[32];
        double defaultf_array[32];
    };
    
    
    
};

struct GenericEnumEntry{
    u64 name_hash;
    s8 name_string[128];
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
    
    GenericTypeDef args_array[256];
    u32 args_count;
};

enum ParserKeyWord{
    PARSERKEYWORD_REFL = PHashString("REFL"),
    PARSERKEYWORD_COMPONENT = PHashString("REFLCOMPONENT"),
};

struct GenericStruct : GenericTypeDec{
    ParserKeyWord pkey;
    u32 members_count = 0;
    GenericTypeDef members_array[256];
};

enum ParseTags{
    TAG_SYMBOL = 0,
    TAG_CTYPE,
    TAG_STRUCT,
    TAG_KEY,
    TAG_VALUE,
    TAG_START_ARG,
    TAG_END_ARG,
    TAG_ENUM,
    TAG_UNION,
    TAG_INDIR,
    TAG_ASSIGN,
    
    //TODO: implement this
    TAG_START_SQUARE,
    TAG_END_SQUARE,
    TAG_START_CURLY,
    TAG_END_CURLY,
    
    TAG_DOUBLE_QUOTE,
};

struct EvalChar{
    u64 hash;
    s8 string[128] = {};
    ParseTags tag;
};

logic IsParserKeyword(u64 hash){
    
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

logic IsReflStruct(EvalChar* eval_buffer,u32 count){
    
    logic is_struct = eval_buffer[0].tag == TAG_STRUCT;
    
    logic has_keyword = false;
    
    for(u32 i = 1; i < count; i++){
        
        has_keyword = eval_buffer[i].tag == TAG_KEY;
        
        if(has_keyword){
            break;
        }
    }
    
    return is_struct && has_keyword;
}

logic IsReflEnum(EvalChar* eval_buffer,u32 count){
    
    logic is_enum = eval_buffer[0].tag == TAG_ENUM;
    
    logic has_keyword = false;
    
    for(u32 i = 1; i < count; i++){
        
        has_keyword = eval_buffer[i].tag == TAG_KEY;
        
        if(has_keyword){
            break;
        }
    }
    
    return is_enum && has_keyword;
}

logic IsReflFunc(EvalChar* eval_buffer,u32 count){
    
    logic has_return_type = eval_buffer[0].tag == TAG_CTYPE;
    
    logic has_arg_brackets = false;
    
    for(u32 i = 1; i < count - 1; i++){
        
        auto a = eval_buffer[i];
        auto b = eval_buffer[i + 1];
        
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
    
    return has_return_type && has_arg_brackets;
}

//NOTE: we will crash if we encounter a '}' first
void SkipBracketBlock(s8* buffer,u32* a){
    
    auto cur = *a;
    
    u32 k = 0;
    
    for(;;cur++){
        
        auto c = buffer[cur];
        
        if(c == '{'){
            k++;
        }
        
        if(c == '}'){
            
            _kill("incomplete scope error\n",!k);
            
            k --;
        }
        
        if(!k){
            break;
        }
    }
    
    *a = cur;
}

void ExtractScope(s8* scope_buffer,s8* buffer,u32* a){
    
    u32 scope_count = 0;
    u32 count = *a;
    u32 i = 0;
    
    for(;;){
        
        auto c = buffer[count];
        scope_buffer[i] = c;
        
        if(c == '{'){
            scope_count++;
        }
        
        if(c == '}'){
            scope_count--;
        }
        
        
        count++;
        i++;
        
        
        if(!scope_count){
            break;
        }
    }
    
    *a = count;
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
        
        for(u32 k = 0; k < f->dim_array_count;k++){
            
            printf("[%d]",f->dim_array[k]);
        }
        
        if(f->default_count){
            
            if(f->default_count == (u8)-1){
                
                printf("= {}");
            }
            
            else if(f->default_count == (u8)-2){
                
                printf("= \"%s\"",&f->default_string[0]);
            }
            
            else{
                
                printf("= {");
                
                for(u32 j = 0; j < f->default_count; j++){
                    
                    
                    if(IsIntType(PHashString(f->type_string))){
                        printf(" %d ",(u32)f->default_array[j]);
                    }
                    
                    if(IsFloatType(PHashString(f->type_string))){
                        printf("%f ",f->defaultf_array[j]);
                    }
                }
                
                printf("}");
            }
            
        }
        
        
        
        printf("\n");
    }
}

void GetArgsData(s8** argv,u32 argc,s8*** source_array,u32* source_count,s8** componentfile,s8** metafile){
    
    u32 i = 1;
    
    for(; i < argc; i++){
        
        auto hash = PHashString(argv[i]);
        
        if(hash == PHashString("-component") || hash == PHashString("-meta")){
            break;
        }
    }
    
    *source_count = i - 1;
    
    if((*source_count)){
        *source_array = &argv[1];
    }
    
    u32 k = (argc - (*source_count) - 1);
    
    if(k != 4 && k != 2){
        
        *source_count = (u32)-1;
        
        return;
    }
    
    for(; i < argc; i++){
        
        if(PHashString(argv[i]) == PHashString("-component")){
            i++;
            *componentfile = argv[i];
        }
        
        if(PHashString(argv[i]) == PHashString("-meta")){
            i++;
            *metafile = argv[i];
        }
        
    }
    
    
    
}

const s8* InternalGetMainFile(const s8* string){
    
    auto len = strlen(string);
    
    for(u32 i = len - 1; i != (u32)-1; i--){
        
        if(string[i] == '/'){
            return &string[i + 1];
        }
    }
    
    return 0;
}

void WriteMetaFile(const s8* file_string,GenericStruct* struct_array,u32 struct_count,GenericEnum* enum_array,u32 enum_count){
    
#if !(_testing)
    
    auto file = FOpenFile(file_string,F_FLAG_READWRITE |
                          F_FLAG_TRUNCATE | F_FLAG_CREATE);
    
#endif
    
    {
        
        auto buffer = R"FOO(
                /*This file is generated by the preprocessor*/ 
                #pragma once
                
                
                #include"pparse.h"
                #include "aassetmanager.h"
                
                struct MetaDataEntry{
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
                u32 element_size;
                s8 comp_name_string[128];
                u32 comp_name_hash;
                MetaDataEntry* metadata_table;
                u32 metadata_count;
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

                enum CType{
                CType_U8 = PHashString("u8"),
                CType_U16 = PHashString("u16"),
                CType_U32 = PHashString("u32"),
                CType_U64 = PHashString("u64"),
                CType_S8 = PHashString("s8"),
                CType_S16 = PHashString("s16"),
                CType_S32 = PHashString("s32"),
                CType_S64 = PHashString("s64"),
                CType_LOGIC = PHashString("logic"),
                CType_F32 = PHashString("f32"),
                CType_F64 = PHashString("f64"),
                CType_PTRSIZE = PHashString("ptrsize"),
                CType_VOID = PHashString("void"),
                CType_STRUCT,
                };
                
                )FOO";
        
        FWrite(file,(void*)buffer,strlen(buffer));
        
    }
    
    //structs
    
    for(u32 i = 0; i < struct_count; i++){
        
        auto s = &struct_array[i];
        
        {
            
            s8 buffer[256] = {};
            
            sprintf(buffer,"\n\n_persist MetaDataEntry %s_META_STRUCT[] = {\n",s->name_string);
            
            FWrite(file,(void*)buffer,strlen(buffer));
            
        }
        
        {
            
            
            for(u32 j = 0; j < s->members_count;j++){
                
                auto m = &s->members_array[j];
                
                s8 buffer[256] = {};
                
                //TODO: support more than 1d arrays
                //Remove ref_metadatacomp_index
                sprintf(buffer,"{(u32)%d,(u32)%d,\"%s\",\"%s\",(u32)sizeof(%s),(u32)offsetof(%s,%s),%d,(u32)-1},\n"
                        ,(u32)PHashString(m->type_string),(u32)m->name_hash,m->type_string,m->name_string,m->type_string,s->name_string,m->name_string,m->dim_array[0]);
                
                FWrite(file,(void*)buffer,strlen(buffer));
            }
            
            
            auto end_struct = "};";
            FWrite(file,(void*)end_struct,strlen(end_struct));
            
        }
    }
    
    
    {
        auto metacomp_string = "\n\n\nMetaDataStructEntry META_STRUCT_ARRAY[] = {\n";
        
        FWrite(file,(void*)metacomp_string,strlen(metacomp_string));
        
        for(u32 i = 0; i < struct_count; i++){
            
            auto s = &struct_array[i];
            
            u32 len = strlen(s->name_string);
            
            s8 outbuffer[2048] = {};
            s8 lowercasebuffer[256] = {};
            
            for(u32 j = 0; j < len; j++){
                lowercasebuffer[j] = tolower(s->name_string[j]);
            }
            
            sprintf(outbuffer,
                    "{sizeof(%s),\"%s\",(u32)%d,&%s_META_STRUCT[0],_arraycount(%s_META_STRUCT)},\n",
                    s->name_string,s->name_string,
                    (u32)s->name_hash,s->name_string,
                    s->name_string);
            
            FWrite(file,(void*)outbuffer,strlen(outbuffer));
        }
        
        metacomp_string = "\n};\n";
        
        FWrite(file,(void*)metacomp_string,strlen(metacomp_string));
    }
    
    
    //enums
    
    {
        
        for(u32 i = 0; i < enum_count;i++){
            
            auto e = &enum_array[i];
            
            {
                s8 buffer[256] = {};
                
                sprintf(buffer,"\n\n_persist MetaEnumEntry %s_META_ENUM[] = {\n",e->name_string);
                
                FWrite(file,(void*)buffer,strlen(buffer));
            }
            
            for(u32 j = 0; j < e->members_count;j++){
                
                auto m = &e->members_array[j];
                
                {
                    s8 buffer[256] = {};
                    
                    sprintf(buffer,"{%d,\"%s\",(u64)%s},\n",
                            (u32)m->name_hash,m->name_string,m->name_string);
                    
                    FWrite(file,(void*)buffer,strlen(buffer));
                }
            }
            
            auto end_struct = "};";
            FWrite(file,(void*)end_struct,strlen(end_struct));
        }
        
        {
            
            auto metacomp_string = "\n\n\nMetaEnumData META_ENUM_ARRAY[] = {\n";
            
            FWrite(file,(void*)metacomp_string,strlen(metacomp_string));
            
            for(u32 i = 0; i < enum_count;i++){
                
                auto e = &enum_array[i];
                
                {
                    s8 buffer[256] = {};
                    
                    sprintf(buffer,"{%d,\"%s\",sizeof(%s),_arraycount(%s_META_ENUM),&%s_META_ENUM[0]},\n",
                            (u32)e->name_hash,e->name_string,e->members_array[0].name_string,e->name_string,e->name_string);
                    
                    FWrite(file,(void*)buffer,strlen(buffer));
                }
                
            }
            
            auto end_struct = "};";
            FWrite(file,(void*)end_struct,strlen(end_struct));
            
        }
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

void MetaGetEnumNamesByValue(u32 value,MetaEnumEntry* array,u32 count,const s8** names_array,u32* names_count){

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
  
  if(hash == entry->comp_name_hash){
  return entry;
  }
  
  }
  
  return 0;
  }
  
  MetaDataStructEntry* MetaGetStructByName(const s8* name){
  return MetaGetStructByNameHash(PHashString(name));
  }
  
  u32 MetaGetTypeByNameHash(u32 hash,MetaDataEntry* array,
  u32 array_count){
  
  for(u32 i = 0; i < array_count; i++){
  MetaDataEntry* entry = &array[i];
  if(entry->name_hash == hash){
  return entry->type_hash;
  }
  }
  return (u32)-1;
  }
  
  
  u32 MetaGetTypeByName(const s8* name,MetaDataEntry* array,
  u32 array_count){
  
  return MetaGetTypeByNameHash(PHashString(name),array,array_count);
  }
  
  
  logic MetaGetValueByNameHash(void* obj,u32 index,void* outdata,u32 hash,MetaDataEntry* array,
  u32 array_count){
  
  for(u32 i = 0; i < array_count; i++){
  MetaDataEntry* entry = &array[i];
  if(entry->name_hash == hash){
  
  _kill("index exceeds arraycount\n",index >= entry->arraycount);
  
  auto data = (s8*)obj;
  memcpy(outdata,data + entry->offset + (entry->size * index),entry->size);
  return true;
  }
  }
  
  return false;
  }
  
  logic MetaGetValueByName(void* obj,u32 index,void* outdata,const s8* name,MetaDataEntry* array,
  u32 array_count){
  
  return MetaGetValueByNameHash(obj,index,outdata,PHashString(name),array,array_count);
  }
  
  logic MetaSetValueByNameHash(void* obj,u32 index,void* value,u32 hash,MetaDataEntry* array,
  u32 array_count){
  
  for(u32 i = 0; i < array_count; i++){
  
  MetaDataEntry* entry = &array[i];
  
  if(entry->name_hash == hash){
  
  _kill("index exceeds arraycount\n",index >= entry->arraycount);
  
  
  auto data = (s8*)obj;
  memcpy(data + entry->offset + (entry->size * index),value,entry->size);
  return true;
  }
  }
  
  return false;
  }
  
  logic MetaSetValueByName(void* obj,u32 index,void* value,const s8* name,MetaDataEntry* array,
  u32 array_count){
  
  return MetaSetValueByNameHash(obj,index,value,PHashString(name),array,array_count);
  }
  
  logic MetaIsCType(u32 hash){
  return hash == CType_U8 || hash == CType_U16 || hash == CType_U32  || hash == CType_U64 ||
  hash == CType_S8 || hash == CType_S16 || hash == CType_S32 || hash == CType_S64 ||
  hash == CType_LOGIC || hash == CType_F32 || hash == CType_F64 || hash == CType_PTRSIZE ||
  hash == CType_VOID;
  }
  
  logic MetaIsCType(const s8* string){
  return MetaIsCType(PHashString(string));
  }
  
  u32 _ainline MetaStringToType(const s8* string){
  return PHashString(string);
  }
  
  logic IsIntType(u32 type){
  
  return type == CType_U8 ||
          type == CType_U16 ||
          type == CType_U32 ||
          type == CType_U64 ||
          type == CType_S8||
          type == CType_S16||
          type == CType_S32||
          type == CType_S64;
  }
  
  logic IsFloatType(u32 type){
  return type == CType_F32 ||
        type == CType_F64;
  }
  
  
  )FOO";
    
    FWrite(file,(void*)function_string,strlen(function_string));
    
#if !(_testing)
    
    FCloseFile(file);
    
#endif
    
}

void WriteComponentMetaData(const s8* file_string,GenericStruct* struct_array,u32 struct_count,const s8* metafile){
    
#if !(_testing)
    
    auto file = FOpenFile(file_string,F_FLAG_READWRITE |
                          F_FLAG_TRUNCATE | F_FLAG_CREATE);
    
#endif
    
    _kill("meta file not found\n",!metafile);
    
    {
        
        metafile = InternalGetMainFile(metafile);
        
        s8 buffer[1024 * 4] = {};
        
        sprintf(buffer,
                R"FOO(
                
                /*This file is generated by the preprocessor*/ 
                #pragma once
                
                #include "%s"
                
                struct MetaDataCompEntry{
                u32 array; //offset to component array
                u32 count;//offset to component count
                u32 element_size;
                s8 comp_name_string[128];
                u32 comp_name_hash;
                MetaDataEntry* metadata_table;
                u32 metadata_count;
                };
                
                #define _component_count 200
                
                )FOO",
                metafile);
        
        FWrite(file,(void*)buffer,strlen(buffer));
        
    }
    
    
    {
        
        auto comp_struct_string =  "\n\nstruct ComponentStruct{";
        
        FWrite(file,(void*)comp_struct_string,strlen(comp_struct_string));
        
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
            
            FWrite(file,(void*)outbuffer,strlen(outbuffer));
            
        }
        
        comp_struct_string =  "\n};\n\n";
        
        FWrite(file,(void*)comp_struct_string,strlen(comp_struct_string));
    }
    
    {
        auto metacomp_string = "\n\n\nMetaDataCompEntry METACOMP_ARRAY[] = {\n";
        
        FWrite(file,(void*)metacomp_string,strlen(metacomp_string));
        
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
            
            FWrite(file,(void*)outbuffer,strlen(outbuffer));
        }
        
        metacomp_string = "\n};\n";
        
        FWrite(file,(void*)metacomp_string,strlen(metacomp_string));
    }
    
    auto function_string = R"FOO(
    
    struct MetaDataCompOut{
  s8* array; //array ptr
  u32* count;// ptr to array count
  u32 element_size;
  const s8* comp_name_string;
  u32 comp_name_hash;
  MetaDataEntry* metadata_table;
  u32 metadata_count;
  };
  
    MetaDataCompOut GetComponentData(ComponentStruct* components,
  MetaDataCompEntry metacomp){
  
  auto data = (s8*)components;
  
  MetaDataCompOut output = 
  {(data + metacomp.array),(u32*)(data + metacomp.count),metacomp.element_size,
  metacomp.comp_name_string,metacomp.comp_name_hash,metacomp.metadata_table,
  metacomp.metadata_count};
  
  return output;
  }
  
      MetaDataCompEntry* MetaGetCompByNameHash(u32 hash){
      
  for(u32 i = 0; i < _arraycount(METACOMP_ARRAY); i++){
  auto entry = &METACOMP_ARRAY[i];
  
  if(hash == entry->comp_name_hash){
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
        
        auto comp_entry_data = array + (j * outdata.element_size);
        
        for(u32 k = 0; k < outdata.metadata_count; k++){
        
   auto comp_meta_entry = outdata.metadata_table[k];
   
   printf("%s %s : ",comp_meta_entry.type_string,comp_meta_entry.name_string);
   
   for(u32 k = 0; k < comp_meta_entry.arraycount; k++){
   
   s8 buffer[256] = {};
   
   MetaGetValueByName(comp_entry_data,k,&buffer[0],comp_meta_entry.name_string,
        outdata.metadata_table,outdata.metadata_count);
        
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
      
      printf("\nCOMP: %s\n",meta_comp->comp_name_string);
      
      for(u32 j = 0; j < meta_comp->metadata_count; j++){
      
        auto field_entry = &meta_comp->metadata_table[j];
        
        printf("%s %s\n",field_entry->type_string,field_entry->name_string);
      }
      
    }
    
  }
  
  )FOO";
    
    FWrite(file,(void*)function_string,strlen(function_string));
    
#if !(_testing)
    
    FCloseFile(file);
    
#endif
}