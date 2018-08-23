#!/bin/bash

g++ -std=c++17 -o OgarCpp -I ./ *.cpp Connection/*.cpp Entities/*.cpp Game/*.cpp Modules/*.cpp Packets/*.cpp -luWS -lz -lssl -pthread