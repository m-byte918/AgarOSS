#!/bin/bash

sudo apt-get install libssl-dev zlib1g-dev libuv-dev 
git clone https://github.com/uWebSockets/uWebSockets 
cd uWebSockets
git checkout 6d80b42
make
sudo make install
cd ..
sudo rm -r uWebSockets