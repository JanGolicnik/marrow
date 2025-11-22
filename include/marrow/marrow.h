#ifndef MARROW_H
#define MARROW_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef uint8_t       u8;
typedef uint16_t      u16;
typedef uint32_t      u32;
typedef uint64_t      u64;

typedef int8_t        i8;
typedef int16_t       i16;
typedef int32_t       i32;
typedef int64_t       i64;

typedef uintmax_t     usize;

typedef float         f32;
typedef double        f64;

typedef _Bool         bool;

#define U8_MAX  UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX

#define I8_MAX  INT8_MAX
#define I16_MAX INT16_MAX
#define I32_MAX INT32_MAX
#define I64_MAX INT64_MAX

#ifndef true
#define true 1
#endif // true

#ifndef false
#define false 0
#endif // false

#ifndef nullptr
#define nullptr ((void*)0)
#endif // nullptr

#define BIT(n) (1ULL << (n))
#define BIT_IS_SET(val,n)  (((val) >> (n)) & 1)
#define BIT_SET(val, n)   ((val) |= BIT(n))
#define BIT_CLEAR(val, n) ((val) &= ~BIT(n))
#define BIT_TOGGLE(val,n) ((val) ^= BIT(n))

#ifndef loop
#define loop while(true)
#endif // loop

#ifndef mrw_unused
#define mrw_unused (void)
#endif // mrw_unused

#ifndef alignof
#define alignof _Alignof
#endif // alignof

#ifndef thread_local
#define thread_local _Thread_local
#endif // thread_local

#ifndef LINE_UNIQUE_VAR
#define LINE_UNIQUE_VAR_CONCAT(a, b) a##b
#define LINE_UNIQUE_VAR_PASS(a, b) LINE_UNIQUE_VAR_CONCAT(a, b)
#define LINE_UNIQUE_VAR(var) LINE_UNIQUE_VAR_PASS(var, __LINE__)
#define LINE_UNIQUE_I LINE_UNIQUE_VAR(i)
#endif //LINE_UNIQUE_VAR

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif // max

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif // min

#ifndef clamp
#define clamp(val, low, high) min(max(val, low), high)
#endif // clamp

static inline f32 wrap_float(f32 val, f32 max) {
    val = fmodf(val, max);
    if (val < 0.f) val += max;
    return val;
}

#ifndef is_between
#define is_between(val, low, high) ((val) > (low) && (val) < (high))
#endif // is_between

#ifndef is_between_inclusive
#define is_between_inclusive(val, low, high) ((val) >= (low) && (val) <= (high))
#endif // is_between_inclusive

#ifndef array_len
#define array_len(arr) (sizeof(arr)/sizeof((arr)[0]))
#define array_slice(arr) slice_to((arr), array_len((arr)))
#define array_for_each(arr, ptr) for(typeof((arr)) (ptr) = (arr); (ptr) < ((arr) + array_len((arr))); (ptr)++)
#define array_for_each_i(arr, i) for(usize i = 0; (i) < array_len((arr)); (i)++)
#endif // array_len

static inline void buf_copy(void* dst, const void* source, usize len)
{
    while(len--) ((u8*)dst)[len] = ((u8*)source)[len];
}
static inline i32 buf_cmp(const void* a, const void* b, usize len)
{
    for (usize i = 0; i < len; i++) {
        if (((u8*)a)[i] != ((u8*)b)[i])
            return (i32)((u8*)a)[i] - (i32)((u8*)b)[i];
    }
    return 0;
}
static inline void buf_set(void* dst, u8 value, usize len)
{
    while (len--) ((u8*)dst)[len] = value;
}

#define SLICE(type)                struct { type* start; type* end; }

#define slice_start(s)             ((s).start)
#define slice_end(s)               ((s).end)

#define slice(ptr_start, ptr_end)  { .start = (ptr_start), .end = (ptr_end) }
#define slice_range(ptr, from, to) slice((ptr) + (from), (ptr) + (to))
#define slice_to(ptr, count)       slice_range((ptr), 0, (count))

#define slice_t(s, t)              ((t ## Slice)slice((t*)slice_start((s)), (t*)slice_end((s))))

#define slice_count(s)             ((usize)(slice_end((s)) - slice_start((s))))
#define slice_size(s)              (slice_count(s) * sizeof(*slice_start((s))))

#define slice_for_each(s, ptr)     for(typeof(slice_start((s))) ptr = slice_start((s)); ptr != slice_end((s)); ptr++)
#define slice_for_each_i(s, i)     for(usize i = 0; (slice_start((s)) + (i)) != slice_end((s)); i++)

#define slice_copy_ptr(s, from)    buf_copy((void*)slice_start((s)), (void*)(from), slice_size((s)))
#define slice_copy(s, from)        slice_copy_ptr(s, slice_start(from))
#define slice_cmp(a, b)            buf_cmp((void*)slice_start(a), (void*)slice_start(b), min(slice_size((a)), slice_size((b))))
#define slice_set(s, value)        do { slice_for_each(s, ptr) (*ptr) = (value); } while(false)

#ifndef struct
#define struct(name)         \
    typedef struct name name;\
    typedef SLICE(name) name##Slice;\
    struct name
#endif // struct

typedef SLICE(char) s8;
typedef SLICE(u8) u8Slice;
#define str(s) ((s8)slice_to(s, str_len(s)))
#define slice_bytes(s)             slice_t(s, u8)

static inline u32 str_len(const char* str)
{
    const char* iter = str;
    while (*iter) iter++;
    return iter - str;
}

static inline char* s8_find_r(s8 s, char c)
{
    if (s.start == s.end) return nullptr;
    do { if(*(--s.end) == c) return s.end; } while (s.end != s.start);
    return nullptr;
}

static inline i32 s8_cmp(s8 a, s8 b)
{
    u32 a_count = slice_count(a);
    u32 b_count = slice_count(b);
    if (a_count != b_count) return a_count - b_count;
    return slice_cmp(a, b);
}

static inline u64 hash_bytes(u8Slice s)
{
    u64 hash = 0xcbf29ce484222325ULL;
    slice_for_each(s, v) {
        hash ^= (u8)(*v);
        hash *= 0x100000001b3ULL;
    }
    return hash;
}
#define hash_slice(s) hash_bytes(slice_bytes((s)))

static inline u64 hash_u64(u64 val)
{
    val ^= val >> 33;
    val *= 0xff51afd7ed558ccdULL;
    val ^= val >> 33;
    val *= 0xc4ceb9fe1a85ec53ULL;
    val ^= val >> 33;
    return val;
}

static inline u64 hash_combine(u64 a, u64 b)
{
    u64 x = a ^ b;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

#define LINE_UNIQUE_HASH hash_combine(hash_slice(str(__FILE__)), hash_u64(__LINE__))

#define INV_SQRT_3 0.5773502691896258f  // 1/sqrt(3)

// insertion sort
#define sort_indices(out, array, n, cmp) do {\
    for (u32 i = 0; i < n; i++) {\
        typeof(*array)* a = &array[i];\
        u32 index = 0;\
        while(index < i) {\
            typeof(*array)* b = &array[out[index]];\
            if (cmp) break;\
            index++;\
        }\
        for (u32 j = i; j > index; j--) {\
            out[j] = out[j - 1];\
        }\
        out[index] = i;\
    }\
} while(false)

#define mergesort_indices(out, array, n, cmp) do {\
    u32 aux[(n)];\
    for (u32 i = 0; i < (n); i++) out[i] = i;\
    for (u32 width = 1; width < (n); width *= 2) {\
        for (u32 low = 0; low < (n); low += 2 * width) {\
            u32 mid = ((low + width) < (n)) ? low + width : (n);\
            u32 high = (low + 2 * width < (n)) ? low + 2 * width : (n);\
            u32 i = low, j = mid, k = low;\
            while (i < mid && j < high) {\
                typeof(*array)* a = &array[out[i]];\
                typeof(*array)* b = &array[out[j]];\
                aux[k++] = (!(cmp)) ? out[i++] : out[j++];\
            }\
            while (i < mid) {\
                aux[k++] = out[i++];\
            }\
            while (j < high) {\
                aux[k++] = out[j++];\
            }\
            for (k = low; k < high; k++) {\
                out[k] = aux[k];\
            }\
        }\
    }\
} while(false)

static inline u32 value_to_rgb(f32 value)
{
    value = clamp(value, 0.0f, 1.0f);
    return ((u8)(value * 0xff) << 16) | ((u8)(value * 0xff) << 8) | (u8)(value * 0xff);
}

static inline u32 to_byte(f32 x){
    // x in [0,1], round to nearest and clamp
    int v = (int)lrintf(x * 255.0f);
    if (v < 0) v = 0; else if (v > 255) v = 255;
    return (u32)v;
}

struct(HSV) {
    f32 hue;
    f32 saturation;
    f32 value;
};

static inline u32 hsv_to_rgb(HSV hsv)
{
    hsv.value = clamp(hsv.value, 0.0f, 1.0f);
    hsv.saturation = clamp(hsv.saturation, 0.0f, 1.0f);
    if (!isfinite(hsv.hue)) hsv.hue = 0; // hue irrelevant when S==0 or V==0; pick any
    hsv.hue = wrap_float(hsv.hue, 360.0f);

    f32 c = hsv.value * hsv.saturation;
    f32 h6 = hsv.hue / 60.0f;         // 0..6
    f32 x = c * (1.0f - fabsf(fmodf(h6, 2.0f) - 1.0f));
    f32 m = hsv.value - c;

    f32 r=0, g=0, b=0;
    if      (h6 < 1) { r=c; g=x; b=0; }
    else if (h6 < 2) { r=x; g=c; b=0; }
    else if (h6 < 3) { r=0; g=c; b=x; }
    else if (h6 < 4) { r=0; g=x; b=c; }
    else if (h6 < 5) { r=x; g=0; b=c; }
    else             { r=c; g=0; b=x; }

    u32 R = to_byte(r + m);
    u32 G = to_byte(g + m);
    u32 B = to_byte(b + m);
    return (R << 16) | (G << 8) | B;   // 0xRRGGBB
}

static inline HSV rgb_to_hsv(u32 color) {
    f32 r = (f32)((color >> 16) & 0xFF) / 255.f;
    f32 g = (f32)((color >>  8) & 0xFF) / 255.f;
    f32 b = (f32)( color        & 0xFF) / 255.f;

    f32 maxv = fmaxf(r, fmaxf(g, b));
    f32 minv = fminf(r, fminf(g, b));
    f32 delta = maxv - minv;

    f32 V = maxv;

    f32 S = (maxv <= 0.f) ? 0.f : (delta / maxv);

    f32 H;
    if (delta == 0.f) {
        H = 0.f;
    } else if (maxv == r) {
        H = 60.f * fmodf(((g - b) / delta), 6.f);
        if (H < 0.f) H += 360.f;
    } else if (maxv == g) {
        H = 60.f * (((b - r) / delta) + 2.f);
    } else {
        H = 60.f * (((r - g) / delta) + 4.f);
    }

    return (HSV){wrap_float(H, 360.0f), clamp(S, 0.0f, 1.0f), clamp(V, 0.0f, 1.0f)};
}

#ifndef push_stream
#define push_stream(stream) fflush(stream)
#endif // push_stream

#ifndef mrw_debug_color
#define mrw_debug_color "\x1b[92m"
#endif // mrw_debug_color

#ifndef mrw_error_color
#define mrw_error_color "\x1b[91m"
#endif // mrw_error_color

#ifndef mrw_text_color
#define mrw_text_color "\x1b[0m\x1b[90m"
#endif // mrw_text_color

#ifdef _PRINTCCY_H

#ifndef mrw_format
thread_local s8 _format_buf;
thread_local u32 _format_buf_len;
#define mrw_format(f, allocator, ...)\
(\
    _format_buf_len = print(0, 0, f, __VA_ARGS__),\
    _format_buf.start = allocator_alloc((Allocator*)allocator, _format_buf_len + 1, 1),\
    (void)print((char*)_format_buf.start, _format_buf_len, f, __VA_ARGS__),\
    _format_buf.end = _format_buf.start + _format_buf_len,\
    _format_buf\
)
#endif // mrw_format

#ifndef mrw_debug
#define mrw_debug(f, ...) do { printfb(stderr, mrw_debug_color "[DEBUG]" mrw_text_color " {} on line {}: \x1b[0m" f "\n", __FILE__, __LINE__ ,##__VA_ARGS__); push_stream(stderr); } while(0)
#endif // mrw_debug

#ifndef mrw_error
#define mrw_error(f, ...) do { printfb(stderr, mrw_error_color "[ERROR]" mrw_text_color " {} on line {}: \x1b[0m" f "\n", __FILE__, __LINE__ ,##__VA_ARGS__); push_stream(stderr); } while(0)
#endif // mrw_error

#ifndef mrw_abort
#define mrw_abort(f, ...) do { printfb(stderr, mrw_error_color "[ABORT]" mrw_text_color " {} on line {}: \x1b[0m" f "\n", __FILE__, __LINE__ ,##__VA_ARGS__); push_stream(stderr); exit(1); } while(0)
#endif // mrw_abort

#endif // _PRINTCCY_H

#endif // MARROW_H
