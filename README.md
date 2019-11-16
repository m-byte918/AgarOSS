![AgarOSS](https://raw.githubusercontent.com/Megabyte918/AgarOSS/master/Logo.png)

# AgarOSS (Agar.io Open Source Server)
An Agar.io server implementation written in C++.

## Installing prerequisites
In order for the server to compile, you need to download and install some prerequisites:
- git
- make
- cmake 3.15+
- gcc 7.4.0+ (or [MinGW](https://github.com/m-byte918/AgarOSS/issues/14) for Windows) 

## Compiling

1. Open up a terminal in the `Run` folder or `cd` into it.
2. Run `compile.bat` for Windows or `sudo sh ./compile.sh` for Linux
3. Compilation will start after uWebSockets library has been installed

## Running

Windows: Run `AgarOSS.exe`

Linux: In a terminal, `cd` into the "Run" folder and run `./AgarOSS`
