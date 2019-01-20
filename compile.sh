#!/bin/bash

g++-7 -std=c++17 -o OgarCpp -I ./ *.cpp Connection/*.cpp Entities/*.cpp Game/*.cpp Modules/*.cpp Player/*.cpp Protocol/*.cpp -luWS -lz -lssl -pthread