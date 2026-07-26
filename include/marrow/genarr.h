#ifndef MARROW_GENARR_H
#define MARROW_GENARR_H

#include "marrow.h"
#include "vektor.h"

typedef struct { union { u32 i; u32 valid; }; u32 gen; } GenarrHandle;

// 0th element is sentinel
// gen & 1 -> alive
// n_items(from vektor) doesnt count freed stuff
#define GENARR(T) VEKTOR(struct { u32 gen; u32 next_free; T val; })

#define GENARR_ITER(T) struct { T* _val; GenarrHandle handle; }
#define GENARR_ITER_ALIAS(T, alias) struct { union { T* alias; T* _val; }; GenarrHandle handle; }

#define next_free(i) *(u32*)((*items) + (i) * item_size + next_free_offset)
#define gen(i) *(u32*)((*items) + (i) * item_size)

#define genarr_init(a, initial_size, allocator) vektor_init((a), (initial_size), (allocator))
#define genarr_free(a) vektor_free((a))
#define genarr_clear(a) \
do { \
    for (u32 _genarr_clear_i = 0; _genarr_clear_i < (a).size; _genarr_clear_i++) { \
        if ((a).items[_genarr_clear_i].gen & 1)\
            (a).items[_genarr_clear_i].gen++;\
        (a).items[_genarr_clear_i].next_free = 0; \
    }\
    vektor_clear((a));\
} while (0)

static thread_local GenarrHandle _genarr_tmp_handle;
void _genarr_add(u8** items, u64* n_items, u64 item_size, u64 next_free_offset)
{
    u32 first_free = next_free(0);
    u32 i = first_free;
    if (first_free) {
        next_free(0) = next_free(first_free);
    }
    else {
        i = *n_items + 1;
        (*n_items)++;
    }
    gen(i) += 1;
    _genarr_tmp_handle = (GenarrHandle){ .i = i, .gen = gen(i) };
}

#define genarr_add(a, ...) (\
    vektor_ensure((a), (a).n_items + 2)/*extra space for the sentinel*/,\
    _genarr_add((u8**)&(a).items, &(a).n_items, sizeof(*(a).items), sizeof((a).items[0].gen)),\
    (a).items[_genarr_tmp_handle.i].val = (__VA_ARGS__),\
    _genarr_tmp_handle\
)

#define genarr_is(a, handle) (((handle).i < (a).size) && ((a).items[(handle).i].gen == (handle).gen) && ((a).items[(handle).i].gen & 1))
#define genarr_get(a, handle) (genarr_is((a), (handle)) ? &((a).items[(handle).i]).val : nullptr)

bool _genarr_next_valid(u8** items, u64 n_items, u64 item_size, GenarrHandle* handle)
{
    do { handle->i += 1; }
    while (handle->i <= n_items && !(gen(handle->i) & 1));
    if (handle->i <= n_items) {
        handle->gen = gen(handle->i);
        return true;
    }
    return false;
}

#define genarr_next_valid(a, iter) (\
    _genarr_next_valid((u8**)&(a).items, (a).n_items, sizeof(*(a).items), &(iter)->handle) ?\
        ((iter)->_val = &(a).items[(iter)->handle.i].val, true)\
        : false)

#define genarr_remove(a, handle) \
do { \
    if (!genarr_is((a), (handle))) break;\
    (a).items[(handle).i].gen++;\
    (a).items[(handle).i].next_free = (a).items[0].next_free;\
    (a).items[0].next_free = (handle).i;\
} while (0)

#undef next_free
#undef gen

#endif // MARROW_VEKTOR_H
