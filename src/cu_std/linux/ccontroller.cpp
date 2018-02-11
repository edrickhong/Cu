#include "ccontroller.h"
#define _joystickhash ('j' + 's')

#if 0

//hash the string instead. it'll be faster
u32 ParseIndex(const s8* string){
  return 0;
}

Controller CreateController(const s8* joystickpath){
  return {FOpenFile(joystickpath,F_FLAG_READONLY),ParseIndex(&(joystickpath[2]))};
}

void CRefreshControllers(ControllerList* controller_list){

  DirectoryHandle dirhandle;
  FileInfo info;
  
  u32 res = FFindFirstFile("/dev/input/",&dirhandle,&info);

  if(!res){
    return;
  }

  goto processname;

  //if info.filename contains js and index is not in the list, add

  while(FFindNextFile(&dirhandle,&info)){

  processname:
    //check if 'js'
    u32 hash = info.filename[0] + info.filename[1];

    if(hash == _joystickhash){
      s8* index_string = &(info.filename[2]);
      //parse index
      u32 index = ParseIndex(index_string);

      logic hasmatch = 0;
      
      //check if controller index is in list
      for(ptrsize i = 0; i < controller_list-> count; i++){

	if((*controller_list)[i].id == index){
	  hasmatch = 1;
	  break;
	}
	
      }

      if(!hasmatch){
	controller_list->PushBack(CreateController(info.filename));
      }
      
    }
    
  }
  
}

#endif
