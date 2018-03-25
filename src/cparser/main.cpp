#include "stdio.h"
#include "stdlib.h"

#include "ttype.h"
#include "mode.h"

#include "ffileio.h"
#include "pparse.h"

#include "main.h"

#include "ctype.h"

/*
  TODO: 
  handle inheritance and enums.
  ptr inspection can be implemented in assembly
  be able to iterate through all components
  IDREF() should do a search in that component id
  
  FIXME: We do not always parse out // comments properly
*/


enum ParsedType{
    ParsedType_STRUCT = 0,
    ParsedType_CType,
    ParsedType_ExtKeyword,
    ParsedType_STRUCTNAME,
    ParsedType_VALUENAME,
    ParsedType_IDREF,
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
    
    CType_STRUCT = PHashString("struct"),
    CType_ENUM = PHashString("enum"),
    CType_UNION = PHashString("union"),
};

enum Keyword{
    Keyword_STRUCT = PHashString("struct"),
};

enum ExtKeyword {
    ExtKeyword_NONE,
    ExtKeyword_REFLCOMPONENT = PHashString("REFLCOMPONENT"),
    ExtKeyword_IDREFCOMPONENT = PHashString("IDREF"),
};

logic IsCType(u32 hash){
    
    return hash == CType_U8 || hash == CType_U16 || hash == CType_U32  || hash == CType_U64 ||
        hash == CType_S8 || hash == CType_S16 || hash == CType_S32 || hash == CType_S64 ||
        hash == CType_LOGIC || hash == CType_F32 || hash == CType_F64 || hash == CType_PTRSIZE ||
        hash == CType_VOID;
}

logic IsExtKeyword(u32 hash){
    return hash == ExtKeyword_REFLCOMPONENT || hash == ExtKeyword_IDREFCOMPONENT;
}


//This is unprocessed data
struct StructEntry{
    s8 name[256];
    CType type;
    s8 type_name[256];
    u32 type_hash;
    u32 arraycount;
    u32 indirectioncount;
    u32 idref_hash;
};

struct FormedStruct{
    s8 name[256];
    u32 name_hash;
    StructEntry entry_array[128];
    u32 entry_count;
    ExtKeyword ext_attrib;
    ParsedType prev_hash;
};

FormedStruct ResolveStructReference(u32 type_hash,FormedStruct* array,u32 count){
    
    for(u32 i = 0; i < count; i++){
        auto struct_entry = &array[i];
        
        if(struct_entry->name_hash == type_hash){
            return *struct_entry;
        }
        
    }
    
    return {};
}

void ProcessStruct(u32 hash,s8* string,FormedStruct* struct_entry){
    
    if(PIsNumeric(string[0])){
        return;
    }
    
    if(struct_entry->prev_hash == ParsedType_IDREF){
        struct_entry->prev_hash = ParsedType_VALUENAME;
        struct_entry->entry_array[struct_entry->entry_count].idref_hash = hash;
        return;
    }
    
    if(IsCType(hash)){
        
        _kill("not empty\n",
              strlen(struct_entry->entry_array[struct_entry->entry_count].type_name) != 0);
        
        
        struct_entry->prev_hash = ParsedType_CType;
        struct_entry->entry_array[struct_entry->entry_count].type = (CType)hash;
        memcpy(struct_entry->entry_array[struct_entry->entry_count].type_name,string,
               strlen(string));
    }
    
    else if(IsExtKeyword(hash)){
        
        if(hash == ExtKeyword_REFLCOMPONENT){
            struct_entry->prev_hash = ParsedType_ExtKeyword;
            struct_entry->ext_attrib = (ExtKeyword)hash;
        }
        
        else{
            struct_entry->prev_hash = ParsedType_IDREF;
        }
        
    }
    
    else if(struct_entry->prev_hash == ParsedType_STRUCT ||
            struct_entry->prev_hash == ParsedType_ExtKeyword){
        
        struct_entry->prev_hash = ParsedType_STRUCTNAME;
        memcpy(struct_entry->name,string,strlen(string));
        struct_entry->name_hash = PHashString(struct_entry->name);
    }
    
    else if(struct_entry->prev_hash == ParsedType_CType){
        struct_entry->prev_hash = ParsedType_VALUENAME;
        memcpy(struct_entry->entry_array[struct_entry->entry_count].name,string,strlen(string));
    }
    
    else{
        
        struct_entry->prev_hash = ParsedType_CType;
        
        auto len = strlen(string);
        
        struct_entry->entry_array[struct_entry->entry_count].type = CType_STRUCT;
        memcpy(struct_entry->entry_array[struct_entry->entry_count].type_name,string,len);
        
        struct_entry->entry_array[struct_entry->entry_count].type_hash =
            PHashString(struct_entry->entry_array[struct_entry->entry_count].type_name);
        
    }
    
}

void Lex(s8* string,ParseStateStack* pstate,FormedStruct* struct_array,u32* struct_count){
    
    u32 cur = 0;
    u32 len = strlen(string);
    
    FormedStruct* cur_entry = &struct_array[(*struct_count) - 1];
    
    for(;;){
        
        if(cur >= len || IsPreprocessor(string[cur]) || IsComment(string[cur],string[cur + 1])){
            break;
        }
        
        PSkipWhiteSpace(string,&cur);
        SkipBlockComments(string,&cur,pstate);
        
        
        s8 dst[512] = {};
        
        PGetSymbol(dst,string,&cur,0);
        
        if(strlen(dst)){
            
            auto hash = PHashString(dst);
            
            if(hash == Keyword_STRUCT){
                
                cur_entry = &struct_array[(*struct_count)];
                (*struct_count)++;
                
                (*cur_entry) = {};
                
                for(u32 i = 0; i < _arraycount(cur_entry->entry_array); i++){
                    cur_entry->entry_array[i].arraycount = 1;  
                }
                
                
                PushState(pstate,ParseState_STRUCT);
                continue;
            }
            
            if(GetState(pstate) == ParseState_STRUCT || GetState(pstate) == ParseState_SCOPEBLOCK){
                ProcessStruct(hash,dst,cur_entry);
            }
            
            if(GetState(pstate) == ParseState_SCOPEARRAY){
                cur_entry->entry_array[cur_entry->entry_count].arraycount = atoi(dst);	
            }
            
        }
        
        if(GetState(pstate) == ParseState_STRUCT && string[cur] == '{'){
            PushState(pstate,ParseState_SCOPEBLOCK);
        }
        
        if(GetState(pstate) == ParseState_SCOPEBLOCK && string[cur] == '}'){
            PopState(pstate);
        }
        
        if((GetState(pstate) == ParseState_SCOPEBLOCK) && string[cur] == '*'){
            cur_entry->entry_array[cur_entry->entry_count].indirectioncount++;
        }
        
        if((GetState(pstate) == ParseState_STRUCT ||
            GetState(pstate) == ParseState_SCOPEBLOCK) && string[cur] == '['){
            PushState(pstate,ParseState_SCOPEARRAY);
        }
        
        if(GetState(pstate) == ParseState_SCOPEARRAY && string[cur] == ']'){
            PopState(pstate);
        }
        
        if(GetState(pstate) == ParseState_SCOPEBLOCK && string[cur] == ';'){
            cur_entry->entry_count++;
        }
        
        
        if(GetState(pstate) == ParseState_STRUCT && string[cur] == ';'){
            
            
            if(cur_entry->ext_attrib != ExtKeyword_REFLCOMPONENT){
                (*cur_entry) = {};
                (*struct_count)--;	
            }
            
            PopState(pstate);
        }
        
        cur++;
    }
    
}

//TODO: struct reference solver and handle opaque structs
void PrintStruct(FormedStruct* curstruct,FormedStruct* array,u32 array_count){
    
    printf("STRUCT %s\n",curstruct->name);
    
    for(u32 i = 0; i < curstruct->entry_count; i++){
        
        auto entry = &curstruct->entry_array[i];
        
        if(entry->type == CType_STRUCT){
            
            auto ref_struct = ResolveStructReference(entry->type_hash,array,array_count);
            
            //inspectable struct
            if(ref_struct.name_hash){
                printf("Sub struct (%d)\n",entry->indirectioncount);
                PrintStruct(&ref_struct,array,array_count);
                printf("end Sub struct\n");
                continue;
            }
            //opaque struct
            else{
                printf("Opaque struct : ");
            }
            
        }
        
        //standard c type
        printf("%s %s %d (%d)\n",entry->type_name,entry->name,entry->arraycount,
               entry->indirectioncount);
        
    }
    
}

void WriteStructMetaData(FormedStruct* curstruct,FormedStruct* array,u32 array_count,
                         FileHandle file,const s8* offset_string = 0,const s8* parent_string = 0){
    
    if(!offset_string){
        s8 buffer[256] = {};
        
        sprintf(buffer,"\n\n_persist MetaDataEntry %s_METACOMP_STRUCT[] = {\n",curstruct->name);
        
        FWrite(file,(void*)buffer,strlen(buffer));
    }
    
    for(u32 i = 0; i < curstruct->entry_count; i++){
        
        auto entry = &curstruct->entry_array[i];
        
        s8 typename_buffer[256] = {};
        s8 indir_buffer[256] = {};
        
        s8 name_buffer[256] = {};
        
        for(u32 k = 0; k < entry->indirectioncount; k++){
            indir_buffer[k] = '*';
        }
        
        sprintf(typename_buffer,"%s%s",entry->type_name,indir_buffer);  
        
        if(parent_string){
            sprintf(name_buffer,"%s::%s",parent_string,entry->name);  
        }
        
        else{
            memcpy(name_buffer,entry->name,strlen(entry->name));
        }
        
        
        s8 writebuffer[512];
        
        u32 ref_id = (u32)-1;
        
        if(entry->idref_hash){
            for(u32 k = 0; k < array_count; k++){
                
                auto cur_struct = &array[k];
                
                if(entry->idref_hash == cur_struct->name_hash){
                    ref_id = k;
                    break;
                }
                
            }
            
        }
        
        if(offset_string){
            
            sprintf(writebuffer,"{(u32)%d,(u32)%d,\"%s\",\"%s\",sizeof(%s),offsetof(%s,%s) + %s,%d,(u32)%d},\n",
                    (u32)PHashString(typename_buffer),(u32)PHashString(name_buffer),
                    typename_buffer,name_buffer,
                    typename_buffer,
                    curstruct->name,entry->name,offset_string,entry->arraycount,ref_id);
        }
        
        else{
            sprintf(writebuffer,"{(u32)%d,(u32)%d,\"%s\",\"%s\",sizeof(%s),offsetof(%s,%s),%d,(u32)%d},\n",
                    (u32)PHashString(typename_buffer),(u32)PHashString(name_buffer),
                    typename_buffer,name_buffer,
                    typename_buffer,curstruct->name,entry->name,entry->arraycount,ref_id); 
        }
        
        FWrite(file,(void*)writebuffer,strlen(writebuffer));
        
        
        if(entry->type == CType_STRUCT){
            
            auto ref_struct = ResolveStructReference(entry->type_hash,array,array_count);
            
            //inspectable struct
            if(ref_struct.name_hash){
                
                //we do not parse pointers for now
                if(!entry->indirectioncount){
                    
                    s8 offset_buffer[256] = {};
                    s8 parent_buffer[256] = {};
                    
                    sprintf(offset_buffer,"offsetof(%s,%s)",curstruct->name,entry->name);
                    memcpy(parent_buffer,entry->name,strlen(entry->name));
                    
                    WriteStructMetaData(&ref_struct,array,array_count,file,offset_buffer,parent_buffer);
                }
                
                else{
                    printf("We don't handle ptr inspection rn\n");
                }
                
            }
            
        }
        
    }
    
    
    
    if(!offset_string){
        auto end_struct = "};";
        
        FWrite(file,(void*)end_struct,strlen(end_struct));
    }
    
    
}

#if 1

s32 main(s32 argc,s8** argv){
    
    if(argc != 3){
        printf("please provide an in and out file\n");
        
        return -1;
    }
    
    auto infile_string = argv[1];
    
    auto file = FOpenFile(infile_string,F_FLAG_READONLY);
    
    ptrsize size;
    u32 cur = 0;
    auto buffer = FReadFileToBuffer(file,&size);
    
    FCloseFile(file);
    
    
    ParseStateStack pstate;
    
    auto struct_array = (FormedStruct*)alloc(sizeof(FormedStruct) * 1024);
    u32 struct_count = 0;
    
    for(;;){
        
        s8 dst[1024] = {};
        
        PGetLine(dst,buffer,&cur,0);
        
        Lex(dst,&pstate,struct_array,&struct_count);
        
        if(cur >= size){
            break;
        }
        
    }
    
    auto outfile = FOpenFile(argv[2],F_FLAG_READWRITE |
                             F_FLAG_TRUNCATE | F_FLAG_CREATE);
    
    auto initialcontents = R"FOO(
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
  
  struct MetaDataCompEntry{
  u32 array; //offset to component array
  u32 count;//offset to component count
  u32 element_size;
  s8 comp_name_string[128];
  u32 comp_name_hash;
  MetaDataEntry* metadata_table;
  u32 metadata_count;
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
  
  #define _component_count 200
  
  )FOO";
    
    FWrite(outfile,(void*)initialcontents,strlen(initialcontents));
    
    {
        auto comp_struct_string =  "\n\nstruct ComponentStruct{";
        
        FWrite(outfile,(void*)comp_struct_string,strlen(comp_struct_string));
        
        //Write component array
        for(u32 i = 0; i < struct_count; i++){
            
            auto struct_entry = &struct_array[i];
            
            s8 outbuffer[256] = {};
            
            s8 lowercasebuffer[256] = {};
            
            u32 len = strlen(struct_entry->name);
            
            for(u32 j = 0; j < len; j++){
                lowercasebuffer[j] = tolower(struct_entry->name[j]);
            }
            
            sprintf(buffer,"\n\n\n%s %s_array[_component_count];\nu32 %s_count = 0;\n",
                    struct_entry->name,lowercasebuffer,lowercasebuffer);
            
            FWrite(outfile,(void*)buffer,strlen(buffer));
        }
        
        comp_struct_string =  "\n};\n\n";
        
        FWrite(outfile,(void*)comp_struct_string,strlen(comp_struct_string)); 
    }
    
    
    
    for(u32 i = 0; i < struct_count; i++){
        
        auto struct_entry = &struct_array[i];
        
        WriteStructMetaData(struct_entry,struct_array,struct_count,outfile);
        
    }
    
    
    //Write metadata about the component system - list of components and corresponding metadata
    
    auto metacomp_string = "\n\n\nMetaDataCompEntry METACOMP_ARRAY[] = {\n";
    
    FWrite(outfile,(void*)metacomp_string,strlen(metacomp_string));
    
    for(u32 i = 0; i < struct_count; i++){
        
        auto struct_entry = &struct_array[i];
        
        u32 len = strlen(struct_entry->name);
        
        s8 outbuffer[2048] = {};
        s8 lowercasebuffer[256] = {};
        
        for(u32 j = 0; j < len; j++){
            lowercasebuffer[j] = tolower(struct_entry->name[j]);
        }
        
        
        sprintf(outbuffer,
                "{offsetof(ComponentStruct,%s_array),offsetof(ComponentStruct,%s_count),sizeof(ComponentStruct::%s_array[0]),\"%s\",(u32)%d,&%s_METACOMP_STRUCT[0],_arraycount(%s_METACOMP_STRUCT)},\n",
                lowercasebuffer,lowercasebuffer,lowercasebuffer,struct_entry->name,
                (u32)PHashString(struct_entry->name),struct_entry->name,
                struct_entry->name);
        
        FWrite(outfile,(void*)outbuffer,strlen(outbuffer));
    }
    
    metacomp_string = "\n};\n";
    
    FWrite(outfile,(void*)metacomp_string,strlen(metacomp_string));
    
    
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
  
  MetaDataCompEntry* MetaGetCompByNameHash(const s8* name){
  return MetaGetCompByNameHash(PHashString(name));
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
    
    FWrite(outfile,(void*)function_string,strlen(function_string));
    
    unalloc(struct_array);
    FCloseFile(outfile);
    
    return 0;
}

#else

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

//TODO: struct union and enum have the same parse structure (should we condense here?)
enum ParseTags{
    TAG_SYMBOL = 0,
    TAG_CTYPE = 1,
    TAG_STRUCT = 2,
    TAG_KEY = 4,
    TAG_KEY_TAG = 8,
    TAG_START_ARG = 16,
    TAG_END_ARG = 32,
    TAG_ENUM = 64,
    TAG_UNION = 128,
};

struct EvalChar{
    u64 hash;
    s8 string[128] = {};
    ParseTags tag;
};


//Why not just condense it to keyword REFL() ?

enum ParserKeyWord{
    PARSERKEYWORD_REFLSTRUCT = PHashString("REFLSTRUCT"),
    PARSERKEYWORD_REFLFUNC = PHashString("REFLFUNC"),
    PARSERKEYWORD_REFLENUM = PHashString("REFLENUM"),
    PARSERKEYWORD_REFLUNION = PHashString("REFLUNION"),
    PARSERKEYWORD_TAGFIELD = PHashString("TAGFIELD"),
};

logic IsParserKeyword(u64 hash){
    
    ParserKeyWord array[] = {
        PARSERKEYWORD_REFLSTRUCT,
        PARSERKEYWORD_REFLFUNC,
        PARSERKEYWORD_REFLENUM,
        PARSERKEYWORD_TAGFIELD,
    };
    
    for(u32 i = 0; i < _arraycount(array); i++){
        
        if(hash == array[i]){
            return true;
        }
    }
    
    return false;
}

void TagEvalBuffer(EvalChar* eval_buffer,u32 count){
    
    u32 mask = 0;
    
    logic to_tag_key_tag = false;
    
    for(u32 i = 0; i < count; i++){
        
        auto c = &eval_buffer[i];
        
        //        printf("%s",c->string);
        
        if(c->hash == PHashString("struct")){
            c->tag = TAG_STRUCT;
            mask |= TAG_STRUCT;
            
            //            printf("[struct] ");
        }
        
        else if(c->hash == PHashString("enum")){
            c->tag = TAG_ENUM;
            mask |= TAG_ENUM;
            
            //            printf("[struct] ");
        }
        
        else if(IsCType(c->hash)){
            c->tag = TAG_CTYPE;
            mask |= TAG_CTYPE;
            
            //            printf("[type] ");
        }
        
        else if(IsParserKeyword(c->hash)){
            c->tag = TAG_KEY;
            mask |= TAG_KEY;
            
            //            printf("[key] ");
        }
        
        else if(c->hash == PHashString("(")){
            c->tag = TAG_START_ARG;
            
            if((c - 1)->tag == TAG_KEY){
                to_tag_key_tag = true;
            }
        }
        
        else if(c->hash == PHashString(")")){
            c->tag = TAG_END_ARG;
            
            if(to_tag_key_tag){
                to_tag_key_tag = false;
            }
        }
        
        else if(to_tag_key_tag){
            c->tag = TAG_KEY_TAG;
            //            printf("[key_tag] ");
        }
        
        else{
            c->tag = TAG_SYMBOL;
            
            //            printf("[symbol] ");
        }
    }
    
    //    printf("\n\n");
    
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

void SkipBracketBlock(s8* buffer,u32* a){
    
    auto cur = *a;
    
    u32 k = 0;
    
    for(;;cur++){
        
        auto c = buffer[cur];
        
        if(c == '{'){
            k++;
        }
        
        if(c == '}'){
            k --;
        }
        
        if(!k){
            break;
        }
    }
    
    *a = cur;
}

struct GenericTypeDec{
    
    u32 tag_count = 0;
    u32 taghash_array[128];
    s8 tagstring_array[128][128];
    
    s8 type_string[128];
    s8 name_string[128];
    u64 name_hash;
    CType type;
};

struct GenericTypeDef : GenericTypeDec{
    
    u32 indir_count;
    u32 array_dim[128];
    u32 array_dim_count;
};

struct GenericStruct : GenericTypeDec{
    
    u32 members_count = 0;
    GenericTypeDef members_array[256];
    
};

logic FillEvalBuffer(s8* buffer,u32* a,EvalChar* evaluation_buffer,u32* k,s8 terminator){
    
    auto cur = *a;
    
    u32 evaluation_count = *k;
    
    u32 symbol_len = 0;
    s8 symbol_buffer[128] = {};
    
    logic ret = false;
    
    PGetSymbol(&symbol_buffer[0],buffer,&cur,&symbol_len);
    
    if(symbol_len){
        
        //                printf("%s\n",&symbol_buffer[0]);
        
        evaluation_buffer[evaluation_count] =
        {PHashString(&symbol_buffer[0])};
        memcpy(&evaluation_buffer[evaluation_count].string[0],&symbol_buffer[0],strlen(&symbol_buffer[0]));
        
        evaluation_count++;
    }
    
    if(buffer[cur] == '('){
        
        evaluation_buffer[evaluation_count] =
        {PHashString("("),"("};
        
        evaluation_count++;
    }
    
    if(buffer[cur] == ')'){
        
        evaluation_buffer[evaluation_count] =
        {PHashString(")"),")"};
        
        evaluation_count++;
        
    }
    
    if(buffer[cur] == '*'){
        
        evaluation_buffer[evaluation_count] =
        {PHashString("*"),"*"};
        
        evaluation_count++;
        
    }
    
    if(buffer[cur] == ';'){
        evaluation_count = 0;
    }
    
    if(buffer[cur] == terminator){
        
        TagEvalBuffer(&evaluation_buffer[0],evaluation_count);
        
        ret = true;
    }
    
    *k = evaluation_count;
    *a = cur;
    
    return ret;
}


//TODO: test this function
void GenerateGenericStruct(EvalChar* eval_buffer,u32 count,s8* buffer,u32* a,GenericStruct* struct_array,u32* struct_count){
    
    auto t = &struct_array[*struct_count];
    (*struct_count)++;
    
    //get general info
    
    for(u32 i = 0; i < count; i++){
        
        auto c = &eval_buffer[i];
        
        if(c->tag == TAG_KEY_TAG){
            
            memcpy(&t->tagstring_array[t->tag_count][0],&c->string[0],strlen(&c->string[0]));
            
            t->taghash_array[t->tag_count] = c->hash;
            
            t->tag_count++;
        }
        
        if(c->tag == TAG_SYMBOL){
            
            memcpy(&t->name_string[0],&c->string[0],strlen(&c->string[0]));
            t->name_hash = c->hash;
            
            t->type = CType_STRUCT;
            memcpy(&t->type_string[0],"struct",strlen("struct"));
        }
    }
    
    auto cur = *a;
    
    u32 k = 0;
    
    
    EvalChar fieldeval_array[256] = {};
    u32 fieldeval_count = 0;
    
    for(;;cur++){
        
        SanitizeString(buffer,&cur);
        
        auto c = buffer[cur];
        
        if(FillEvalBuffer(buffer,&cur,&fieldeval_array[0],&fieldeval_count,';')){
            
            //evaluated the buffer etc
            
            if(fieldeval_count){
                
                auto member = &t->members_array[t->members_count];
                
                memset(member,0,sizeof(GenericTypeDef));
                
                for(u32 i = 0; i < fieldeval_count; i++){
                    
                    auto x = fieldeval_array[i];
                    
                    if(x.tag == TAG_CTYPE){
                        member->type = (CType)x.hash;
                        
                        memcpy(&member->type_string[0],&x.string[0],strlen(&x.string[0]));
                    }
                    
                    if(x.tag == TAG_SYMBOL){
                        
                        member->name_hash = x.hash;
                        
                        memcpy(&member->name_string[0],&x.string[0],strlen(&x.string[0]));
                        
                    }
                    
                    if(x.tag == TAG_KEY_TAG){
                        
                    }
                    
                    //indirection
                    if(x.string[0] == '*'){
                        
                    }
                    
                    //is an array
                    if(PIsStringInt(&x.string[0])){
                        
                    }
                }
                
            }
            
            fieldeval_count = 0;
            continue;
        }
        
        if(c == '{'){
            
            if(fieldeval_count){
                
                TagEvalBuffer(&fieldeval_array[0],fieldeval_count);
                
                for(u32 i = 0; i < fieldeval_count; i++){
                    
                    auto x = fieldeval_array[i];
                    
                    if(x.tag == TAG_SYMBOL){
                        
                        //this is NOT  an anonymouse struct
                        
                        s8 buffer[512] = {};
                        
                        sprintf(&buffer[0],"%s::%s",&t->name_string[0],&x.string[0]);
                        
                        memcpy(&x.string[0],&buffer[0],strlen(&buffer[0]));
                        
                        GenerateGenericStruct(&fieldeval_array[0],fieldeval_count,buffer,&cur,struct_array,struct_count);
                        break;
                    }
                }
                
                fieldeval_count = 0;
            }
            
            k++;
        }
        
        if(c == '}'){
            
            k --;
        }
        
        if(!k){
            break;
        }
    }
    
    
    *a = cur;
}

//TODO: we should handle unions as well
s32 main(s32 argc,s8** argv){
    
    {
        
        u32 evaluation_count = 0;
        EvalChar evaluation_buffer[32] = {};
        
        auto string = R"FOO(
        
        #define REFLSTRUCT(TAG)
#define REFLFUNC(TAG)

/*
  hello world
*/

//THIS IS A TEST COMMENT

struct REFLCOMPONENT Character{
    s8* name;
    u32 health;
    u32 atk;
};

struct REFLSTRUCT(COMP) BossCharacter{
    Character character;//this is a char name
    u32 special_ability;/*asdasd*/
};

struct REFLSTRUCT(GENERIC) Foo{
    u32 a;
    u32 b;
    u32 c;
};

s32 REFLFUNC(GENERIC) AddFunction(u32 a,u32 b){

return a + b;
}

u32 REFLFUNC(GENERIC) FOO(u32* a){
    return *a;
}

void REFLFUNC(GENERIC) BAR(u32 a[5],u32 count){

for(u32 i = 0; i < count; i++){
a[i] += 1;
}

}


enum REFLENUM(GENERIC) ENUMT{
ENUMT_NONE,
ENUMT_ONE,
ENUMT_TWO,
ENUMT_THREE,
};


typedef u32 ObjectID;
typedef u32 LightID;
typedef u32 AnimationID;
typedef u32 ModelID;
typedef u32 TextureID;
typedef u32 RenderGroupIndex;
typedef u32 MaterialID;

//NOTE: we should not be able to edit core formats. this is because the engine depends not changing
struct REFLSTRUCT(COMP) EntityAnimationData{
    ObjectID id;
    AnimationID animdata_id;
    u16 animationindex;
    u16 islooping;
    f32 animationtime;
    f32 speed;
};

struct REFLSTRUCT(COMP) EntityDrawData{
    ObjectID id;
    ModelID model;
    MaterialID material;
    RenderGroupIndex group;
};

struct REFLSTRUCT(COMP) EntityAudioData{
    ObjectID id;
    AudioAssetHandle audioasset;
    u16 islooping = 0;
    u16 toremove = 0;
};

struct REFLSTRUCT(COMP) PointLight{
    ObjectID id;
    f32 R;
    f32 G;
    f32 B;
    
    f32 radius;
    f32 intensity;
};


// soft angle is the outer circle where it starts to drop off
//effective angle is hard_angle + soft_angle
struct REFLSTRUCT(COMP) SpotLight{
    ObjectID id;
    
    f32 R; // replace w color
    f32 G;
    f32 B;
    
    f32 dir_x;
    f32 dir_y;
    f32 dir_z;
    
    f32 full_angle;
    f32 hard_angle;
    f32 radius;
    
    f32 intensity;
};

)FOO";
        
        GenericStruct struct_array[1024] = {};
        u32 struct_count = 0;
        
        ptrsize size = strlen(string) + 1;
        u32 cur = 0;
        s8* buffer = (s8*)&string[0];
        
        
        //TODO: we should abstract this away (so we can recurse this)
        for(;;){
            
            SanitizeString(buffer,&cur);
            
            if(FillEvalBuffer(buffer,&cur,&evaluation_buffer[0],&evaluation_count,'{')){
                
                //start evaluating
                TagEvalBuffer(&evaluation_buffer[0],evaluation_count);
                
                if(IsReflStruct(&evaluation_buffer[0],evaluation_count)){
                    //parse the struct
                    
                    printf("found struct:");
                    
                    for(u32 i = 0; i < evaluation_count; i++){
                        printf("%s ",evaluation_buffer[i].string);
                    }
                    
                    printf("\n");
                    
#if 0
                    
                    GenerateGenericStruct(&evaluation_buffer[0],evaluation_count,buffer,&cur,&struct_array[0],&struct_count);
                    
                    _kill("examine struct",1);
                    
#endif
                }
                
                if(IsReflEnum(&evaluation_buffer[0],evaluation_count)){
                    
                    
                    //parse the enum
                    
                    printf("found enum:");
                    
                    for(u32 i = 0; i < evaluation_count; i++){
                        printf("%s ",evaluation_buffer[i].string);
                    }
                    
                    printf("\n");
                    
                    
                }
                
                if(IsReflFunc(&evaluation_buffer[0],evaluation_count)){
                    //parse the function
                    
                    printf("found function:");
                    
                    for(u32 i = 0; i < evaluation_count; i++){
                        printf("%s ",evaluation_buffer[i].string);
                    }
                    
                    printf("\n");
                }
                
                SkipBracketBlock(buffer,&cur);
                
                
                
                evaluation_count = 0;
            }
            
            cur++;
            
            if(cur >= size){
                break;
            }
        }
    }
    
    printf("\n");
    
    return 0;
}

#endif
