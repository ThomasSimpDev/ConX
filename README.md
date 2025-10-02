# ConX Game Engine

A 2D/3D game engine with Lua scripting support.

## Prerequisites

### Linux/macOS
- CMake 3.10+
- SDL2 development libraries
- OpenGL development libraries
- Lua 5.4 development libraries

### Windows
- Visual Studio 2019+ or MinGW-w64
- CMake 3.10+
- vcpkg (recommended for dependencies)

## Building

### Linux/macOS
```bash
./build.sh
```

### Windows
```cmd
build.bat
```

## Running

### Linux/macOS
```bash
./build/conx_engine <path_to_entry_file>
```

### Windows
```cmd
.\build\Release\conx_engine.exe <path_to_entry_file>
```

## Cleaning

### Linux/macOS
```bash
./clean.sh
```

### Windows
```cmd
clean.bat
```

## Examples

### Linux/macOS
```bash
./build/conx_engine lua_scripts/example.lua
```

### Windows
```cmd
.\build\Release\conx_engine.exe lua_scripts\example.lua
```
