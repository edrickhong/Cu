#include "ffileio.h"
#include "aallocator.h"
#include "iintrin.h"

#ifdef FOpenFile
#undef FOpenFile
#endif

#ifdef  FCloseFile
#undef FCloseFile
#endif

s8* FReadFileToBuffer(FileHandle filehandle,ptrsize* filesize){
    
    ptrsize size = FSeekFile(filehandle, 0, F_METHOD_END);
    
    FSeekFile(filehandle,0,F_METHOD_START);
    
    s8* buffer = (s8*)alloc(size);
    FRead(filehandle,buffer,size);
    
    if(filesize){
        *filesize = size;
    }
    
    return buffer;
}


u32 FFindFirstFile(const s8* dirpath,DirectoryHandle* dirhandle,FileInfo* info){
    
    WIN32_FIND_DATA FindFileData;
    *dirhandle = (DirectoryHandle)FindFirstFileEx(dirpath, FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);
    
    if (dirhandle == INVALID_HANDLE_VALUE)
        return 0;
    
    strcpy_s(info->filename, FindFileData.cFileName);
    info->type = FindFileData.dwFileAttributes;
    
    return 1;
}

u32 FFindNextFile(DirectoryHandle* dirhandle,FileInfo* info){
    WIN32_FIND_DATA FindFileData;
    if (!FindNextFile(*dirhandle, &FindFileData))
        return 0;
    
    strcpy_s(info->filename, FindFileData.cFileName);
    info->type = FindFileData.dwFileAttributes;
    return 1;
}

logic FFileChanged(const s8* file,FileNode* node){
    
    do{
        node->filehandle = FOpenFile(file,F_FLAG_READONLY);
    }while(node->filehandle == F_FILE_INVALID);
    
    
    
    auto old_timestamp = node->timestamp;
    
    ULARGE_INTEGER newtime,oldtime;
    
    GetFileTime(node->filehandle,0,0,&node->timestamp);
    
    FCloseFile(node->filehandle);
    
    memcpy(&oldtime,&old_timestamp,sizeof(old_timestamp));
    memcpy(&newtime,&node->timestamp,sizeof(node->timestamp));
    
    if(newtime.QuadPart != oldtime.QuadPart){
        return 1;
    }
    
    return 0;
}


#define _short_file_len 64

struct DebugFileEntry{
    FileHandle filehandle;
    s8* file;
    s8* function;
    u32 line;
    s8 filename[_short_file_len];
};

_persist DebugFileEntry file_array[1024] = {};

void DebugDumpOpenFiles(){
    
    for(u32 i = 0; i < _arraycount(file_array); i++){
        
        auto entry = file_array[i];
        
        if(entry.file){
            
            printf("%d %s:%s %s %d\n",(u32)entry.filehandle,entry.filename,entry.file,entry.function,
                   entry.line);  
        }
        
    }
    
}

void GetShortenedFilePath(const s8* filepath,s8* out_buffer){
    
    u32 len = strlen(filepath);
    
    u32 start_index = len - (_short_file_len - 1);
    u32 write_len = _short_file_len;
    
    if((_short_file_len - 1) > len){
        start_index = 0;
        write_len = len + 1;
    }
    
    memcpy(&out_buffer[0],&filepath[start_index],write_len);
}

FileHandle DebugFOpenFile(const s8* filepath,u32 flags,s8* file,s8* function,u32 line){
    
    auto filehandle = FOpenFile(filepath,flags);
    
    _kill("Invalid file\n", filehandle == F_FILE_INVALID);
    
    DebugFileEntry* entry = 0;
    
    for(u32 i = 0; i < _arraycount(file_array); i++){
        
        auto cur_entry = &file_array[i];
        
        if(!cur_entry->file){
            
            entry = cur_entry;
            
            auto expected_handle = cur_entry->filehandle;
            
            auto actual_handle = (FileHandle)LockedCmpXchg64((LONG64*)&cur_entry->filehandle,(LONG64)expected_handle,
                                                             (LONG64)filehandle);
            
            if(expected_handle == actual_handle){
                break;	
            }
            
        }
        
    }
    
    _kill("array is full\n",!entry);
    
    *entry = {filehandle,file,function,line};
    
    GetShortenedFilePath(filepath,&entry->filename[0]);
    
    return filehandle;
}

void DebugFCloseFile(FileHandle filehandle){
    
    DebugFileEntry* entry = 0;
    
    for(u32 i = 0; i < _arraycount(file_array); i++){
        
        auto cur_entry = &file_array[i];
        
        if(cur_entry->filehandle == filehandle){
            entry = cur_entry;
            break;
        }
        
    }
    
    *entry = {};
    
    _kill("entry not found\n",!entry);
    
    
    FCloseFile(filehandle);
}