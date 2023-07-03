cmake_minimum_required(VERSION 3.15)
project(wasm_project_2)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Emscripten toolchain file
set(EMSCRIPTEN_TOOLCHAIN C:/Users/perri/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake)

# Find Emscripten toolchain
if (EXISTS ${EMSCRIPTEN_TOOLCHAIN})
    include(${EMSCRIPTEN_TOOLCHAIN})
else()
    message(FATAL_ERROR "Emscripten toolchain not found. Please set the correct path.")
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Set the source files
set(SOURCES shell.cpp)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES}  "stb_image.h" "vec3.h" "maths.h" "camera.h" "vec4.h" "mat4.h"  "assets/vertices.h" )

# Set Emscripten-specific options
set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".html"
    COMPILE_FLAGS ""
    #Use later for custom html and CSS
    #LINK_FLAGS "-s ALLOW_MEMORY_GROWTH=1 -s USE_WEBGL2=1 --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html --preload-file"
    LINK_FLAGS "-s USE_GLFW=3 -s FULL_ES3 -s ALLOW_MEMORY_GROWTH=1 -s USE_WEBGL2=1 --preload-file ../../../assets --preload-file ../../../shaders"
)