#include "ffileio.h"

#ifdef  FOpenFile

#include "iintrin.h"

#undef FOpenFile

#endif

#ifdef  FCloseFile

#undef FCloseFile

#endif

s8* FReadFileToBuffer(FileHandle filehandle,ptrsize* filesize){

  ptrsize size = FSeekFile(filehandle,0,F_METHOD_END);

  FSeekFile(filehandle,0,F_METHOD_START);
  
    s8* buffer = (s8*)alloc(size);
  FRead(filehandle,buffer,size);

  if(filesize){
    *filesize = size;
  }

  return buffer;
}


u32 FFindFirstFile(const s8* dirpath,DirectoryHandle* dirhandle,FileInfo* info){
  
  *dirhandle = opendir(dirpath);
  
  return FFindNextFile(dirhandle,info);
}

u32 FFindNextFile(DirectoryHandle* dirhandle,FileInfo* info){

  dirent* dirent = readdir(*dirhandle);

  if(!dirent){
    return 0;  
  }

  //we can call stat() for the additional info

  strcpy(info->filename,dirent->d_name);

  info->type = dirent->d_type;

  return 1;
}

logic FFileChanged(const s8* file,FileNode* node){
  struct stat attr;

  stat(file,&attr);

  FileNode n = *node;

  *node = attr.st_ino;

  return n != attr.st_ino;
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
      
      auto actual_handle = LockedCmpXchg(&cur_entry->filehandle,expected_handle,
					    filehandle);

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

/*
  struct dirent {
  ino_t          d_ino;       //inode number 
  off_t          d_off;       //offset to the next dirent 
  unsigned short d_reclen;    //length of this record 
  unsigned char  d_type;      //type of file; not supported by all file system types 
  char           d_name[256]; //filename
  };

  //we can get the rest of this thru stat()
  typedef struct _WIN32_FIND_DATA {
  DWORD    dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD    nFileSizeHigh;
  DWORD    nFileSizeLow;
  DWORD    dwReserved0;
  DWORD    dwReserved1;
  TCHAR    cFileName[MAX_PATH];
  TCHAR    cAlternateFileName[14];
  } WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;
*/
