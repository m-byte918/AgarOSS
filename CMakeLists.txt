cmake_minimum_required(VERSION 3.0.2)

project(AgarOSS)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Run)

# -Wunknown-pragmas: disabled because visual studio
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -g -Wall -Wno-unknown-pragmas -Wno-unused-variable")

# Not using ssl or compression (yet)
add_definitions(-DLIBUS_NO_SSL -DUWS_NO_ZLIB)

file(GLOB AgarOSS_Source
    "*.cpp"
    "Protocol/*.cpp"
    "Player/*.cpp"
    "Modules/*.cpp"
    "Game/*.cpp"
    "Entities/*.cpp"
    "Connection/*.cpp"
)

# Disable warnings from uWS headers
include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/Run/installed/include)
link_directories(${CMAKE_SOURCE_DIR}/Run/installed/lib)

link_libraries(uSockets pthread)
add_executable(${CMAKE_PROJECT_NAME} ${AgarOSS_Source})