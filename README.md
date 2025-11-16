# Marrow

A collection of headers I like

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
