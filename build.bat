@echo off

REM rmdir /s /q build
del /s /q build\editor*
del /s /q build\*.pdb
del /s /q build\*.obj
del /s /q build\*.ilk

pushd build

rem set PATH=%PATH%C:\Users\Stagiaire\Desktop\SDL\lib\

SET NAME="editor.exe"
SET MYINC=-I..\include -IC:\Users\Stagiaire\Desktop\SDL\include

SET MYLIB=SDL3.lib
SET MYFILES=/Tc ..\src\vector.c /Tc ..\src\main.c /Tc ..\src\editor.c^
 /Tc ..\src\errors.c /Tc ..\src\audio_setup.c /Tc ..\src\file_process.c
rem  /Tp ..\src\TracyClient.cpp

rem SET DFL=/DTRACY_ENABLE /Zi /Od
rem SET DFL= /fsanitize=address /Zi /Od
SET DFL= /Zi /Od
rem SET DFL= /EHsc /D_DEBUG /MDd /Zi /Od

cl /Fe%NAME% %DFL% %MYLIB% %MYFILES% %MYINC% /link /libpath:C:\Users\Stagiaire\Desktop\SDL\lib

rem cl /Fe"editor.exe" /Zi /Od SDL3.lib SDL3_ttf.lib SDL3_mixer.lib ..\src\*.c^
rem -I..\include -IC:\Lib\SDL3\include /link /libpath:C:\Lib\SDL3\lib
popd
