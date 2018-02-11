#pragma once

#include "mode.h"
#include "ttype.h"

#include "Windows.h"

typedef HMODULE LibHandle;
typedef void* LibFunc;

LibHandle _ainline LLoadLibrary(const s8* file){
  auto ret = LoadLibrary(file);
  auto error = GetLastError();
  return ret;
}

void _ainline LUnloadLibrary(LibHandle handle){
  auto err = FreeLibrary(handle);
}

LibFunc _ainline LGetLibFunction(LibHandle handle,const s8* function_name){
  auto ret = GetProcAddress(handle,function_name);
  auto error = GetLastError();
  return ret;
}
