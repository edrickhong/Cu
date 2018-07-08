#pragma once

#include "mode.h"
#include "ttype.h"

#include "dlfcn.h"//for dlopen close


typedef void* LibHandle;
typedef void* LibFunc;

LibHandle _ainline LLoadLibrary(const s8* file){
    return dlopen(file,RTLD_NOW);
}

void _ainline LUnloadLibrary(LibHandle handle){
    dlclose(handle);
}

LibFunc _ainline LGetLibFunction(LibHandle handle,const s8* function_name){
    auto fptr = dlsym(handle,function_name);
    _kill("function doesn't exist\n",!fptr);
    return fptr;
}
