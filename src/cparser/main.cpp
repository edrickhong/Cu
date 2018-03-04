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
    
    CType_STRUCT,
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
            
            sprintf(writebuffer,"{(u32)%d,(u32)%d,\"%s\",\"%s\",sizeof(%s),offsetof(%s,%s) + %s,(u32)%d},\n",
                    (u32)PHashString(typename_buffer),(u32)PHashString(name_buffer),
                    typename_buffer,name_buffer,
                    typename_buffer,
                    curstruct->name,entry->name,offset_string,ref_id);
        }
        
        else{
            sprintf(writebuffer,"{(u32)%d,(u32)%d,\"%s\",\"%s\",sizeof(%s),offsetof(%s,%s),(u32)%d},\n",
                    (u32)PHashString(typename_buffer),(u32)PHashString(name_buffer),
                    typename_buffer,name_buffer,
                    typename_buffer,curstruct->name,entry->name,ref_id); 
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
  
  
  logic MetaGetValueByNameHash(void* obj,void* outdata,u32 hash,MetaDataEntry* array,
  u32 array_count){
  
  for(u32 i = 0; i < array_count; i++){
  MetaDataEntry* entry = &array[i];
  if(entry->name_hash == hash){
  auto data = (s8*)obj;
  memcpy(outdata,data + entry->offset,entry->size);
  return true;
  }
  }
  
  return false;
  }
  
  logic MetaGetValueByName(void* obj,void* outdata,const s8* name,MetaDataEntry* array,
  u32 array_count){
  
  return MetaGetValueByNameHash(obj,outdata,PHashString(name),array,array_count);
  }
  
  logic MetaSetValueByNameHash(void* obj,void* value,u32 hash,MetaDataEntry* array,
  u32 array_count){
  
  for(u32 i = 0; i < array_count; i++){
  MetaDataEntry* entry = &array[i];
  if(entry->name_hash == hash){
  auto data = (s8*)obj;
  memcpy(data + entry->offset,value,entry->size);
  return true;
  }
  }
  
  return false;
  }
  
  logic MetaSetValueByName(void* obj,void* value,const s8* name,MetaDataEntry* array,
  u32 array_count){
  
  return MetaSetValueByNameHash(obj,value,PHashString(name),array,array_count);
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
   
   s8 buffer[256] = {};
   
   MetaGetValueByName(comp_entry_data,&buffer[0],comp_meta_entry.name_string,
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
