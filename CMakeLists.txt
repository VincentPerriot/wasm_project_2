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
set(SOURCES shell.cpp
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_impl_opengl3.cpp
    third_party/imgui/imgui_impl_glfw.cpp)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set Emscripten-specific options
set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".html"
    COMPILE_FLAGS ""
    OUTPUT_NAME "wasm_project_2"

    LINK_FLAGS "--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html 
        -s USE_GLFW=3 -s FULL_ES3 -s ALLOW_MEMORY_GROWTH=1 -s USE_WEBGL2=1 
        --preload-file ../../../assets --preload-file ../../../shaders"
)

#ASSIMP
set (BUILD_SHARED_LIBS OFF CACHE BOOL "")

set (ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set (ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set (ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "")
set (ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "")
set (ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "")
set (ASSIMP_BUILD_ALL_POSTPROCESSORS_BY_DEFAULT OFF CACHE BOOL "")

set (ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "")
set (ASSIMP_BUILD_MTL_IMPORTER ON CACHE BOOL "")
set (ASSIMP_BUILD_ASSJSON_EXPORTER ON CACHE BOOL "")

add_subdirectory(third_party/assimp)

# Include Assimp headers
target_include_directories(${PROJECT_NAME} PRIVATE third_party/assimp/include)

# Link Assimp
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)

# Include ImGui headers
target_include_directories(${PROJECT_NAME} PRIVATE third_party/imgui)

