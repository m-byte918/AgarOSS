include(vcpkg_common_functions)

IF (NOT VCPKG_CMAKE_SYSTEM_NAME STREQUAL "Linux")
   set(USE_LIBUV ON)
EndIF ()

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO uNetworking/uSockets
    REF v0.3.3
    SHA512  81c4408b28b4c18b09fd16c40e2e0a6119e4d7f2ea175c1dc3801b962fb0d7320d5d400e1c47de2d75820af98b26a3794cd934f70718c9e7efc9e06737c9b874
    HEAD_REF master
)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt DESTINATION ${SOURCE_PATH})

set(USE_OPENSSL OFF)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS 
        -DCMAKE_USE_OPENSSL=${USE_OPENSSL}
        -DLIBUS_USE_LIBUV=${USE_LIBUV}
    OPTIONS_DEBUG
        -DINSTALL_HEADERS=OFF
)

vcpkg_install_cmake()

file(COPY ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/usockets)
file(RENAME ${CURRENT_PACKAGES_DIR}/share/usockets/LICENSE ${CURRENT_PACKAGES_DIR}/share/usockets/copyright)

vcpkg_copy_pdbs()