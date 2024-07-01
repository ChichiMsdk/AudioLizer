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

SET DFL=/fsanitize=address /Zi /Od /DWIN_32
rem SET DFL= /Zi /Od /DWIN_32
rem SET DFL= /EHsc /D_DEBUG /MDd /Zi /Od

if "%1"=="" (
    echo No argument provided. Default dll...
	SET "MYLIBPATH=C:\Lib\debug\SDL3-dll"
	SET "PATH=%PATH%;C:\Lib\debug\SDL3-dll"
)

if "%1"=="static" (
    echo building with debug static
	SET "MYLIBPATH=C:\Lib\debug\SDL3-static /NODEFAULTLIB:msvcrt /NODEFAULTLIB:libcmtd"
	SET "MYLIB=%STATICLIB% %MYLIB%"
	SET "DFL=/MDd %DFL%"
)

if "%1"=="dll" (
    echo building with debug dynamic
	SET "MYLIBPATH=C:\Lib\debug\SDL3-dll"
	SET "PATH=%PATH%;C:\Lib\debug\SDL3-dll"
)


cl /Fe"%NAME%.exe" %DFL% %MYLIB% %MYFILES% %MYINC% /link /libpath:%MYLIBPATH%

popd
