#pragma once

#include "ttype.h"
#include "mode.h"

#include "string.h"

/*
TODO: we should make size or pos to ptrsize instead of u32
*/

enum PTokenType{
    
    PTOKEN_SYMBOL,
    PTOKEN_NUMBER,
    
    PTOKEN_OPENPAREN = '(',
    PTOKEN_CLOSEPAREN = ')',
    PTOKEN_OPENBRACKET = '[',
    PTOKEN_CLOSEBRACKET = ']',
    PTOKEN_OPENBRACE = '{',
    PTOKEN_CLOSEBRACE = '}',
    
    PTOKEN_COLON = ':',
    PTOKEN_SEMICOLON = ';',
    PTOKEN_COMMA = ',',
    PTOKEN_PERIOD = '.',
    PTOKEN_HYPHEN = '-',
    PTOKEN_OPENANGLEBRACKET = '<',
    PTOKEN_CLOSETANGLEBRACKET = '>',
    
    PTOKEN_ASTERISK = '*',
    PTOKEN_ADD = '+',
    PTOKEN_SUBTRACT = PTOKEN_HYPHEN,
    PTOKEN_POUND = '#',
    PTOKEN_AMPERSAND = '&',
    PTOKEN_PERCENT = '%',
    PTOKEN_EQUALS = '=',
    
    PTOKEN_SINGLEQUOTE = '\'',
    PTOKEN_DOUBLEQUOTE = '"',
    
    PTOKEN_BACKSLASH = '/',
    PTOKEN_FORWARDSLASH = '\\',
    
    PTOKEN_DOLLAR = '$',
};


logic _ainline PIsWhiteSpace(s8 c){
    return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}

logic _ainline PIsAlphabet(s8 c){
    
    return ((c > 64) && (c < 91)) || ((c > 96) && (c < 123));
}

logic _ainline PIsNumeric(s8 c){
    return (c > 47) && (c < 58);
}

logic _ainline PIsSymbol(s8 c){
    return PIsAlphabet(c) || PIsNumeric(c) || c == '$' || c == '#' || c == '_';
}

logic _ainline PIsVisibleChar(s8 c){
    return (c > 32) && (c < 127);
}

constexpr u32 PStrLen(const s8* string){
    
    u32 len = 0;
    
    for(;;){
        
        if(string[len] == 0){
            break;
        }
        
        len++;
    }
    
    return len;
}

constexpr u64 PHashString(const s8* string){
    
    u32 len = PStrLen(string);
    
    u64 hash = 0;
    
    for(u32 i = 0; i < len; i++){
        hash += ((string[i] * (i + 1)) * 31) ^ (((string[i] * 13) ^ ((hash + string[i]) | 19)) | 1);
    }
    
    return hash;
}


u32 IdentifyToken(s8 c);


f32 PHexStringToFloat(const s8* string);

//We should add an function pointer to allow different criteria 

void PGetWord(s8* dst_string,s8* src_string,u32* pos,u32* word_count);

void PGetSymbol(s8* dst_string,s8* src_string,u32* pos,u32* word_count);

void PGetLine(s8* dst_string,s8* src_string,u32* pos,u32* len);

void PSkipLine(s8* src_string,u32* pos);

logic PSkipWhiteSpace(s8* src_string,u32* pos);

void PSkipUntilChar(s8* src_string,u32* pos,s8 c);

void PParseUntilChar(s8* dst_string,s8* src_string,u32* pos,s8 c,u32* len);

void PGetFileExtension(s8* dst_string,const s8* file,u32* len);

logic _ainline PIsStringInt(s8* string){
    
    logic is_numeric = true;
    
    auto len = strlen(string);
    
    if(!len){
        return false;
    }
    
    for(u32 i = 0; i < len; i++){
        auto c = string[i];
        is_numeric = is_numeric &&(PIsNumeric(c) || (c == '-' && i == 0));
        
        if(!is_numeric){
            break;
        }
    }
    
    return is_numeric;
}

logic _ainline PIsStringFloat(s8* string){
    
    logic is_numeric = true;
    
    u32 dot_count = 0;
    
    auto len = strlen(string);
    
    if(!len){
        return false;
    }
    
    for(u32 i = 0; i < len; i++){
        
        auto c = string[i];
        
        is_numeric =
            is_numeric &&(PIsNumeric(c) || c == '.' || (c == '-' && i == 0)) && (dot_count < 2);
        
        if(c == '.'){
            
            if(!i){
                return false;	
            }
            
            dot_count ++;
        }
        
        if(!is_numeric){
            break;
        }
    }
    
    return is_numeric;
    
}

constexpr u32 PFindStringInBuffer(const s8* target_string,const s8* buffer,
                                  u32 buffer_len){
    
    auto target_len = PStrLen(target_string);
    
    auto offset = 0;
    
    for(u32 i = 0; i < buffer_len; i++){
        
        auto c = buffer[i];
        
        offset = i;
        
        if(c == target_string[0]){
            
            for(u32 j = 0; j < target_len; j++){
                
                if(target_string[j] != buffer[i]){
                    break;
                }
                
                if(j == (target_len - 1)){
                    return offset;
                }
                
                i++;
            }
            
        }
        
    }
    
    return (u32)-1;
}

#define PFindStringInString(target,ref) PFindStringInBuffer(target,ref,PStrLen(ref))


//TODO:
void PBufferToByteArrayString(s8* array_name,s8* src_buffer,u32 src_size,s8* dst_buffer,u32* dst_size);

void PBufferToDWordArrayString(s8* array_name,s8* src_buffer,u32 src_size,s8* dst_buffer,u32* dst_size);


void PBufferListToArrayString(s8* array_name,s8* src_buffer,u32 src_size,s8* dst_buffer,u32* dst_size);

constexpr logic PStringCmp(const s8* string1,const s8* string2){
    
    auto len = PStrLen(string1);
    
    if(len != PStrLen(string2)){
        return false;
    }
    
    for(u32 i = 0; i < len; i++){
        
        if(string1[i] != string2[i]){
            return false;
        }
        
    }
    
    return true;
}