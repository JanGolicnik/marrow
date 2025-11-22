cmake_minimum_required(VERSION 3.10)
project(marrow_test)

set(C_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    marrow
    GIT_REPOSITORY https://github.com/JanGolicnik/marrow.git
)

FetchContent_Declare(
    printccy
    GIT_REPOSITORY https://github.com/JanGolicnik/printccy.git
)

FetchContent_MakeAvailable(marrow printccy)

set(SOURCES main.c)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pedantic -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-function -Wno-bitfield-constant-conversion -Wno-misleading-indentation -Wno-unused-variable")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC)

target_link_libraries(${PROJECT_NAME}
    marrow
    printccy
)
