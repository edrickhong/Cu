#include "pparse.h"
#include "string.h"

void PSkipUntilChar(s8* src_string,u32* pos,s8 c){
    
    u32 curpos = *pos;
    
    while((src_string[curpos] != c) && (src_string[curpos] != 0)){
        curpos++;
    }
    
    *pos = curpos;
    
}

void PSkipLine(s8* src_string,u32* pos){
    PSkipUntilChar(src_string,pos,'\n');
    (*pos)++;
}

void PParseUntilChar(s8* dst_string,s8* src_string,u32* pos,s8 c,u32* outlen){
    
    
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

void PGetLine(s8* dst_string,s8* src_string,u32* pos,u32* outlen){
    
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


void PGetWord(s8* dst_string,s8* src_string,u32* pos,u32* word_count){
    
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

void PGetSymbol(s8* dst_string,s8* src_string,u32* pos,u32* word_count){
    
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


logic PSkipWhiteSpace(s8* src_string,u32* pos){
    
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


void PBufferListToArrayString(s8* array_name,s8* src_buffer,u32 src_size,s8* dst_buffer,u32* dst_size){
    
    if(dst_size){
        (*dst_size) = 0;
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
    
    for(u32 i = 0;;){
        
        if(i >= src_size){
            break;
        }
        
        s8 t_buffer[512] ={};
        u32 len = 0;
        
        PGetLine(&t_buffer[0],&src_buffer[0],&i,&len);
        
        if(len){
            
            s8 out_string[512] = {};
            out_string[0] = '"';
            
            for(u32 j = 0; j < len;j++){
                
                if(t_buffer[j] == '\r'){
                    continue;
                }
                
                if(t_buffer[j] == '\\'){
                    
                    //TODO: handle this
                }
                
                out_string[j + 1] = t_buffer[j];
            }
            
            out_string[len + 1] = '"';
            out_string[len + 2] = ',';
            out_string[len + 3] = '\n';
            
            len += 4;
            
            if(dst_size){
                (*dst_size) += len;
            }
            
            if(dst_buffer){
                memcpy(dst_buffer,out_string,len);
                dst_buffer += len;
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