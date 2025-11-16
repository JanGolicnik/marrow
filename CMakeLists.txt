cmake_minimum_required(VERSION 3.16)
project(marrow)

add_library(marrow INTERFACE)

target_include_directories(marrow
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)