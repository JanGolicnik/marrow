# Marrow

A header I made that does things I like

## Usage

Include the header wherever you need it.

Also define the appropriate implementation macros for the things you use in only one file.

```c
#define MARROW_VEKTOR_IMPLEMENTATION
#define MARROW_MAPA_IMPLEMENTATION
#include <marrow.h>
```

## Contents

### Types

```c
#define true 1

#define false 0

#define nullptr 0

#define NULL {}

typedef uint8_t        u8;
typedef uint16_t       u16;
typedef uint32_t       u32;
typedef uint64_t       u64;

typedef int8_t         i8;
typedef int16_t        i16;
typedef int32_t        i32;
typedef int32_t        i64;

typedef float          f32;
typedef double         f64;

typedef _Bool          bool;
```

### Functions

```c
#define loop while(true)

#define max(a, b) (((a) > (b)) ? (a) : (b))

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define clamp(val, low, high) min(max(val, low), high)
```

### Printing

`print`, `println` functions act like youd expect, basically just forward the calls to vprintf, I wish to make the syntax `"Value 1 is {}"` somehow possible, but we will have to wait for that.

`format` will `malloc` a new buffer to hold the formatted string so watch out for that

`debug`, `error` and `abort` macros use some formatting I like and print to stderr

All these function flush the output buffer after every call.

```c
void print(const char* format, ...);

void println(const char* format, ...);

char* format(const char* format, ...);

#define debug(format, ...) do { fprintf(stderr, debug_color "[DEBUG]" text_color " %s on line %d:\x1b[0m " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); push_stream(stderr); } while(0)

#define error(format, ...) do { fprintf(stderr, error_color "[ERROR]" text_color " %s on line %d:\x1b[0m " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); push_stream(stderr); } while(0)

#define abort(format, ...) do { fprintf(stderr, error_color "[ABORT]" text_color " %s on line %d:\x1b[0m " format "\n", __FILE__, __LINE__, ##__VA_ARGS__); push_stream(stderr); exit(1); } while(0)
```

### Vektor

A simple heap allocated resizable array implementation.

Basic usage:

```c
    typedef struct {
        int x;
        int y;
    } Data;
    Vektor* my_favorite_data = vektor_create(10, sizeof(Data)); // allocates space for 10 elements

    for (u32 i = 0; i < 10; i++)
    {
        vektor_add(my_favorite_data, &(Data){.x = i, .y = i});
    }

    Data* data = nullptr;
    while(data = (Data*)vektor_pop(my_favorite_data))
    {
        println("{.x: %d, .y: %d}", data->x, data->y);
    }

    vektor_destroy(&my_favorite_data);
```

### Mapa

A very basic bucket hash map implementation.

Supports both strings and arbitrary size structs.

Basic usage:

```c
    Mapa* mapa = mapa_create(mapa_hash_djb2, mapa_cmp_bytes);

    const char* key = "brum";
    mapa_insert_str(mapa, key, "brrr");

    MapaItem* item = mapa_get_str(mapa, key);
    debug("got '%s': %s", key, (char*)item->data);

    if (mapa_remove_str(mapa, key)) {
        debug("removed '%s'", key);
    }

    const char* item = mapa_get_str(mapa, key);
    if (!item) {
        debug("'%s' is not in the map", key);
    }

    mapa_destroy(mapa);

```

## Caution

Relies on a few standard library header files which i should get rid of.

```c
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
```
