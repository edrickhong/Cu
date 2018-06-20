@echo off

SET mypath=%~dp0

rem actually compiling

if not defined DevEnvDir (
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)



cd /d %mypath:~0,-1%


cd src\shaders\


echo "BUILDING SHADERS"

for %%f in (*) do glslc -I ..\shadercommon -std=450 --target-env=vulkan %%f -o ..\..\rsrc\shaders\%%f.spv
for %%f in (*) do .\..\..\bin\Debug\glslparser %%f ..\..\rsrc\shaders\%%f.spv

glslc -I ..\shadercommon -std=450 --target-env=vulkan -DUSE_SKEL model.vert -o ..\..\rsrc\shaders\model_skel.vert.spv
.\..\..\bin\Debug\glslparser -DUSE_SKEL model.vert ..\..\rsrc\shaders\model_skel.vert.spv

glslc -I ..\shadercommon -std=450 --target-env=vulkan -DUSE_SKEL tfetch_region.vert -o ..\..\rsrc\shaders\tfetch_region_skel.vert.spv
.\..\..\bin\Debug\glslparser -DUSE_SKEL tfetch_region.vert ..\..\rsrc\shaders\tfetch_region_skel.vert.spv

glslc -I ..\shadercommon -std=450 --target-env=vulkan -DUSE_TEXTURE m_gui.frag -o ..\..\rsrc\shaders\m_gui_tex.frag.spv
.\..\..\bin\Debug\glslparser -DUSE_TEXTURE m_gui.frag ..\..\rsrc\shaders\m_gui_tex.frag.spv


cd ..\..\


echo "COMPILING..."

cd Make
cmake ..
msbuild ALL_BUILD.vcxproj /verbosity:quiet

cd ..

rem assuming we are using cl

rem set OUT_DIR=bin\Debug\

rem cd %OUT_DIR%

rem rem MARK: you need to replace the vulkan include path to your vulkan include directory
rem set INCLUDE_DIR=/I..\..\src\game\ /I..\..\src\engine\ /I..\..\include\ /I..\..\src\cu_std\cu /I..\..\src\cu_std\win32\ /I..\..\src\cu_std\amd64

rem set FLAGS=/Zi /std:c++14

rem set COMMON_LIBS=kernel32.lib User32.lib Ole32.lib

rem game We'll do this later because this needs a bit of work
rem cl %FLAGS% %INCLUDE_DIR% /LD ..\..\src\game\game.cpp cu_std.lib xaudio2.lib %COMMON_LIBS%
