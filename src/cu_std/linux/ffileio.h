#pragma once

#include "mode.h"
#include "ttype.h"


#include "fcntl.h"
#include "unistd.h"
#include "dirent.h"
#include "sys/stat.h" // get file size

#define F_METHOD_START 0xFFFFFFFF
#define F_METHOD_CUR SEEK_CUR
#define F_METHOD_END SEEK_END

#define F_FLAG_READONLY O_RDONLY
#define F_FLAG_WRITEONLY O_WRONLY
#define F_FLAG_READWRITE O_RDWR

#define F_FLAG_CREATE (O_CREAT)


#define F_FLAG_TRUNCATE O_TRUNC

#define F_FILE_INVALID (u32)-1


typedef u32 FileHandle;

typedef DIR* DirectoryHandle;

typedef ino64_t FileNode;

struct FileInfo{
    s8 filename[256];
    u32 type;
};

FileHandle _ainline FOpenFile(const s8* filepath,u32 flags){
    
    /*
    http://man7.org/linux/man-pages/man2/open.2.html
    
    The last arg gives the owner read write and execute permissions
*/
    
    FileHandle filehandle = open(filepath,flags,S_IRWXU);
    
    return filehandle;
}


void _ainline FCloseFile(FileHandle filehandle){
    close(filehandle);
}

logic _ainline FIsFileExists(const s8* filepath){
    auto file = FOpenFile(filepath,F_FLAG_READONLY);
    logic ret = file != F_FILE_INVALID;
    FCloseFile(file);
    return ret;
}

void _ainline FRead(FileHandle filehandle,void* buffer,ptrsize size){
    read(filehandle,buffer,size);
}

void _ainline FWrite(FileHandle filehandle,void* buffer,ptrsize size){
    write(filehandle,buffer,size);
}

ptrsize _ainline FSeekFile(FileHandle filehandle,ptrsize move_size,u32 movemethod){
    
    if(movemethod == F_METHOD_START){
        lseek(filehandle,0,SEEK_SET);
        movemethod = F_METHOD_CUR;
    }
    
    return lseek(filehandle,move_size,movemethod);
}

ptrsize _ainline FCurFilePosition(FileHandle filehandle){
    return FSeekFile(filehandle,0,F_METHOD_CUR);
}

ptrsize _ainline FGetFileSize(FileHandle filehandle){
    
    auto curpos = FCurFilePosition(filehandle);
    
    ptrsize size = FSeekFile(filehandle,0,F_METHOD_END);
    
    FSeekFile(filehandle,curpos,F_METHOD_START);
    
    return size;
}

s8* FReadFileToBuffer(FileHandle filehandle,ptrsize* size);

u32 FFindFirstFile(const s8* dirpath,DirectoryHandle* dirhandle,FileInfo* info);

u32 FFindNextFile(DirectoryHandle* dirhandle,FileInfo* info);

FileNode FGetFileNode(const s8* file);

logic FFileChanged(const s8* file,FileNode* node);


#ifdef DEBUG

FileHandle DebugFOpenFile(const s8* filepath,u32 flags,s8* file,s8* function,u32 line);

void DebugFCloseFile(FileHandle filehandle);

void DebugDumpOpenFiles();

#define FOpenFile(filepath,flags) DebugFOpenFile(filepath,flags,(s8*)__FILE__,(s8*)__FUNCTION__,__LINE__)

#define FCloseFile(filehandle) DebugFCloseFile(filehandle)

#endif
