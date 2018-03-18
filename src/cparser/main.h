#pragma once
#include "aallocator.h"

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

enum ParseState{
    ParseState_NORMAL,
    ParseState_STRING,
    ParseState_COMMENT,
    ParseState_STRUCT,
    ParseState_SCOPEBLOCK,
    ParseState_SCOPEARRAY,
    ParseState_ARGS,
};

struct ParseStateStack{
    ParseState parse_array[16] = {ParseState_NORMAL};
    u32 parse_count = 1;
};

ParseState GetState(ParseStateStack* pstate){
    return pstate->parse_array[pstate->parse_count - 1];
}

void PushState(ParseStateStack* pstate,ParseState state){
    pstate->parse_array[pstate->parse_count] = state;
    pstate->parse_count++;
}

ParseState PopState(ParseStateStack* pstate){
    auto state = GetState(pstate);
    pstate->parse_count--;
    return state;
}

void SkipBlockComments(s8* string,u32* pos,ParseStateStack* pstate){
    
    auto cur = *pos;
    
    if(IsStartComment(string[cur],string[cur + 1])){
        PushState(pstate,ParseState_COMMENT);
    }
    
    if(GetState(pstate) == ParseState_COMMENT){
        
        for(;;){
            
            s8 c = string[cur];
            
            if(c == 0){
                break;
            }
            
            if(IsEndComment(c,string[cur + 1])){
                cur += 2;
                PopState(pstate);
                break;
            }
            
            cur++;
        }
        
        *pos = cur;
        
    }
}
