![AgarOSS](https://raw.githubusercontent.com/Megabyte918/AgarOSS/master/Logo.png)

# AgarOSS (Agar.io Open Source Server)
An Agar.io server implementation written in C++.

# Installing prerequisites
In order for the server to work, you need to download and install some prerequisites. You will need to install `git` in order for this to work.

## Windows

This should work on most versions of Windows.
1. Go into the `Run` folder.
2. Run `install_uws.bat`.
3. Wait until the installation has finished.

## Linux

Tested on Archlinux (Manjaro) and Ubuntu 16.04.
1. Open up a terminal in the `Run` folder or `cd` into it.
2. Run `sudo sh ./install_uws.sh`.
3. Wait until the installation has finished.


# Compilation and Running

Compiling the server can be done using the pre-written scripts located in the `Run` folder. You will need to install `g++-7` for this to work. On Windows you can check out this [guide](https://github.com/Megabyte918/AgarOSS/issues/14). On Linux you will need to install `g++` version 7 or its distro-equivalent.

## Windows

Simply go into the `Run` folder and run `compile.bat`. Once the server has compiled, go back into the root folder and run `AgarOSS.exe`.


## Linux

1. Open up a terminal in the `Run` folder or `cd` into it.
2. Run `sudo sh ./compile.sh`.
3. `cd` back into the root folder and run `./AgarOSS`.

