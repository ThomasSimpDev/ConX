@echo off
if not exist build mkdir build
cd build
cmake ..
cmake --build . --config Release
cd ..
echo Build complete! Run with: .\build\Release\conx_engine.exe
echo Clean with: .\clean.bat