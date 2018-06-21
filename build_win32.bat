@echo off

SET mypath=%~dp0

rem actually compiling

if not defined DevEnvDir (
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)



cd /d %mypath:~0,-1%


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
