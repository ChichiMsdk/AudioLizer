@echo off

REM rmdir /s /q build

SET NAME=key_record
del /s /q build\%NAME%*
del /s /q build\*.pdb
del /s /q build\*.obj
del /s /q build\*.ilk

pushd build

SET MYINC=-I..\include -IC:\Lib\SDL\include
SET MYLIB=SDL3.lib SDL3_image.lib SDL3_ttf.lib SDL3_mixer.lib
SET MYFILES= ..\src\*.c ..\src\gui\*.c
SET STATICLIB=User32.lib winmm.lib Advapi32.lib Shell32.lib Gdi32.lib Ole32.lib^
 Setupapi.lib Uuid.lib Imm32.lib Strmiids.lib Version.lib OleAut32.lib 
SET DFL=/O2 /DWIN_32

if "%1"=="" (
    echo No argument provided. Default dll...
	SET "MYLIBPATH=C:\Lib\release\SDL3-dll"
	SET "PATH=%PATH%;C:\Lib\release\SDL3-dll"
)

if "%1"=="static" (
    echo building with release static
	SET "MYLIBPATH=C:\Lib\release\SDL3-static /NODEFAULTLIB:msvcrt /NODEFAULTLIB:libcmt"
	SET "MYLIB=%STATICLIB% %MYLIB%"
	SET "DFL=/MDd %DFL%"
)

if "%1"=="dll" (
    echo building with release dynamic
	SET "MYLIBPATH=C:\Lib\release\SDL3-dll"
	SET "PATH=%PATH%;C:\Lib\release\SDL3-dll"
)


cl /Fe"%NAME%.exe" %DFL% %MYLIB% %MYFILES% %MYINC% /link /libpath:%MYLIBPATH% 


rem cl /Fe"editor.exe" /Zi /Od SDL3.lib SDL3_ttf.lib SDL3_mixer.lib ..\src\*.c^
rem -I..\include -IC:\Lib\SDL3\include /link /libpath:C:\Lib\SDL3\lib
popd
