#include "stdio.h"
#include "stdlib.h"

#include "mode.h"
#include "ttype.h"

#include "ffileio.h"
#include "pparse.h"
#include "aallocator.h"

_intern void GetFileNameToStringArrayVar(s8* buffer,s8* filename){
    
    auto len = strlen(filename);
    
    u32 count = len;
    
    for(u32 i = 0; i < len; i++){
        
        if(filename[i] == '.'){
            count = i;
        }
    }
    
    memcpy(buffer,filename,count);
}

void PrintHelp(){
    
    printf("Format: filearray [-filedatatoarray/-filelisttoarray] src1 src2 src3 ... -header header -code code\n");
}

s32 main(s32 argc,s8** argv){
    
    if(argc < 5){
        PrintHelp();
        return 0;
    }
    
    auto option = argv[1];
    s8* source_array[16] = {};
    u32 source_count = 0;
    
    FileHandle codefile = 0;
    FileHandle headerfile = 0;
    {
        
        enum ADD_MODE{
            ADD_MODE_SRC,
            ADD_MODE_HEADER,
            ADD_MODE_CODE,
        };
        
        ADD_MODE mode = ADD_MODE_SRC;
        
        for(s32 i = 2; i < argc; i++){
            
            if(PHashString(argv[i]) == PHashString("-code")){
                
                mode = ADD_MODE_CODE;
                continue;
            }
            
            if(PHashString(argv[i]) == PHashString("-header")){
                
                mode = ADD_MODE_HEADER;
                continue;
            }
            
            if(mode == ADD_MODE_CODE){
                
                codefile = FOpenFile(argv[i],F_FLAG_WRITEONLY | F_FLAG_CREATE);
            }
            
            if(mode == ADD_MODE_HEADER){
                
                headerfile = FOpenFile(argv[i],F_FLAG_WRITEONLY | F_FLAG_CREATE);
            }
            
            
            
            if(mode == ADD_MODE_SRC){
                
                source_array[source_count] = argv[i];
                source_count++;
            }
        }
        
    }
    
    if(!codefile && !headerfile){
        PrintHelp();
        return 0;
    }
    
    switch(PHashString(option)){
        
        case PHashString("-filedatatoarray"):{
            
            u32 header_count = 0;
            
            
            for(u32 i = 0; i < source_count; i++){
                
                auto source = source_array[i];
                
                
                
                auto infile = FOpenFile(source,F_FLAG_READONLY);
                ptrsize size = 0;
                auto buffer = FReadFileToBuffer(infile,&size);
                
                FCloseFile(infile);
                
                
                
                s8 arrayname_string[256] = {};
                
                GetFileNameToStringArrayVar(arrayname_string,source);
                
                
                ptrsize dst_size = 0;
                
                PBufferToByteArrayString(arrayname_string,buffer,size,0,&dst_size);
                
                auto dst_buffer = (s8*)alloc(dst_size);
                
                PBufferToByteArrayString(arrayname_string,buffer,size,dst_buffer,&dst_size);
                
                if(dst_size){
                    
                    if(codefile){
                        FWrite(codefile,dst_buffer,dst_size);
                    }
                    
                    if(headerfile){
                        
                        if(!header_count){
                            auto string = "#pragma once\n\n\n";
                            FWrite(headerfile,(void*)string,strlen(string));
                        }
                        
                        s8 tbuffer[256] = {};
                        
                        sprintf(tbuffer,"u8 %s[%d];\n\n",arrayname_string,(u32)size);
                        
                        FWrite(headerfile,tbuffer,strlen(tbuffer));
                        
                        header_count++;
                    }
                    
                }
                
                
                unalloc(dst_buffer);
                unalloc(buffer);
            }
            
            
            
        }break;
        
        case PHashString("-filelisttoarray"):{
            
            u32 header_count = 0;
            
            
            for(u32 i = 0; i < source_count; i++){
                
                auto source = source_array[i];
                
                
                
                auto infile = FOpenFile(source,F_FLAG_READONLY);
                ptrsize size = 0;
                auto buffer = FReadFileToBuffer(infile,&size);
                
                FCloseFile(infile);
                
                
                
                s8 arrayname_string[256] = {};
                
                GetFileNameToStringArrayVar(arrayname_string,source);
                
                
                ptrsize dst_size = 0;
                u32 arraycount = 0;
                
                PBufferListToArrayString(arrayname_string,buffer,size,0,&dst_size);
                
                auto dst_buffer = (s8*)alloc(dst_size);
                
                PBufferListToArrayString(arrayname_string,buffer,size,dst_buffer,&dst_size,&arraycount);
                
                if(dst_size){
                    
                    
                    if(codefile){
                        FWrite(codefile,dst_buffer,dst_size);
                        
                        //printf("code %s\n",dst_buffer);
                    }
                    
                    if(headerfile){
                        
                        if(!header_count){
                            auto string = "#pragma once\n\n\n";
                            FWrite(headerfile,(void*)string,strlen(string));
                        }
                        
                        s8 tbuffer[256] = {};
                        
                        sprintf(tbuffer,"const s8* %s[%d];\n\n",arrayname_string,arraycount);
                        
                        FWrite(headerfile,tbuffer,strlen(tbuffer));
                        
                        header_count++;
                        
                        //printf("header %s\n",tbuffer);
                    }
                    
                }
                
                
                
                
                unalloc(dst_buffer);
                unalloc(buffer);
            }
            
        }break;
        
        default:{
            printf("Error: Invalid command %s\n",option);
            exit(1);
        }break;
    }
    
    
    FCloseFile(codefile);
    
    return 0;
}