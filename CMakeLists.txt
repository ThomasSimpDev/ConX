cmake_minimum_required(VERSION 3.10)
project(ConX VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(LUA REQUIRED lua)

# Include directories
include_directories(include)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${LUA_INCLUDE_DIRS})

# Source files
set(CONX_SOURCES
    src/core/conx_core.c
    src/math/conx_math.c
    src/scripting/conx_lua.c
    src/core/conx_all.c
    src/2d/conx_2d.c
    src/3d/conx_3d.c
)

# Create library
add_library(conx STATIC ${CONX_SOURCES})

# Link libraries
target_link_libraries(conx 
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${LUA_LIBRARIES}
    ${OPENGL_LIBRARIES}
    GLU
    m
)

# Engine executable
add_executable(conx_engine src/main.c)
target_link_libraries(conx_engine conx)
