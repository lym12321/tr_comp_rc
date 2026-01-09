cmake_minimum_required(VERSION 3.22)
project(rc VERSION 0.0.1 LANGUAGES C CXX ASM)

file(GLOB_RECURSE SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
)

add_library(rc STATIC ${SOURCES})
target_include_directories(rc PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(rc PUBLIC bsp m)

set_target_properties(rc PROPERTIES
        C_STANDARD 17
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED YES
        POSITION_INDEPENDENT_CODE NO
)

add_library(components::rc ALIAS rc)