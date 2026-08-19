# Marrow

A collection of headers that i use for making stuff in c

All the container types are type safe and should be reasonably fast, the macro definitions and allocator api usage needs some polish tho.

Right now theres:
- useful typedefs, slices, and general utility functions (marrow.h)
- an allocator api (allocator.h)
- dynamic array (vektor.h)
- hash map (mapa.h)
- generational array (genarr.h)
- 0 allocation json parser (json.h)
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

## License

This project is [Beerware](https://en.wikipedia.org/wiki/Beerware), if you find any of this cool then buy me a beer :)
