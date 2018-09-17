#include "pparse.h"
#include "string.h"

void PSkipUntilChar(s8* src_string,ptrsize* pos,s8 c){
    
    u32 curpos = *pos;
    
    while((src_string[curpos] != c) && (src_string[curpos] != 0)){
        curpos++;
    }
    
    *pos = curpos;
    
}

void PSkipLine(s8* src_string,ptrsize* pos){
    PSkipUntilChar(src_string,pos,'\n');
    (*pos)++;
}

void PParseUntilChar(s8* dst_string,s8* src_string,ptrsize* pos,s8 c,u32* outlen){
    
    
    u32 startpos = *pos;
    u32 curpos = *pos;
    
    while((src_string[curpos] != c) && (src_string[curpos] != 0)){
        curpos++;
    }
    
    u32 len = (curpos - startpos);
    
    memcpy(dst_string,(src_string + startpos),len);
    
    *pos = curpos;
    
    if(outlen){
        *outlen = len;  
    }
    
}

void PGetLine(s8* dst_string,s8* src_string,ptrsize* pos,u32* outlen){
    
    PParseUntilChar(dst_string,src_string,pos,'\n',outlen);
    (*pos)++;
}


u32 IdentifyToken(s8 c){
    
    u32 type = c;
    
    if(PIsAlphabet(c)){
        type = PTOKEN_SYMBOL;
    }
    
    if(PIsNumeric(c)){
        type = PTOKEN_NUMBER;
    }
    
    return type;
}


f32 PHexStringToFloat(const s8* string){
    
    _kill("hex string wrong format",strlen(string) != 10);
    
    s8* cur = (s8*)(string + 2);
    u32 len = 8;
    
    f32 f;
    
    u32 store = 0;
    
    //u32 bitshift = (l * 4) - 8;
    u32 bitshift = (len * 4) - 4;
    
    u32 mask = 0;
    
    for(u32 i = 0; i < len; i++){
        
        s8 c = cur[i];
        
        switch(c){
            
            case '0': {
                mask = 0;
            }break;
            
            case '1': {
                mask = 1;
            }break;
            
            case '2': {
                mask = 2;
            }break;
            
            case '3': {
                mask = 3;
            }break;
            
            case '4': {
                mask = 4;
            }break;
            
            case '5': {
                mask = 5;
            }break;
            
            case '6': {
                mask = 6;
            }break;
            
            case '7': {
                mask = 7;
            }break;
            
            case '8': {
                mask = 8;
            }break;
            
            case '9': {
                mask = 9;
            }break;
            
            case 'A': {
                mask = 10;
            }break;
            
            case 'B': {
                mask = 11;
            }break;
            
            case 'C': {
                mask = 12;
            }break;
            
            case 'D': {
                mask = 13;
            }break;
            
            case 'E': {
                mask = 14;
            }break;
            
            case 'F': {
                mask = 15;
            }break;
            
        }
        
        store = store ^ (mask << bitshift);
        
        
        bitshift -= 4;
    }
    
    f = *((float*)&store);
    
    return f;
}


void PGetWord(s8* dst_string,s8* src_string,ptrsize* pos,u32* word_count){
    
    u32 startpos = *pos;
    u32 curpos = *pos;
    
    //NOTE:should this be the default
    while(PIsAlphabet(src_string[curpos]) || src_string[curpos] == '-'){
        curpos++;
    }
    
    u32 len = (curpos - startpos);
    
    memcpy(dst_string,(src_string + startpos),len);
    
    *pos = curpos;
    
    if(word_count){
        *word_count = len;  
    }
    
}

void PGetSymbol(s8* dst_string,s8* src_string,ptrsize* pos,u32* word_count){
    
    u32 startpos = *pos;
    u32 curpos = *pos;
    
    //NOTE:should this be the default
    while(PIsSymbol(src_string[curpos])){
        curpos++;
    }
    
    u32 len = (curpos - startpos);
    
    memcpy(dst_string,(src_string + startpos),len);
    
    *pos = curpos;
    
    if(word_count){
        *word_count = len;  
    }
}


void PGetFileExtension(s8* dst_string,const s8* file,u32* len){
    
    u32 blen = strlen(file) + 1;
    u32 offset = 0;
    
    for(u32 i = blen -1; i != (u32)-1; i--){
        
        s8 c = file[i];
        
        if( c == '.'){
            
            offset = i + 1;
            break;
        }
        
    }
    
    _kill("no ext found\n",!offset);
    
    if(len){
        *len = (blen - offset);
    }
    
    memcpy(dst_string,(void*)&file[offset],(blen - offset));
}


logic PSkipWhiteSpace(s8* src_string,ptrsize* pos){
    
    u32 start = *pos;
    u32 cur = *pos;
    
    for(;;){
        
        s8 c = src_string[cur];
        
        
        if(!PIsWhiteSpace(c)){
            break;
        }
        
        
        cur++;
    }
    *pos = cur;
    
    return cur - start;
}


void PBufferListToArrayString(s8* array_name,s8* src_buffer,ptrsize src_size,s8* dst_buffer,ptrsize* dst_size,u32* arraycount){
    
    if(dst_size){
        (*dst_size) = 0;
    }
    
    if(arraycount){
        (*arraycount) = 0;
    }
    
    
    {
        s8 buffer[256] = {};
        
        sprintf(buffer,"const s8* %s[] = {\n",array_name);
        
        u32 len = strlen(buffer);
        
        if(dst_size){
            (*dst_size) += len;
        }
        
        if(dst_buffer){
            memcpy(dst_buffer,buffer,len);
            dst_buffer += len;
        }
        
        
    }
    
    for(ptrsize i = 0;;){
        
        if(i >= src_size){
            break;
        }
        
        s8 t_buffer[512] ={};
        u32 len = 0;
        
        PGetLine(&t_buffer[0],&src_buffer[0],&i,&len);
        
        if(len){
            
            if(arraycount){
                (*arraycount)++;
            }
            
            s8 out_string[512] = {};
            out_string[0] = '"';
            
            
            u32 offset = 1;
            
            for(u32 j = 0; j < len;j++){
                
                if(t_buffer[j] == '\r'){
                    continue;
                }
                
                out_string[j + offset] = t_buffer[j];
                
                if(t_buffer[j] == '\\'){
                    
                    offset++;
                    out_string[j + offset] = '\\';
                }
            }
            
            auto nlen = (u32)strlen(out_string) - 1;
            
            out_string[nlen + 1] = '"';
            out_string[nlen + 2] = ',';
            out_string[nlen + 3] = '\n';
            
            nlen += 4;
            
            if(dst_size){
                (*dst_size) += nlen;
            }
            
            if(dst_buffer){
                memcpy(dst_buffer,out_string,nlen);
                dst_buffer += nlen;
            }
        }
    }
    
    {
        auto end_string = "};\n";
        
        u32 len = strlen(end_string);
        
        if(dst_size){
            (*dst_size) += len + 1;
        }
        
        if(dst_buffer){
            memcpy(dst_buffer,end_string,len);
            dst_buffer += len;
        }
    }
    
}

void PSanitizeStringC(s8* buffer,ptrsize* k){
    
    auto cur = *k;
    
    PIgnoreWhiteSpace(buffer,&cur);
    
    for(;;){
        
        logic reparse = false;
        
        if(PIsCommentC(buffer[cur],buffer[cur + 1])){
            PSkipLine(buffer,&cur);
            reparse = true;
        }
        
        
        if(PIsPreprocessorC(buffer[cur])){
            PSkipLine(buffer,&cur);
            reparse = true;
        }
        
        
        auto keep_parsing = PIsStartCommentC(buffer[cur],buffer[cur + 1]);
        
        while(keep_parsing){
            
            keep_parsing = !PIsEndCommentC(buffer[cur],buffer[cur + 1]);
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


#define _hex_row_count 8


void PBufferToByteArrayString(s8* array_name,s8* src_buffer,ptrsize src_size,s8* dst_buffer,ptrsize* dst_size){
    
    {
        s8 buffer[256] = {};
        
        sprintf(buffer,"%s[] = {\n",array_name);
        u32 len = strlen(buffer);
        
        if(dst_buffer){
            memcpy(dst_buffer,buffer,len);
            dst_buffer += len;
        }
        
        if(dst_size){
            (*dst_size) = len;
        }
    }
    
    
    
    for(u32 i = 0; i < src_size; i++){
        
        auto c = src_buffer[i];
        
        s8 buffer[128] = {};
        
        PCharToHexString(c,buffer);
        
        buffer[4] = ',';
        
        if(((i + 1) % _hex_row_count) == 0){
            buffer[5] = '\n';
        }
        
        u32 len = strlen(buffer);
        
        if(dst_buffer){
            memcpy(dst_buffer,buffer,len);
            dst_buffer += len;
        }
        
        if(dst_size){
            (*dst_size) += len;
        }
    }
    
    
    {
        auto string = "\n};";
        u32 len = strlen(string);
        
        if(dst_buffer){
            memcpy(dst_buffer,string,len);
        }
        
        if(dst_size){
            (*dst_size) += len;
        }
    }
}




logic PFillEvalBufferC(s8* buffer,ptrsize* a,EvalChar* evaluation_buffer,u32* k,s8* terminator_array,u32 terminator_count,void (*tagevalbuffer)(EvalChar*,u32)){
    
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
    
    //MARK: why not just fill in the char anyway instead of all these ifs
    
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
    
    if(buffer[cur] == ':'){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString(":"),":"};
        
        evaluation_count++;
        
    }
    
    if(buffer[cur] == '='){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString("="),"="};
        
        evaluation_count++;
    }
    
    
    
    
    if(buffer[cur] == '['){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString("["),"["};
        
        evaluation_count++;
    }
    
    
    if(buffer[cur] == ']'){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString("]"),"]"};
        
        evaluation_count++;
        
    }
    
    
    if(buffer[cur] == '+'){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString("+"),"+"};
        
        evaluation_count++;
        
    }
    
    if(buffer[cur] == '-'){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString("-"),"-"};
        
        evaluation_count++;
        
    }
    
    if(buffer[cur] == '/'){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString("/"),"/"};
        
        evaluation_count++;
        
    }
    
    
    if(buffer[cur] == ','){
        
        evaluation_buffer[evaluation_count] =
            EvalChar{PHashString(","),","};
        
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
            
            tagevalbuffer(&evaluation_buffer[0],evaluation_count);
            
            ret = true && evaluation_count;
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



logic PFillEvalBufferC(s8* buffer,ptrsize* a,EvalChar* evaluation_buffer,u32* k,s8 terminator,void (*tagevalbuffer)(EvalChar*,u32)){
    
    return PFillEvalBufferC(buffer,a,evaluation_buffer,k,&terminator,1,tagevalbuffer);
}



//string execution

m64 OpAdd_U64(m64 a,m64 b){
    return {a.u + b.u};
}

m64 OpSub_U64(m64 a,m64 b){
    return {a.u - b.u};
}

m64 OpMul_U64(m64 a,m64 b){
    return {a.u * b.u};
}

m64 OpDiv_U64(m64 a,m64 b){
    return {a.u / b.u};
}


m64 OpAdd_S64(m64 a,m64 b){
    return {(u64)(a.i + b.i)};
}

m64 OpSub_S64(m64 a,m64 b){
    return {(u64)(a.i - b.i)};
}

m64 OpMul_S64(m64 a,m64 b){
    return {(u64)(a.i * b.i)};
}

m64 OpDiv_S64(m64 a,m64 b){
    return {(u64)(a.i / b.i)};
}

logic IsMathOp(OpCharType type){
    
    OpCharType array[] = {
        OpChar_SUB,
        OpChar_ADD,
        OpChar_DIV,
        OpChar_MUL,
        OpChar_BRACEBLOCK_START,
        OpChar_BRACEBLOCK_END,
    };
    
    for(u32 i = 0; i < _arraycount(array); i++){
        
        if(type == array[i]){
            
            return true;
        }
    }
    
    return false;
}

struct OpNode{
    
    m64 (*op)(m64,m64);
    
    union{
        m64 value;
    };
    
    union{
        
        struct{
            OpNode* a;
            OpNode* b;
        };
        
        
        struct{
            u64 dep_a;
            u64 dep_b;
        };
        
    };
    
    
    
};

struct OpDepInfo{
    
    OpNode* node;
    OpCharType type;
    
    
    u32 dep_a;
    u32 dep_b;
    
    u32 dep_array[32];
    
    u32 dep_count;
    
    u32 to_skip;
};

OpNode* BuildDepTree(OpDepInfo* info_array,u32 info_count){
    
    
    for(u32 i = 0; i < info_count; i++){
        
        auto a = &info_array[i];
        
        
        if(a->to_skip){
            continue;
        }
        
        for(u32 j = 0; j < info_count; j++){
            
            auto b = &info_array[j];
            
            if(b->to_skip){
                continue;
            }
            
            if(a->node != b->node && a->type >= b->type){
                
                for(u32 k = 0; k < a->dep_count; k++){
                    
                    auto dep = a->dep_array[k];
                    
                    if(b->dep_a == dep){
                        
                        b->node->a = a->node;
                        b->to_skip = true;
                        
                        //not sure about this
                        a->dep_array[a->dep_count] = b->dep_b;
                        a->dep_count++;
                        
                        a->node = b->node;
                        a->type = b->type;
                        
                        a->dep_b = b->dep_b;
                        break;
                    }
                    
                    
                    if(b->dep_b == dep){
                        
                        b->node->b = a->node;
                        b->to_skip = true;
                        
                        //not sure about this
                        a->dep_array[a->dep_count] = b->dep_a;
                        a->dep_count++;
                        
                        a->node = b->node;
                        a->type = b->type;
                        
                        a->dep_a = b->dep_a;
                        break;
                    }
                    
                    
                }
            }
            
            
        }
        
        
        
        
    }
    
    OpNode* node = 0;
    
    for(u32 i = 0; i < info_count; i++){
        
        auto info = &info_array[i];
        
        if(!info->to_skip){
            
            _kill("there is more than 1 valid node left\n",node);
            
            node = info->node;
            
#ifndef DEBUG
            break;
#endif
        }
        
    }
    
    
    return node;
}


m64 ExecuteOpTree(OpNode* node){
    
    
    if(node->op){
        
        auto value_a = ExecuteOpTree(node->a);
        auto value_b = ExecuteOpTree(node->b);
        
        node->value = node->op(value_a,value_b);
        
    }
    
    return node->value;
}


//bare bone extracts the block but ommits the first start and end block
void ExtractBraceBlock(OpChar* dst_array,u32* dst_count,OpChar* char_array,u32* cur,u32 len){
    
    u32 scope_count = 0;
    u32 count = 0;
    
    for(u32 i = (*cur); i < len; i++){
        
        auto c = &char_array[i];
        
        if(c->type == OpChar_BRACEBLOCK_START){
            
            if(scope_count){
                dst_array[count] = *c;
                count++;
            }
            
            scope_count++;
        }
        
        else if(c->type == OpChar_BRACEBLOCK_END){
            scope_count--;
            
            if(scope_count){
                dst_array[count] = *c;
                count++;
            }
        }
        
        else{
            
            dst_array[count] = *c;
            count++;
        }
        
        
        if(!scope_count){
            (*dst_count) = count;
            (*cur) = i;
            return;
        }
        
    }
    
    
}

m64 BuildAndExecuteOpChar(OpChar* char_array,u32 char_count){
    
    
    //building op tree - first get the values and add in math ops and store its dependent values by index
    
    m64 value_array[64] = {};
    u32 value_count = 0;
    
    
    OpNode node_array[64] = {};
    OpDepInfo info_array[64] = {};
    u32 node_count = 0;
    
    for(u32 i = 0; i < char_count; i++){
        
        auto c = &char_array[i];
        
        if(c->type == OpChar_VALUE){
            
            value_array[value_count].u = atoi(c->string);
            value_count++;
        }
        
        if(IsMathOp(c->type)){
            
            m64 (*op)(m64,m64) = 0;
            
            switch(c->type){
                
                case OpChar_SUB:{
                    op = OpSub_U64;
                }break;
                
                case OpChar_ADD:{
                    op = OpAdd_U64;
                }break;
                
                case OpChar_DIV:{
                    op = OpDiv_U64;
                }break;
                
                case OpChar_MUL:{
                    op = OpMul_U64;
                }break;
                
                
                case OpChar_BRACEBLOCK_START:{
                    
                    OpChar brace_char_array[64] = {};
                    u32 brace_char_count = 0;
                    
                    ExtractBraceBlock(brace_char_array,&brace_char_count,char_array,&i,char_count);
                    
                    auto value = BuildAndExecuteOpChar(brace_char_array,brace_char_count);
                    
                    
                    value_array[value_count] = value;
                    value_count++;
                    
                    continue;
                    
                }break;
                
                
                case OpChar_BRACEBLOCK_END:{
                    
                    _kill("all brace blocks should be parsed out above\n",1);
                    
                }break;
                
            }
            
            
            node_array[node_count].op = op;
            
            node_array[node_count].dep_a = value_count - 1;
            node_array[node_count].dep_b = value_count;
            
            
            info_array[node_count].node = &node_array[node_count];
            
            info_array[node_count].type = c->type;
            
            info_array[node_count].dep_a = value_count - 1;
            info_array[node_count].dep_b = value_count;
            
            info_array[node_count].dep_array[0] = value_count - 1;
            info_array[node_count].dep_array[1] = value_count;
            
            
            info_array[node_count].dep_count = 2;
            
            node_count++;
        }
    }
    
    
    if(!node_count && value_count == 1){
        return value_array[0];
    }
    
    
    u32 info_count = node_count;
    
    //sort according to pemdas
    
    qsort(info_array,info_count,
          sizeof(OpDepInfo),
          [](const void * a, const void* b)->s32 {
          
          auto node_a = (OpDepInfo*)a;
          auto node_b = (OpDepInfo*)b;
          
          
          //descending order in type
          if(node_a->type != node_b->type){
          return node_b->type - node_a->type;
          }
          
          //ascending order in dep_b
          else{
          
          return node_a->dep_b - node_b->dep_b;
          
          };
          
          
          });
    
#if 0
    
    
    
    printf("cnode count %d\n",info_count);
    
    for(u32 i = 0; i < info_count; i++){
        
        auto n = &info_array[i];
        
        printf("%d: %d(%d) %d(%d)\n",n->type,(u32)n->dep_a,(u32)value_array[n->dep_a].u,(u32)n->dep_b,(u32)value_array[n->dep_b].u);
    }
    
#endif
    
    
    auto root_node = BuildDepTree(info_array,info_count);
    
    //add value nodes and match values
    {
        
        u32 ncount = node_count;
        
        for(u32 i = 0; i < ncount; i++){
            
            auto n = &node_array[i];
            
            //this is max array count for dep. use macro
            if(n->dep_a < 32){
                
                node_array[node_count] = {0,value_array[n->dep_a],0,0};
                
                n->a = &node_array[node_count];
                
                node_count++;
            }
            
            
            if(n->dep_b < 32){
                
                node_array[node_count] = {0,value_array[n->dep_b],0,0};
                
                n->b = &node_array[node_count];
                
                node_count++;
            }
            
        }
    }
    
    return ExecuteOpTree(root_node);
    
}

m64 PEvaluateMathString(OpChar* char_array,u32 char_count,OpExecMode mode){
    
    return BuildAndExecuteOpChar(char_array,char_count);
}


m64 PEvaluateMathString(s8* string,OpExecMode mode){
    
    /*
    TODO: support float and float promotion
*/
    
    
    //tagging the string
    
    OpChar char_array[32] = {};
    u32 char_count = 0;
    
    u32 len = strlen(string);
    
    
    for(ptrsize i = 0; i < len; i++){
        
        PSanitizeStringC((s8*)string,&i);
        
        s8 symbol_buffer[128] = {};
        u32 symbol_len = 0;
        
        OpCharType type = OpChar_UNKNOWN;
        
        
        PGetSymbol(&symbol_buffer[0],(s8*)string,&i,&symbol_len);
        
        if(symbol_len){
            
            //PGetSymbol puts i right after the symbol (we don't want that)
            i--;
            
            if(PIsStringInt(symbol_buffer)){
                
                type = OpChar_VALUE;
            }
        }
        
        else if(string[i] == '+'){
            type = OpChar_ADD;
            
            symbol_buffer[0]  = '+';
            symbol_len = 1;
        }
        
        else if(string[i] == '-'){
            type = OpChar_SUB;
            
            symbol_buffer[0]  = '-';
            symbol_len = 1;
        }
        
        else if(string[i] == '*'){
            type = OpChar_MUL;
            
            symbol_buffer[0]  = '*';
            symbol_len = 1;
        }
        
        else if(string[i] == '/'){
            type = OpChar_DIV;
            
            symbol_buffer[0]  = '/';
            symbol_len = 1;
        }
        
        else if(string[i] == '('){
            type = OpChar_BRACEBLOCK_START;
            
            symbol_buffer[0]  = '(';
            symbol_len = 1;
        }
        
        else if(string[i] == ')'){
            type = OpChar_BRACEBLOCK_END;
            
            symbol_buffer[0]  = ')';
            symbol_len = 1;
        }
        
        else{
            
            printf("WARNING: UNEXPECTED CHAR IN STRING (%c)\n",string[i]);
        }
        
        
        if(symbol_len){
            
            char_array[char_count] = {type};
            memcpy(char_array[char_count].string,symbol_buffer,symbol_len);
            char_count++;
        }
        
        
        
    }
    
    return PEvaluateMathString(char_array,char_count);
}