REM What was used to run this file: git 2.17.1, cmake 3.14.5, make 3.81, mingw (gcc 8.1.0)

@echo off

REM User has already installed vcpkg, recompile
:Compile
if exist "installed\" (
    echo Compiling AgarOSS...
    make
    echo Finished! Run AgarOSS.exe to play.
    pause > nul
    exit
)

REM Install vcpkg
echo Installing vcpkg...
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
call bootstrap-vcpkg.bat
vcpkg integrate install

REM Apply version patches before installing uWebSockets
echo Applying version patches...
copy /y ..\patches\usockets\* ports\usockets
copy /y ..\patches\uwebsockets\* ports\uwebsockets

REM Install uWebSockets
echo Installing uWebSockets...
set architecture=x86
if not %PROCESSOR_ARCHITECTURE% == %architecture% (
    set architecture=x64
)
vcpkg install uwebsockets:%architecture%-windows

REM Apply uWebSockets patch
echo Applying uWebSockets patch...
copy /y ..\patches\AsyncSocket.h installed\%architecture%-windows\include\uwebsockets

REM Move everything to Run/installed folder
mkdir ..\installed
xcopy /e /i "installed\%architecture%-windows" "..\installed"

REM Cleanup
echo Cleaning up...
cd downloads\tools\powershell*
for /f "delims=" %%g in ('forfiles /m *.exe /s') do (
    taskkill /f /im %%g /t
)
cd ..\..\..\..\
rmdir /s /q vcpkg

REM Copy all dll's to same directory as executable
copy /y installed\bin\*.dll .

REM Installation finished, now compile
echo uWebSockets installation finished. Cmaking...
cmake -G"MinGW Makefiles" ..
goto Compile