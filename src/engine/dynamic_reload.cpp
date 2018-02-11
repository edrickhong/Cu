#pragma once

#ifdef _WIN32

#define GAMELIB_PATH "game.dll"
#define ACTIVE_GAMELIB_PATH "active_game.dll"

#else

#define GAMELIB_PATH "./libgame.so"
#define ACTIVE_GAMELIB_PATH "./libactive_game.so"

#endif

struct GameLib{
  LibHandle handle;
  FileNode file_node;
  void* updaterender;
};

_persist void _ainline CopyLibToActiveLib(){
  
  auto lib = FOpenFile(GAMELIB_PATH,F_FLAG_READONLY);
  
  auto active_lib = FOpenFile(ACTIVE_GAMELIB_PATH,
			      F_FLAG_READWRITE | F_FLAG_CREATE | F_FLAG_TRUNCATE);

  ptrsize size = 0;

  auto lib_buffer = FReadFileToBuffer(lib,&size);

  FWrite(active_lib,lib_buffer,size);

  unalloc(lib_buffer);
  FCloseFile(lib);
  FCloseFile(active_lib);
}

GameLib InitGameLibrary(void** initfuncptr){

  CopyLibToActiveLib();
    
  GameLib lib = {};
    
  lib.handle = LLoadLibrary(ACTIVE_GAMELIB_PATH);
    
#if _debug
    
  if(lib.handle == 0){
        
#ifdef _MSC_VER
    printf("error code %d\n",GetLastError());
#else
    printf("%s\n",dlerror());
#endif
        
        
    _kill("failed to open game lib\n",1);
  }
    
#endif
    
  auto initfunct  = LGetLibFunction(lib.handle,"GameInit");
    
  _kill("failed to get init pointer",initfunct == 0);
    
  *initfuncptr = initfunct;
    
  return lib;
}

GameLib ReloadGameLibrary(GameLib lib,void** reload_funcptr,void* scenecontext){
    
  if(FFileChanged(GAMELIB_PATH,&lib.file_node)){

    auto writecomp_funptr = (void (*)(void*))LGetLibFunction(lib.handle,"GameComponentWrite");

    writecomp_funptr(scenecontext);
        
    if(lib.handle != 0){
      LUnloadLibrary(lib.handle);
      lib.handle = 0;
      lib.updaterender = 0;
    }
        
    lib.handle = 0;   
        
    while(!lib.handle){
      CopyLibToActiveLib();
      lib.handle = LLoadLibrary(ACTIVE_GAMELIB_PATH);  
    }
        
    void* func = LGetLibFunction(lib.handle, "GameUpdateRender");
        
    *reload_funcptr = LGetLibFunction(lib.handle, "GameReload");
        
    lib.updaterender = func;
        
  }
    
  return lib;
}
