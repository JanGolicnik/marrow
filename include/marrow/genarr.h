#ifndef MARROW_GENARR_H
#define MARROW_GENARR_H

#include "marrow.h"
#include "vektor.h"

typedef struct { u32 i; u32 gen; } GenarrIndex;

// 0th element is sentinel
// gen & 1 -> alive
// n_items(from vektor) is the index of the last non free element
#define GENARR(T) VEKTOR(struct { u32 gen; u32 next_free; T val; })

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

static thread_local GenarrIndex _genarr_tmp_index;
void _genarr_add(u8** items, u64* n_items, u64 item_size, u64 next_free_offset)
{
    #define next_free(i) *(u32*)((*items) + (i) * item_size + next_free_offset)
    #define gen(i) *(u32*)((*items) + (i) * item_size)
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
    _genarr_tmp_index = (GenarrIndex){ .i = i, .gen = gen(i) };
    #undef gen
    #undef next_free
}

#define genarr_add(a, ...) (\
    vektor_ensure((a), (a).n_items + 2)/*extra space for the sentinel*/,\
    _genarr_add((u8**)&(a).items, &(a).n_items, sizeof(*(a).items), sizeof((a).items[0].gen)),\
    (a).items[_genarr_tmp_index.i].val = (__VA_ARGS__),\
    _genarr_tmp_index\
)

#define genarr_is(a, index) (((index).i < (a).size) && ((a).items[(index).i].gen == index.gen) && ((a).items[(index).i].gen & 1))
#define genarr_get(a, index) (genarr_is((a), (index)) ? &((a).items[(index).i]).val : (mrw_abort("xd"), &((a).items[0]).val))

#define genarr_remove(a, index) \
do { \
    if (!genarr_is((a), (index))) break;\
    (a).items[(index).i].gen++;\
    (a).items[(index).i].next_free = (a).items[0].next_free;\
    (a).items[0].next_free = (index).i;\
} while (0)

#endif // MARROW_VEKTOR_H
