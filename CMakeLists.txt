cmake_minimum_required(VERSION 3.22)
project(SpaceBars)

include(FetchContent)

FetchContent_Declare(tungsten
    GIT_REPOSITORY "https://github.com/jebreimo/Tungsten.git"
    GIT_TAG master)

FetchContent_MakeAvailable(tungsten)

list(APPEND CMAKE_MODULE_PATH ${tungsten_SOURCE_DIR}/tools/cmake)

include(TungstenTargetEmbedShaders)

set(CMAKE_CXX_STANDARD 17)

add_executable(SpaceBars
    src/SpaceBars/main.cpp
    src/SpaceBars/SpaceBarsShaderProgram.cpp
    src/SpaceBars/SpaceBarsShaderProgram.hpp
    )

target_link_libraries(SpaceBars
    PRIVATE
        Tungsten::Tungsten
    )

tungsten_target_embed_shaders(SpaceBars
    FILES
        src/SpaceBars/SpaceBars-frag.glsl
        src/SpaceBars/SpaceBars-vert.glsl
    )
