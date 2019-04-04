#include "main.h"

/*

NOTE:
if arraycount is 0, it is treated as 1d array w 1 element
we treat enums the same way we treat structs

ptr inspection can be implemented in assembly

TODO:
no automatic way to know if a given struct is nested or not and who the parent of the nested struct is
*/

void TagEvalBuffer(EvalChar* eval_buffer,u32 count){
    
    for(u32 i = 0; i < count; i++){
        
        auto c = &eval_buffer[i];
        
        if(c->hash == PHashString("struct") ||c->hash == PHashString("union")){
            c->tag = TAG_STRUCT;
        }
        
        else if(c->hash == PHashString("enum")){
            c->tag = TAG_ENUM;
        }
        
        else if(IsCType(c->hash)){
            c->tag = TAG_CTYPE;
        }
        
        else if(IsParserKeyword(c->hash)){
            c->tag = TAG_KEY;
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
        
        else if(c->hash == PHashString(":")){
            c->tag = TAG_COLON;
        }
        
        else if(c->hash == PHashString("/")){
            c->tag = TAG_DIV;
        }
        
        else if(c->hash == PHashString("+")){
            c->tag = TAG_ADD;
        }
        
        else if(c->hash == PHashString("-")){
            c->tag = TAG_SUB;
        }
        
        else if(c->hash == PHashString("[")){
            c->tag = TAG_START_SQUARE;
        }
        
        else if(c->hash == PHashString("]")){
            c->tag = TAG_END_SQUARE;
        }
        
        else if(c->hash == PHashString(",")){
            c->tag = TAG_COMMA;
        }
        
        else if(PIsStringFloat(c->string) || PIsStringInt(c->string)){
            c->tag = TAG_VALUE;
        }
        
        else{
            c->tag = TAG_SYMBOL;
        }
    }
    
}

m64 Eval_EvalCharMath(EvalChar* eval_array,u32 eval_count){
    
    OpChar char_array[32] = {};
    u32 char_count = 0;
    
    for(u32 i = 0; i < eval_count; i++){
        
        auto e = &eval_array[i];
        
        OpCharType type = OpChar_UNKNOWN;
        
        switch(e->tag){
            
            case TAG_MUL:{
                type = OpChar_MUL;
            }break;
            
            case TAG_DIV:{
                type = OpChar_DIV;
            }break;
            
            case TAG_ADD:{
                type = OpChar_ADD;
            }break;
            
            case TAG_SUB:{
                type = OpChar_SUB;
            }break;
            
            
            case TAG_VALUE:{
                type = OpChar_VALUE;
            }break;
            
        }
        
        char_array[char_count].type = type;
        memcpy(char_array[char_count].string,e->string,strlen(e->string));
        
        char_count++;
        
    }
    
    return PEvaluateMathString(char_array,char_count);
}


void PrintEvalChar(EvalChar* eval_array,u32 eval_count){
    
    printf("PRINT EVAL:: ");
    
    for(u32 i = 0; i < eval_count; i++){
        
        printf("%s ",eval_array[i].string);
        
    }
    
    printf("\n\n");
    
}



_intern void _ainline HandleStructFields(GenericStruct* t,GenericStruct* struct_array,u32* struct_count,EvalChar* membereval_array,u32 membereval_count){
    
    auto member = &t->members_array[t->members_count];
    t->members_count++;
    
    b32 is_assign = false;
    b32 next_is_struct = false;
    
    for(u32 j = 0; j < membereval_count;j++){
        
        auto x = &membereval_array[j];
        
        if(x->tag == TAG_ASSIGN){
            
            is_assign = true;
            
        }
        
        
        if(is_assign){
            return;
        }
        
        
        if(j == 0 || next_is_struct){
            
            next_is_struct = x->tag == TAG_STRUCT || x->tag == TAG_ENUM || x->tag == TAG_UNION;
            
            
            if(x->tag == TAG_SYMBOL){
                
                member->type = CType_STRUCT;
            }
            
            if(x->tag == TAG_CTYPE){
                member->type = (CType)x->hash;
            }
            
            if(!next_is_struct){
                memcpy(&member->type_string[0],&x->string[0],strlen(&x->string[0]));
            }
            
            
            
        }
        
        else if(x->tag == TAG_SYMBOL){
            
            member->name_hash = x->hash;
            
            memcpy(&member->name_string[0],&x->string[0],strlen(&x->string[0]));
            
        }
        
        if(x->tag == TAG_START_SQUARE){
            
            j++;
            
            
            EvalChar eval_array[32] = {};
            u32 eval_count = 0;
            
            for(;j < membereval_count;j++){
                
                auto k = &membereval_array[j];
                
                if(k->tag == TAG_END_SQUARE){
                    break;
                }
                
                eval_array[eval_count] = *k;
                eval_count++;
            }
            
            auto value = Eval_EvalCharMath(eval_array,eval_count);
            
            member->dim_array[member->dim_count] = (u32)value.u;
            member->dim_count++;
            
        }
        
        
        //indirection
        if(x->tag == TAG_INDIR){
            member->indir_count++;
        }
        
        
        
        
    }
}

b32 IsDuplicateStruct(GenericStruct* struct_array,u32 struct_count,const s8* name){
    
    auto name_hash = PHashString(name);
    
    for(u32 i = 0; i < struct_count; i++){
        
        if(name_hash == struct_array[i].name_hash){
            return true;
        }
        
    }
    
    
    return false;
}

b32 IsDuplicateEnum(GenericEnum* enum_array,u32 enum_count,const s8* name){
    
    auto name_hash = PHashString(name);
    
    for(u32 i = 0; i < enum_count; i++){
        
        if(name_hash == enum_array[i].name_hash){
            return true;
        }
        
    }
    
    
    return false;
}

b32 IsDuplicateFunction(GenericFunction* function_array,u32 function_count,const s8* name){
    
    auto name_hash = PHashString(name);
    
    for(u32 i = 0; i < function_count; i++){
        
        if(name_hash == function_array[i].name_hash){
            return true;
        }
        
    }
    
    
    return false;
}

void DebugPrintGenericFunction(GenericFunction* f){
    
    printf("function %s %s\n",f->ret.type_string,f->ret.name_string);
    
    for(u32 i = 0; i < f->args_count; i++){
        
        printf("args:%s %s\n",f->args_array[i].type_string,f->args_array[i].name_string);
    }
}


_intern void HandleArgs(GenericFunction* f,u32* function_count, EvalChar* eval_buffer,u32 cur,u32 dst){
    
    
    u32 int_count = 0;
    u32 float_count = 0;
    
    EvalChar* start = &eval_buffer[cur];
    
    
    for(;cur < dst; cur++){
        
        auto end = &eval_buffer[cur];
        
        
        if(end->tag == TAG_COMMA || end->tag == TAG_END_ARG){
            
            CType ctype = CType_UNKNOWN;
            EvalChar* name = 0;
            EvalChar* type = 0;
            u32 indir_count = 0;
            
            for(;start <= end; start++){
                
                if(start->tag == TAG_CTYPE){
                    
                    ctype = (CType)start->hash;
                    type = start;
                }
                
                if(start->tag == TAG_SYMBOL){
                    
                    if(ctype == CType_UNKNOWN){
                        
                        ctype = CType_STRUCT;
                        type = start;
                    }
                    
                    else{
                        name = start;
                    }
                }
                
                if(start->tag == TAG_INDIR){
                    indir_count++;
                }
                
            }
            
            
            //TODO: handle struct types (structs cannot be opaque)
            if(ctype == CType_STRUCT){
                
                printf("Error: We do not allow struct types\n");
                
                memset(f,0,sizeof(GenericFunction));
                (*function_count)--;
                return;
            }
            
            if(IsIntType(ctype)){
                int_count++;
                
                if(int_count > 4){
                    
                    printf("Error: Int arguments exceed the limit of 4\n");
                    
                    memset(f,0,sizeof(GenericFunction));
                    (*function_count)--;
                    return;
                }
            }
            
            if(IsFloatType(ctype)){
                float_count++;
                
                if(float_count > 4){
                    
                    printf("Error: Float arguments exceed the limit of 4\n");
                    
                    memset(f,0,sizeof(GenericFunction));
                    (*function_count)--;
                    return;
                }
            }
            
            if (float_count + int_count > 4) {
                printf("Error: Total arguments exceed the limit of 4\n");
                
                memset(f, 0, sizeof(GenericFunction));
                (*function_count)--;
                return;
            }
            
            
            auto arg = &f->args_array[f->args_count];
            f->args_count++;
            
            memcpy(arg->type_string,type->string,strlen(type->string));
            arg->type = (CType)(type->hash);
            
            memcpy(arg->name_string,name->string,strlen(name->string));
            arg->name_hash = PHashString(name->string);
            
            arg->indir_count = indir_count;
            
        }
        
    }
    
}


//MARK: arrays should be passed by ptr. pls check
void GenerateGenericFunction(EvalChar* eval_buffer,u32 count,s8* buffer,ptrsize* a,GenericFunction* function_array,u32* function_count){
    
    
    //TODO: treat arrays as pointers
    for(u32 i = 0; i < count; i++){
        
        auto c = &eval_buffer[i];
        
        if(c->tag == TAG_START_SQUARE || c->tag == TAG_END_SQUARE){
            
            printf("Error Functions cannot have arrays\n");
            
            return;
        }
    }
    
    
    auto f = &function_array[(*function_count)];
    (*function_count)++;
    
    for(u32 i = 0; i < count; i++){
        
        auto c = &eval_buffer[i];
        
        
        
        if(i == 0){
            
            memcpy(f->ret.type_string,c->string,strlen(c->string));
            f->ret.type = (CType)c->hash;
            
        }
        
        if(c->tag == TAG_SYMBOL){
            
            if(IsDuplicateFunction(function_array,*function_count,c->string)){
                
                memset(f,0,sizeof(GenericFunction));
                (*function_count)--;
                return;
            }
            
            memcpy(f->name_string,c->string,strlen(c->string));
            f->name_hash = c->hash;
        }
        
        
        if(c->tag == TAG_START_ARG){
            
            u32 cur = i + 1;
            u32 dst = 0;
            
            for(u32 j = i;;j++){
                
                auto c = &eval_buffer[j];
                
                if(c->tag == TAG_END_ARG){
                    dst = j + 1;
                    break;
                }
                
            }
            
            
            HandleArgs(f,function_count,eval_buffer,cur,dst);
            
            return;
            
        }
        
        
        
        
    }
    
    
    
    //DebugPrintGenericFunction(f);
}

void GenerateGenericEnum(EvalChar* eval_buffer,u32 count,s8* buffer,ptrsize* a,GenericEnum* enum_array,u32* enum_count,const s8* parent_name = 0){
    
    s8 name_buffer[256] = {};
    
    if(ValidateAndFillNameBufferStruct(eval_buffer,count,parent_name,name_buffer)){
        printf("WARNING: Skipped %s. \"__\" is not allowed\n",name_buffer);
        return;
    }
    
    if(IsDuplicateEnum(enum_array,*enum_count,name_buffer)){
        
        return;
    }
    
    auto e = &enum_array[(*enum_count)];
    (*enum_count)++;
    
    e->name_hash = PHashString(name_buffer);
    memcpy(e->name_string,name_buffer,strlen(name_buffer));
    
    s8 scope_buffer[1024 * 4] = {};
    
    PExtractScopeC(&scope_buffer[0],buffer,a);
    
    EvalChar membereval_array[256] = {};
    u32 membereval_count = 0;
    
    for(ptrsize i = 0;;i++){
        
        PSanitizeStringC(&scope_buffer[0],&i);
        
        auto c = scope_buffer[i];
        
        s8 terminator_array[] = {',','}'};
        
        if(PFillEvalBufferC(scope_buffer,&i,&membereval_array[0],&membereval_count,terminator_array,_arraycount(terminator_array),TagEvalBuffer)){
            
            if(membereval_count){
                
                auto k = &membereval_array[0];
                
                _kill("Invalid enum\n",k->tag != TAG_SYMBOL);
                
                auto m = &e->members_array[e->members_count];
                e->members_count++;
                
                m->name_hash = k->hash;
                memcpy(m->name_string,k->string,strlen(k->string));
                
                membereval_count = 0;
            }
        }
        
        if(!c){
            break;
        }
    }
    
    //DebugPrintGenericEnum(e);
}

GenericStruct* GetHParent(EvalChar* eval_buffer,u32 count,GenericStruct* struct_array,u32 struct_count){
    
    b32 has_parent = false;
    
    for(u32 i = 0; i < count; i++){
        
        auto e = &eval_buffer[i];
        
        if(has_parent && e->tag == TAG_SYMBOL){
            
            GenericStruct* s = 0;
            
            for(u32 j = 0; j < struct_count; j++){
                
                if(struct_array[j].name_hash == e->hash){
                    s = &struct_array[j];
                }
            }
            
            _kill("HParent not found\n",!s);
            
            return s;
        }
        
        if(e->tag == TAG_COLON){
            has_parent = true;
        }
        
    }
    
    return 0;
}

/*
NOTE: parent_name refers to the parent struct scope that this scope is declared in

HParent is the heirarchical parent 
*/
void GenerateGenericStruct(EvalChar* eval_buffer,u32 count,s8* buffer,ptrsize* a,GenericStruct* struct_array,u32* struct_count,GenericEnum* enum_array,u32* enum_count,REFLSTRUCTTYPE type,const s8* parent_name = 0){
    
    s8 scope_buffer[1024 * 4] = {};
    PExtractScopeC(&scope_buffer[0],buffer,a);
    
    
    s8 name_buffer[256] = {};
    ParserKeyWord pkey = PARSERKEYWORD_REFL;
    
    //For typedefs, we only take the first name
    if(type == REFLSTRUCTTYPE_TYPEDEF_STRUCT){
        
        
        EvalChar e_array[256] = {};
        u32 e_count = 0;
        
        for(;;(*a)++){
            
            PSanitizeStringC(buffer,a);
            
            auto c = buffer[(*a)];
            
            s8 terminator_array[] = {';','{'};
            
            if(PFillEvalBufferC(buffer,a,&e_array[0],&e_count,&terminator_array[0],_arraycount(terminator_array),TagEvalBuffer)){
                
                for(u32 i = 0; i < e_count; i++){
                    
                    auto e = &e_array[i];
                    
                    if(e->tag == TAG_SYMBOL){
                        
                        memcpy(name_buffer,e->string,strlen(e->string));
                    }
                    
                    
                }
                
                break;
            }
            
        }
        
        
        if(ValidateNameBufferStruct(eval_buffer,count,parent_name,name_buffer,&pkey)){
            printf("WARNING: Skipped %s. \"__\" is not allowed\n",name_buffer);
            return;
        }
        
    }
    
    
    else if(ValidateAndFillNameBufferStruct(eval_buffer,count,parent_name,name_buffer,&pkey)){
        printf("WARNING: Skipped %s. \"__\" is not allowed\n",name_buffer);
        return;
    }
    
    if(IsDuplicateStruct(struct_array,*struct_count,name_buffer)){
        return;
    }
    
    
    //Struct info
    
    auto t = &struct_array[*struct_count];
    (*struct_count)++;
    
    memcpy(&t->name_string[0],&name_buffer[0],strlen(&name_buffer[0]));
    t->name_hash = PHashString(name_buffer);
    
    t->type = CType_STRUCT;
    memcpy(&t->type_string[0],"struct",strlen("struct"));
    
    t->pkey = pkey;
    
    
    //handles inheritance
    {
        auto parent = GetHParent(eval_buffer,count,struct_array,*struct_count);
        
        if(parent){
            
            for(u32 i = 0; i < parent->members_count; i++){
                
                t->members_array[t->members_count] = parent->members_array[i];
                
                t->members_count++;
                
            }
        }
    }
    
    EvalChar membereval_array[256] = {};
    u32 membereval_count = 0;
    
    for(ptrsize i = 0;;i++){
        
        PSanitizeStringC(&scope_buffer[0],&i);
        
        auto c = scope_buffer[i];
        
        s8 terminator_array[] = {';','{'};
        
        if(auto terminator = PFillEvalBufferC(scope_buffer,&i,&membereval_array[0],&membereval_count,&terminator_array[0],_arraycount(terminator_array),TagEvalBuffer)){
            
            if(membereval_count){
                
                
                //Handle internal structs
                if(terminator == '{'&& (membereval_array[0].tag == TAG_STRUCT || membereval_array[0].tag == TAG_ENUM)){
                    
                    b32 named_struct = false;
                    
                    for(u32 k = 0; k < membereval_count;k++){
                        
                        auto d = &membereval_array[k];
                        
                        if(d->tag == TAG_SYMBOL){
                            named_struct = true;
                            break;
                        }
                    }
                    
                    if(named_struct){
                        
                        if(membereval_array[0].tag == TAG_STRUCT){
                            
                            GenerateGenericStruct(&membereval_array[0],membereval_count,scope_buffer,&i,struct_array,struct_count,enum_array,enum_count,type,&t->name_string[0]);
                            
                        }
                        
                        if(membereval_array[0].tag == TAG_ENUM){
                            
                            GenerateGenericEnum(&membereval_array[0],membereval_count,scope_buffer,&i,enum_array,enum_count,&t->name_string[0]);
                        }
                    }
                }
                
                else{
                    
                    HandleStructFields(t,struct_array,struct_count,membereval_array,membereval_count);
                    
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

_intern void ParseSource(s8* buffer,u32 size,GenericStruct* struct_array,u32* struct_count,GenericEnum* enum_array,u32* enum_count,GenericFunction* function_array,u32* function_count){
    
    u32 evaluation_count = 0;
    EvalChar evaluation_buffer[256] = {};
    
    ptrsize cur = 0;
    
    
    for(;;){
        
        PSanitizeStringC(buffer,&cur);
        
        if(PFillEvalBufferC(buffer,&cur,&evaluation_buffer[0],&evaluation_count,'{',TagEvalBuffer)){
            
            //start evaluating
            
            
            
            
            {
                auto type = IsReflStruct(&evaluation_buffer[0],evaluation_count);
                
                
                if(type){
                    
                    GenerateGenericStruct(&evaluation_buffer[0],evaluation_count,buffer,&cur,&struct_array[0],struct_count,enum_array,enum_count,type);
                }
            }
            
            if(IsReflEnum(&evaluation_buffer[0],evaluation_count)){
                
                
                GenerateGenericEnum(&evaluation_buffer[0],evaluation_count,buffer,&cur,enum_array,enum_count);
                
                
            }
            
            if(IsReflFunc(&evaluation_buffer[0],evaluation_count)){
                
                //parse the function
                
#if 0
                
                printf("found function:");
                
                for(u32 i = 0; i < evaluation_count; i++){
                    printf("%s ",evaluation_buffer[i].string);
                }
                
                printf("\n");
                
#endif
                
                GenerateGenericFunction(&evaluation_buffer[0],evaluation_count,buffer,&cur,function_array,function_count);
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


s32 main(s32 argc,s8** argv){
    
    
    argv++;
    argc--;
    
#ifdef DEBUG
    
    TimeSpec start;
    
    GetTime(&start);
    
#endif
    
    for(s32 i = 0; i < argc; i++){
        
        if(PHashString(argv[i]) == PHashString("-help") || argc == 0){
            printf("Format:cparser src1 src2 src3 -component componentfile -meta metafile\n");
            return -1;
        }
        
    }
    
    s8** source_array = 0;
    u32 source_count = 0;
    
    s8* component_src = 0;
    s8* meta_src = 0;
    
    s8* component_h = 0;
    s8* meta_h = 0;
    
    GetArgsData(argv,argc,&source_array,&source_count,&component_src,&meta_src,&component_h,&meta_h);
    
    if(!source_count){
        printf("Error: Not source files specified\n");
        return -1;
    }
    
    if(!source_count){
        printf("Error: No source files specified\n");
        return -1;
    }
    
    if(!component_src && !meta_src){
        printf("Error: No output files specified\n");
        return -1;
    }
    
    if(!meta_src){
        printf("Error: No meta_src file specified\n");
        return -1;
    }
    
    
#if 0
    {
        
        for(u32 i = 0; i < source_count;i++){
            printf("source:%s\n",source_array[i]);
        }
        
        printf("component file:%s\n",component_src);
        printf("meta file:%s\n",meta_src);
        
        return;
    }
#endif
    
    auto struct_array = (GenericStruct*)alloc(sizeof(GenericStruct) * 1024);
    u32 struct_count = 0;
    
    auto enum_array = (GenericEnum*)alloc(sizeof(GenericEnum) * 1024);
    u32 enum_count = 0;
    
    
    auto function_array = (GenericFunction*)alloc(sizeof(GenericFunction) * 1024);
    u32 function_count = 0;
    
    for(u32 i = 0; i < source_count;i++){
        
        auto file = FOpenFile(source_array[i],F_FLAG_READONLY);
        
        ptrsize size;
        auto buffer = FReadFileToBuffer(file,&size);
        
        ParseSource(buffer,size,struct_array,&struct_count,enum_array,&enum_count,function_array,&function_count);
        
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
    
    WriteMetaFile(meta_src,meta_h,struct_array,struct_count,enum_array,enum_count,function_array,function_count);
    
    if(component_src){
        
        auto inc = meta_h;
        
        if(!inc){
            inc = meta_src;
        }
        
        
        WriteComponentMetaData(component_src,component_h
                               ,&struct_array[k],count,inc);
    }
    
    unalloc(struct_array);
    unalloc(enum_array);
    unalloc(function_array);
    
#ifdef DEBUG
    
    
    TimeSpec end;
    
    GetTime(&end);
    
    auto diff = GetTimeDifferenceMS(start,end);
    
    printf("TARGET %s PARSE TIME: %f(s)\n",meta_src,(f64)(diff/1000.0f));
    
#endif
    
    return 0;
}