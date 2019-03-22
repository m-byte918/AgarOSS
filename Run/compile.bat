if %PROCESSOR_ARCHITECTURE% == x86 (
  g++ -std=c++17 -o OgarCpp -L.\vcpkg\installed\x86-windows\lib -luWS -lz -lssl -pthread -I.\vcpkg\installed\x86-windows\include -I.\*.cpp ..\Connection\*.cpp ..\Entities\*.cpp ..\Game\*.cpp ..\Modules\*.cpp ..\Player\*.cpp ..\Protocol\*.cpp
) else (
  g++ -std=c++17 -o OgarCpp -I.\vcpkg\installed\x64-windows\include -I.\*.cpp ..\Connection\*.cpp ..\Entities\*.cpp ..\Game\*.cpp ..\Modules\*.cpp ..\Player\*.cpp ..\Protocol\*.cpp -L..\x64\Release -luWS -lz -lssl -pthread
))