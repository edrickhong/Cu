
#include "main.h"

#define _autoexpand_structs 1

/*
  TODO: 
  handle inheritance and enums.
  ptr inspection can be implemented in assembly
*/

//TODO: struct union and enum have the same parse structure (should we condense here?)

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
    
    
    //MARK: Should we auto expand structs?
    
#if _autoexpand_structs
    
    if(member->type == CType_STRUCT && !member->dim_array_count){
        //Lookup
        
        GenericStruct* sptr = 0;
        
        s8 buffer[256] = {};
        sprintf(buffer,"%s__%s",t->name_string,member->type_string);
        
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
    
#endif
    
    *cur = i;
}

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
                sprintf(&name_buffer[0],"%s__%s",parent_name,&c->string[0]);
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


void InternalParseSource(s8* buffer,u32 size,GenericStruct* struct_array,u32* struct_count){
    
    u32 evaluation_count = 0;
    EvalChar evaluation_buffer[32] = {};
    
    u32 cur = 0;
    
    
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
                
                GenerateGenericStruct(&evaluation_buffer[0],evaluation_count,buffer,&cur,&struct_array[0],struct_count);
                
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
}

//TODO: we should handle unions as well
s32 main(s32 argc,s8** argv){
    
    for(s32 i = 1; i < argc; i++){
        
        if(PHashString(argv[i]) == PHashString("-help") || argc == 1){
            printf("Format:cparser src1 src2 src3 -component componentfile -meta metafile\n");
            return 0;
        }
        
    }
    
    s8** source_array = 0;
    u32 source_count = 0;
    
    s8* componentfile = 0;
    s8* metafile = 0;
    
    GetArgsData(argv,argc,&source_array,&source_count,&componentfile,&metafile);
    
    if(source_count == (u32)-1){
        printf("Error: Commandline args format is wrong\n");
        return 0;
    }
    
    if(!source_count){
        printf("Error: No source files specified\n");
        return 0;
    }
    
    if(!componentfile && !metafile){
        printf("Error: No output files specified\n");
        return 0;
    }
    
    if(!metafile){
        printf("Error: No metafile file specified\n");
        return 0;
    }
    
    
#if 0
    {
        
        for(u32 i = 0; i < source_count;i++){
            printf("source:%s\n",source_array[i]);
        }
        
        printf("component file:%s\n",componentfile);
        printf("meta file:%s\n",metafile);
        
        return 0;
    }
#endif
    
    auto struct_array = (GenericStruct*)alloc(sizeof(GenericStruct) * 1024);
    u32 struct_count = 0;

    memset(struct_array,0,sizeof(GenericStruct) * 1024);
    
    for(u32 i = 0; i < source_count;i++){
        
        auto file = FOpenFile(source_array[i],F_FLAG_READONLY);
        
        ptrsize size;
        auto buffer = FReadFileToBuffer(file,&size);
        
        InternalParseSource(buffer,size,struct_array,&struct_count);
        
        FCloseFile(file);
        unalloc(buffer);
        
    }
    
    
    
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
    
    WriteMetaFile(metafile,struct_array,struct_count);
    
    WriteComponentMetaData(componentfile,&struct_array[k],count,metafile);
    
    unalloc(struct_array);
    
    
    
    return 0;
}
