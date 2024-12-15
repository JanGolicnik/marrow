#ifndef MARROW_H
#define MARROW_H

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;

typedef float              f32;
typedef double             f64;

typedef _Bool              bool;

#ifndef true
#define true 1
#endif // true

#ifndef false
#define false 0
#endif // false

#ifndef NULL
#define NULL 0
#endif // NULL

#ifndef PRINTLN
#define PRINTLN(str, ...) printf(str "\n", __VA_ARGS__); fflush(stdout);
#endif // PRINTLN

#endif // MARROW_H
