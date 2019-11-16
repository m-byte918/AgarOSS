#!/bin/bash
# What was used to run this file: git 2.7.4, cmake 3.15.5, make 4.1, gcc 7.4.0

compile() {
    echo Compiling AgarOSS...
    make
    printf "Finished! Run AgarOSS executable to play."
    read _
    exit
}

# User has already installed vcpkg, recompile
if [ -d installed ]; then
    compile
fi

# Install vcpkg
echo Installing vcpkg...
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
sudo sh bootstrap-vcpkg.sh
./vcpkg integrate install

# Apply version patches before installing uWebSockets
echo Applying version patches...
cp -v ../patches/usockets/* ports/usockets
cp -v ../patches/uwebsockets/* ports/uwebsockets

# Install uWebSockets
echo Installing uWebSockets...
./vcpkg install uwebsockets

# Apply uWebSockets patch
echo Applying uWebSockets patch...
sudo cp -v ../patches/AsyncSocket.h installed/*-*/include/uwebsockets

# Move everything to Run/installed folder
mkdir ../installed
cp -vr installed/*-*/* ../installed

# Cleanup
echo Cleaning up...
cd ..
sudo rm -rf vcpkg

# Installation finished, now compile
echo uWebSockets installation finished. Cmaking...
cmake ..
compile
then