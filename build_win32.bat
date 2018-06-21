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