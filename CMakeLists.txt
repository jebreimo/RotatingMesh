cmake_minimum_required(VERSION 3.22)
project(RotatingMesh)

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR OR DEFINED ENV{LOCAL_SOURCE_PACKAGE_DIR})
    if (NOT LOCAL_SOURCE_PACKAGE_DIR)
        set(LOCAL_SOURCE_PACKAGE_DIR $ENV{LOCAL_SOURCE_PACKAGE_DIR})
    endif ()
endif ()

include(FetchContent)

if (DEFINED LOCAL_SOURCE_PACKAGE_DIR)
    FetchContent_Declare(tungsten
        URL "file://${LOCAL_SOURCE_PACKAGE_DIR}/Tungsten-0.2.68.tar.gz")
    FetchContent_Declare(xyz
        URL "file://${LOCAL_SOURCE_PACKAGE_DIR}/Xyz-0.2.70.tar.gz")
    FetchContent_MakeAvailable(tungsten xyz)
else ()
    FetchContent_Declare(tungsten
        GIT_REPOSITORY "https://github.com/jebreimo/Tungsten.git"
        GIT_TAG v0.2.68)
    FetchContent_MakeAvailable(tungsten)
endif ()


list(APPEND CMAKE_MODULE_PATH ${tungsten_SOURCE_DIR}/tools/cmake)

include(TungstenTargetEmbedShaders)

set(CMAKE_CXX_STANDARD 17)

add_executable(RotatingMesh
    src/RotatingMesh/main.cpp
    src/RotatingMesh/RotatingMeshShaderProgram.cpp
    src/RotatingMesh/RotatingMeshShaderProgram.hpp
    src/RotatingMesh/GouraudShaderProgram.cpp
    src/RotatingMesh/GouraudShaderProgram.hpp)

target_link_libraries(RotatingMesh
    PRIVATE
        Tungsten::Tungsten
    )

tungsten_target_embed_shaders(RotatingMesh
    FILES
        src/RotatingMesh/Gouraud-frag.glsl
        src/RotatingMesh/Gouraud-vert.glsl
        src/RotatingMesh/RotatingMesh-frag.glsl
        src/RotatingMesh/RotatingMesh-vert.glsl
    )
