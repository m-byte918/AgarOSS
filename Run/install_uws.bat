:: incomplete

git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

if %PROCESSOR_ARCHITECTURE% == x86 (
  .\vcpkg install openssl-windows zlib libuv uwebsockets
) else (
  .\vcpkg install openssl-windows:x64-windows zlib:x64-windows libuv:x64-windows uwebsockets:x64-windows
)