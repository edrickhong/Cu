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

#if 0

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

struct GenericTypeDec{
    
    s8 type_string[16];
    s8 name_string[32];
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
        
        else if(IsCType(c->hash)){
            c->tag = TAG_CTYPE;
            
            //            printf("[type] ");
        }
        
        else if(IsParserKeyword(c->hash)){
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
    
    if(buffer[cur] == '='){
        
        evaluation_buffer[evaluation_count] =
        {PHashString("="),"="};
        
        evaluation_count++;
    }
    
    if(buffer[cur] == '"'){
        
        for(;;){
            
            s8 t[2] = {buffer[cur],0};
            
            evaluation_buffer[evaluation_count] =
            {PHashString(&t[0]),buffer[cur]};
            evaluation_count++;
            
            //printf("%c",buffer[cur]);
            
            cur++;
            
            if(buffer[cur] == '"'){
                
                s8 t[2] = {buffer[cur],0};
                
                evaluation_buffer[evaluation_count] =
                {PHashString(&t[0]),buffer[cur]};
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
    
    
    //TODO: do we need this?
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

void _ainline InternalHandleStructFields(GenericStruct* t,GenericStruct* struct_array,u32* struct_count,EvalChar* membereval_array,u32 membereval_count,u32* cur){
    auto i = *cur;
    
    auto member = &t->members_array[t->members_count];
    t->members_count++;
    
    memset(member,0,sizeof(GenericTypeDef));
    
    logic is_assign = false;
    
    
    for(u32 j = 0; j < _arraycount(member->dim_array);j++){
        member->dim_array[j] = 1;
    }
    
    
    
    
    for(u32 j = 0; j < membereval_count;j++){
        
        
        auto prev_x = &membereval_array[j - 1];
        
        if(i == 0){
            prev_x = 0;
        }
        
        auto x = &membereval_array[j];
        
        //                    printf("::%s ",&x->string[0]);
        
        
        if(j == 0){
            
            //TODO: handle anonymous structs
            
            if(x->tag == TAG_SYMBOL){
                
                member->type = CType_STRUCT;
            }
            
            if(x->tag == TAG_CTYPE){
                member->type = (CType)x->hash;
            }
            
            memcpy(&member->type_string[0],&x->string[0],strlen(&x->string[0]));
            
        }
        
        else if(x->tag == TAG_SYMBOL){
            
            member->name_hash = x->hash;
            
            memcpy(&member->name_string[0],&x->string[0],strlen(&x->string[0]));
            
        }
        
        
        //indirection
        if(x->tag == TAG_INDIR){
            member->indir_count++;
        }
        
        if(x->tag == TAG_ASSIGN){
            
            if(member->type == CType_STRUCT){
                
                printf("WARNING: struct initialization ignored:");
                
                for(u32 k = 0; k < membereval_count; k++){
                    
                    printf("%s ",&membereval_array[k].string[0]);
                }
                
                printf("\n");
                
                break;
            }
            
            else{
                is_assign = true;
            }
            
        }
        
        
        /*
        TODO: I do not like how initialization is done rn
        How do we handle many dimensionds?
*/
        if(is_assign){
            
            _kill("too many default initializers\n",member->default_count >= _arraycount(member->default_array));
            
            auto hash = PHashString(member->type_string);
            
            if(x->tag == TAG_DOUBLE_QUOTE){
                
                s8 abc[256] = {};
                
                InternalBufferGetString(&member->default_string[0],&membereval_array[0],membereval_count,&j);
                
                member->default_count = (u8)-2;
            }
            
            if(PIsStringFloat(&x->string[0])){
                
                if(IsIntType(hash)){
                    
                    member->default_array[member->default_count] = atoi(x->string);
                    member->default_count++;
                }
                
                if(IsFloatType(hash)){
                    
                    member->defaultf_array[member->default_count] = atof(x->string);
                    member->default_count++;
                    
                }
                
            }
        }
        
        else{
            
            //FIXME: this is messy. deal explicitly w arrays
            if(PIsStringInt(&x->string[0])){
                
                _kill("too many dims\n",member->dim_array_count >= _arraycount(member->dim_array));
                
                member->dim_array[member->dim_array_count] = atoi(&x->string[0]);
                member->dim_array_count++;
                
                
            }
        }
        
        
        
        
    }
    
    if(is_assign && !member->default_count){
        member->default_count = (u8)-1;
    }
    
    
    
    if(member->type == CType_STRUCT){
        //Lookup
        
        GenericStruct* sptr = 0;
        
        s8 buffer[256] = {};
        sprintf(buffer,"%s___%s",t->name_string,member->type_string);
        
        auto struct_hash = PHashString(member->type_string);
        auto sub_struct_hash = PHashString(buffer);
        
        for(u32 j = 0; j < (*struct_count);j++){
            
            auto s = &struct_array[j];
            
            if(s->name_hash == struct_hash || s->name_hash == sub_struct_hash){
                sptr = s;
                break;
            }
        }
        
        if(sptr){
            
            for(u32 j = 0; j < sptr->members_count;j++){
                
                auto c_m = &t->members_array[t->members_count];
                t->members_count++;
                
                auto m = &sptr->members_array[j];
                
                memcpy(c_m,m,sizeof(GenericTypeDef));
                
                s8 tbuffer[1024] = {};
                
                sprintf(&tbuffer[0],"%s::%s",member->name_string,m->name_string);
                
                memcpy(c_m->name_string,tbuffer,strlen(tbuffer));
                
            }
            
            
        }
        
        
    }
    
    *cur = i;
}


//TODO: test this function
void GenerateGenericStruct(EvalChar* eval_buffer,u32 count,s8* buffer,u32* a,GenericStruct* struct_array,u32* struct_count,const s8* parent_name = 0){
    
    auto t = &struct_array[*struct_count];
    (*struct_count)++;
    
    t->pkey = PARSERKEYWORD_REFL;
    
    //Struct info
    for(u32 i = 0; i < count; i++){
        
        auto c = &eval_buffer[i];
        
        if(c->tag == TAG_SYMBOL){
            
            s8 name_buffer[256] = {};
            
            if(parent_name){
                sprintf(&name_buffer[0],"%s___%s",parent_name,&c->string[0]);
            }
            
            else{
                memcpy(&name_buffer[0],&c->string[0],strlen(&c->string[0]));
            }
            
            
            
            memcpy(&t->name_string[0],&name_buffer[0],strlen(&name_buffer[0]));
            
            t->name_hash = PHashString(name_buffer);
            
            t->type = CType_STRUCT;
            memcpy(&t->type_string[0],"struct",strlen("struct"));
        }
        
        if(c->tag == TAG_KEY){
            t->pkey = (ParserKeyWord)c->hash;
        }
    }
    
    s8 scope_buffer[1024 * 4] = {};
    
    ExtractScope(&scope_buffer[0],buffer,a);
    
    EvalChar membereval_array[256] = {};
    u32 membereval_count = 0;
    
    for(u32 i = 0;;i++){
        
        SanitizeString(&scope_buffer[0],&i);
        
        auto c = scope_buffer[i];
        
        s8 terminator_array[] = {';','{'};
        
        if(FillEvalBuffer(scope_buffer,&i,&membereval_array[0],&membereval_count,&terminator_array[0],_arraycount(terminator_array))){
            
            if(membereval_count){
                
                
                //Handle internal structs
                if(membereval_array[0].tag == TAG_STRUCT){
                    
                    logic named_struct = false;
                    
                    for(u32 k = 0; k < membereval_count;k++){
                        
                        auto d = &membereval_array[k];
                        
                        if(d->tag == TAG_SYMBOL){
                            named_struct = true;
                            break;
                        }
                    }
                    
                    if(named_struct){
                        
                        GenerateGenericStruct(&membereval_array[0],membereval_count,scope_buffer,&i,struct_array,struct_count,&t->name_string[0]);
                    }
                }
                
                else{
                    
                    InternalHandleStructFields(t,struct_array,struct_count,membereval_array,membereval_count,&i);
                    
                }
                
                membereval_count = 0;
            }
        }
        
        if(!c){
            break;
        }
    }
    
    
    //DebugPrintGenericStruct(t);
    
}


#define _testing 1

#if 1

#define FWrite(file,buffer,len) printf("%s",buffer);

#endif

void WriteComponentMetaData(const s8* file_string,GenericStruct* struct_array,u32 struct_count){
    
#if !(_testing)
    
    auto file = FOpenFile(file_string,F_FLAG_READWRITE |
                          F_FLAG_TRUNCATE | F_FLAG_CREATE);
    
#endif
    
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
    
    
    {
        FWrite(file,(void*)initialcontents,strlen(initialcontents));
        
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
    
    for(u32 i = 0; i < struct_count; i++){
        
        auto s = &struct_array[i];
        
        {
            
            s8 buffer[256] = {};
            
            sprintf(buffer,"\n\n_persist MetaDataEntry %s_METACOMP_STRUCT[] = {\n",s->name_string);
            
            FWrite(file,(void*)buffer,strlen(buffer));
            
        }
        
        {
            
            
            for(u32 j = 0; j < s->members_count;j++){
                
                auto m = &s->members_array[j];
                
                s8 buffer[256] = {};
                
                //TODO: support more than 1d arrays
                //Remove ref_metadatacomp_index
                sprintf(buffer,"{%d,%d,\"%s\",\"%s\",sizeof(%s),offsetof(%s,%s),%d,(u32)-1},\n"
                        ,(u32)PHashString(m->type_string),(u32)m->name_hash,m->type_string,m->name_string,m->type_string,s->name_string,m->name_string,m->dim_array[0]);
                
                FWrite(file,(void*)buffer,strlen(buffer));
            }
            
            
            auto end_struct = "};";
            FWrite(file,(void*)end_struct,strlen(end_struct));
            
        }
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
                    "{offsetof(ComponentStruct,%s_array),offsetof(ComponentStruct,%s_count),sizeof(ComponentStruct::%s_array[0]),\"%s\",(u32)%d,&%s_METACOMP_STRUCT[0],_arraycount(%s_METACOMP_STRUCT)},\n",
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
    
#if !(_testing)
    
    FCloseFile(file);
    
#endif
}

//TODO: we should handle unions as well
s32 main(s32 argc,s8** argv){
    
    auto string = R"FOO(
    
    #define REFLCOMPONENT
#define REFL

/*
hello world
*/

//THIS IS A TEST COMMENT

struct REFLCOMPONENT Character{
s8* name = "hello world";
u32 health;
u32 atk;
};

//MARK: we will not handle struct

    struct REFLCOMPONENT BossCharacter{
            Character character = {};
            u32 special_ability;
        };
        
        struct REFLCOMPONENT SAILOR{
        
        struct HELLO{
        u32 a;
        u32 b;
};

HELLO s;
f32 k;

        };
        
        struct REFL ABC{
        
        struct DEF{
        
        u32 d;
        u32 e;
        u32 f;
        
};

u32 a;
        u32 b;
        u32 c;
        
        DEF k;
        
        struct{
        u32 g;
        u32 h;
        u32 i;
};

};


//FIXME: we are skipping Foo
struct REFL Foo{
u32 a;
u32 b;
u32 c;
};

struct REFL Bar{
u32* a = 0;
u32 b[4] = {};

//TODO: handle this case
//f32 c[4] = {};
};

s32 REFL AddFunction(u32 a,u32 b){

return a + b;
}

u32 REFL K(u32* a){
return *a;
}

void REFL BAR(u32 a[5],u32 count){

for(u32 i = 0; i < count; i++){
a[i] += 1;
}

}


enum REFL ENUMT{
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
struct REFLCOMPONENT EntityAnimationData{
ObjectID id;
AnimationID animdata_id;
u16 animationindex;
u16 islooping;
f32 animationtime;
f32 speed;
};

struct REFLCOMPONENT EntityDrawData{
ObjectID id;
ModelID model;
MaterialID material;
RenderGroupIndex group;
};

struct REFLCOMPONENT EntityAudioData{
ObjectID id;
AudioAssetHandle audioasset;
u16 islooping = 0;
u16 toremove = 0;
};

struct REFLCOMPONENT PointLight{
ObjectID id;
f32 R;
f32 G;
f32 B;

f32 radius;
f32 intensity;
};


// soft angle is the outer circle where it starts to drop off
//effective angle is hard_angle + soft_angle
struct REFLCOMPONENT SpotLight{
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
    
    u32 evaluation_count = 0;
    EvalChar evaluation_buffer[32] = {};
    
    auto struct_array = (GenericStruct*)alloc(sizeof(GenericStruct) * 1024);
    u32 struct_count = 0;
    
    ptrsize size = strlen(string) + 1;
    u32 cur = 0;
    s8* buffer = (s8*)&string[0];
    
    
    //TODO: we should abstract this away (so we can recurse this)
    for(;;){
        
        SanitizeString(buffer,&cur);
        
        if(FillEvalBuffer(buffer,&cur,&evaluation_buffer[0],&evaluation_count,'{')){
            
            //start evaluating
            
            if(IsReflStruct(&evaluation_buffer[0],evaluation_count)){
                
#if 0
                
                
                printf("found struct:");
                
                for(u32 i = 0; i < evaluation_count; i++){
                    printf("%s ",evaluation_buffer[i].string);
                }
                
                printf("\n");
                
#endif
                
                GenerateGenericStruct(&evaluation_buffer[0],evaluation_count,buffer,&cur,&struct_array[0],&struct_count);
                
            }
            
            if(IsReflEnum(&evaluation_buffer[0],evaluation_count)){
                
                
                //parse the enum
                
                //                printf("found enum:");
                //                
                //                for(u32 i = 0; i < evaluation_count; i++){
                //                    printf("%s ",evaluation_buffer[i].string);
                //                }
                //                
                //                printf("\n");
                
                
            }
            
            if(IsReflFunc(&evaluation_buffer[0],evaluation_count)){
                //parse the function
                
                //                printf("found function:");
                //                
                //                for(u32 i = 0; i < evaluation_count; i++){
                //                    printf("%s ",evaluation_buffer[i].string);
                //                }
                //                
                //                printf("\n");
            }
            
            SkipBracketBlock(buffer,&cur);
            
            
            
            evaluation_count = 0;
        }
        
        cur++;
        
        if(cur >= size){
            break;
        }
    }
    
    {
        
        qsort(struct_array,struct_count,
              sizeof(GenericStruct),
              [](const void * a, const void* b)->s32 {
              
              auto block_a = (GenericStruct*)a;
              auto block_b = (GenericStruct*)b;
              
              return block_a->pkey - block_b->pkey;
              });
        
        u32 count = 0;
        u32 k = 0;
        
        for(u32 i = 0; i < struct_count;i++){
            
            if(struct_array[i].pkey == PARSERKEYWORD_COMPONENT){
                count = struct_count - i;
                k = i;
                break;
            }
        }
        
        WriteComponentMetaData("test.txt",&struct_array[k],count);
        
    }
    
    unalloc(struct_array);
    
    
    
    return 0;
}

#endif
