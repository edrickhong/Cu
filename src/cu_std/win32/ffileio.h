#pragma once

#pragma once
#include <Windows.h>

#include "mode.h"
#include "ttype.h"
#include "string.h"

#define F_METHOD_START 0xFFFFFFFF
#define F_METHOD_CUR SEEK_CUR
#define F_METHOD_END SEEK_END

#define F_FLAG_READONLY GENERIC_READ
#define F_FLAG_WRITEONLY GENERIC_WRITE
#define F_FLAG_READWRITE (GENERIC_READ | GENERIC_WRITE)


#define F_FLAG_CREATE 0x00000008
#define F_FLAG_TRUNCATE 16
#define F_FILE_INVALID INVALID_HANDLE_VALUE

typedef HANDLE FileHandle;

typedef HANDLE* DirectoryHandle;

struct FileNode{
    FileHandle filehandle = 0;
    FILETIME timestamp = {};
};

struct FileInfo{
    s8 filename[256];
    u32 type;
};

FileHandle _ainline FOpenFile(const s8* filepath,u32 flags){
    
    u32 exflags = OPEN_EXISTING;
    
    if(flags & F_FLAG_CREATE){
        exflags = OPEN_ALWAYS;
        flags ^= F_FLAG_CREATE;
    }
    
    if(flags & F_FLAG_TRUNCATE){
        exflags = CREATE_ALWAYS;
        flags ^= F_FLAG_TRUNCATE;
    }
    
    FileHandle filehandle =
        CreateFile(filepath,flags,FILE_SHARE_READ,NULL,exflags,FILE_ATTRIBUTE_NORMAL,NULL);
    
    return filehandle;
}

void _ainline FCloseFile(FileHandle filehandle){
    CloseHandle(filehandle);
}

logic _ainline FIsFileExists(const s8* filepath){
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    logic ret = file != F_FILE_INVALID;
    FCloseFile(file);
    return ret;
}

void _ainline FRead(FileHandle filehandle,void* buffer,ptrsize size){
    DWORD dwRead;
    ReadFile(filehandle, buffer, size, &dwRead, NULL);
}

void _ainline FWrite(FileHandle filehandle,void* buffer,ptrsize size){
    DWORD dwWritten;
    WriteFile(filehandle, buffer, size, &dwWritten, NULL);
}

ptrsize _ainline FSeekFile(FileHandle filehandle,ptrsize move_size,u32 movemethod){
    if(movemethod == F_METHOD_START){
        SetFilePointer(filehandle, 0, NULL, SEEK_SET);
        movemethod = F_METHOD_CUR;
    }
    return SetFilePointer(filehandle, move_size, NULL, movemethod);
}

ptrsize _ainline FCurFilePosition(FileHandle filehandle){
    return FSeekFile(filehandle,0,F_METHOD_CUR);
}

ptrsize _ainline FGetFileSize(FileHandle filehandle){
    
    ptrsize size = FSeekFile(filehandle,0,F_METHOD_END);
    
    FSeekFile(filehandle,0,F_METHOD_START);
    
    return size;
}

s8* FReadFileToBuffer(FileHandle filehandle,ptrsize* size);

u32 FFindFirstFile(const s8* dirpath,DirectoryHandle* dirhandle,FileInfo* info);

u32 FFindNextFile(DirectoryHandle* dirhandle,FileInfo* info);

logic FFileChanged(const s8* file,FileNode* node);

#ifdef DEBUG

FileHandle DebugFOpenFile(const s8* filepath,u32 flags,s8* file,s8* function,u32 line);

void DebugFCloseFile(FileHandle filehandle);

void DebugDumpOpenFiles();

#define FOpenFile(filepath,flags) DebugFOpenFile(filepath,flags,(s8*)__FILE__,(s8*)__FUNCTION__,__LINE__)

#define FCloseFile(filehandle) DebugFCloseFile(filehandle)

#endif

