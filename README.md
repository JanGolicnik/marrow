# Marrow

A collection of headers that i use for making stuff in c

Right now theres:
- useful typedefs, slices, and general utility functions (marrow.h)
- an allocator api (allocator.h)
- type safe dynamic vector (vektor.h)
- type safe hash map (mapa.h)
- type safe generational array (genarr.h)
- rendering abstraction over webgpu (reni.h)

## Including in your own project

You can just copy the header files over manually or use cmake and fetch content like so:

```cmake
include(FetchContent)

FetchContent_Declare(
    marrow
    GIT_REPOSITORY https://github.com/JanGolicnik/marrow.git
)

FetchContent_MakeAvailable(... marrow)

target_link_libraries(${PROJECT_NAME}
    ...
    marrow
)
```
