include(vcpkg_common_functions)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO uNetworking/uWebSockets
    REF v0.16.2
    SHA512 309180890c7c1450c3317103a5d7483e74b30abb23f5938bcef65903c1b5c11422e6b606e096340576fab873a2b745876ad0036170df67361e0a6d5f499cdd22
    HEAD_REF master
)

file(COPY ${SOURCE_PATH}/src  DESTINATION ${CURRENT_PACKAGES_DIR}/include)
file(RENAME ${CURRENT_PACKAGES_DIR}/include/src ${CURRENT_PACKAGES_DIR}/include/uwebsockets/)

file(COPY ${SOURCE_PATH}/LICENSE DESTINATION ${CURRENT_PACKAGES_DIR}/share/uwebsockets)
file(RENAME ${CURRENT_PACKAGES_DIR}/share/uwebsockets/LICENSE ${CURRENT_PACKAGES_DIR}/share/uwebsockets/copyright)