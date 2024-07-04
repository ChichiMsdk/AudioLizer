@echo off

REM rmdir /s /q build

SET NAME=key_record
del /s /q build\%NAME%*
del /s /q build\*.pdb
del /s /q build\*.obj
del /s /q build\*.ilk

pushd build

SET MYINC=-I..\include -IC:\Lib\SDL\include -IC:\Lib\tracy\public^
 -IC:\Lib\tracy\public\tracy -IC:\Lib\sources\fftw-3.3.10\api

rem SET MYLIBPATH=/libpath:C:\Lib\sources\fftwdll
SET MYLIBPATH=/libpath:C:\Lib\sources\fftw-3.3.10\.libs

SET MYLIB=SDL3.lib SDL3_image.lib SDL3_ttf.lib SDL3_mixer.lib libfftw3.a

SET MYFILES=..\src\*.c ..\src\gui\*.c /Tp..\src\TracyClient.cpp

SET STATICLIB=User32.lib winmm.lib Advapi32.lib Shell32.lib Gdi32.lib Ole32.lib^
 Setupapi.lib Uuid.lib Imm32.lib Strmiids.lib Version.lib OleAut32.lib 

SET DFL=/fsanitize=address /Zi /Od /DWIN_32 /DTRACY_ENABLE
rem SET DFL= /Zi /Od /DWIN_32
rem SET DFL= /EHsc /D_DEBUG /MDd /Zi /Od

if "%1"=="" (
    echo No argument provided. Default dll...
	SET "MYLIBPATH=C:\Lib\debug\SDL3-dll"
	SET "PATH=%PATH%;C:\Lib\debug\SDL3-dll"
)

if "%1"=="static" (
    echo building with debug static
	SET "MYLIBPATH=C:\Lib\debug\SDL3-static /NODEFAULTLIB:msvcrt /NODEFAULTLIB:libcmtd %MYLIBPATH%"
	SET "MYLIB=%STATICLIB% %MYLIB%"
	SET "DFL=/MDd %DFL%"
)

if "%1"=="dll" (
    echo building with debug dynamic
	SET "MYLIBPATH=C:\Lib\debug\SDL3-dll %MYLIBPATH%"
	SET "PATH=%PATH%;C:\Lib\debug\SDL3-dll"
)


cl /Fe"%NAME%.exe" %DFL% %MYLIB% %MYFILES% %MYINC% /link /libpath:%MYLIBPATH% 

popd
