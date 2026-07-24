#ifndef MARROW_VEKTOR_H
#define MARROW_VEKTOR_H

#include "marrow.h"
#include "allocator.h"

#define VEKTOR(item)\
struct \
{ \
    item* items; \
    u64 n_items; \
    u64 size; \
    Allocator* _allocator; \
}

#define vektor_init(v, initial_size, allocator) \
do { \
    v.size = 0; v.items = nullptr; v.n_items = 0; v._allocator = allocator; \
    vektor_ensure((v), (initial_size));\
} while (0)

#define vektor_free(v) \
do { \
    allocator_free(v._allocator, v.items, v.size * sizeof(*v.items)); \
    v.n_items = 0; v.size = 0; v.items = nullptr; \
} while (0)

#define vektor_clear(v) \
do { \
    v.n_items = 0;\
} while (0)

#define vektor_add(v, ...) vektor_insert(v, v.n_items, __VA_ARGS__)

static inline void _vektor_ensure(u8 **items, u64 *size, u64 new_size, u64 n_items, u64 item_size, Allocator *a) {
    if (n_items < *size) return;
    new_size = u64_nextpow2(max(new_size, n_items));
    *items = (u8*)(*items
        ? allocator_realloc(a, *items, *size * item_size, new_size * item_size, 1)
        : allocator_alloc(a, new_size * item_size, 1));
    buf_set(*items + *size * item_size, 0, (new_size - *size) * item_size);
    *size = new_size;
}

#define vektor_ensure(a, new_size) \
    _vektor_ensure((u8 **)&(a).items, &(a).size, (new_size), (a).n_items + 1, sizeof(*(a).items), (a)._allocator)

#define vektor_insert(v, position, ...) \
do { \
    vektor_ensure(v, position);\
    for (u64 i = v.n_items; i > position; i--) \
        v.items[i] = v.items[i - 1]; \
    v.items[position] = (__VA_ARGS__); \
    v.n_items++;\
} while (0)

#define vektor_remove(v, position) \
do { \
    if (position >= v.n_items) break; \
    for (u64 i = position; i < v.n_items - 1; i++) \
        v.items[i] = v.items[i + 1]; \
    v.n_items--; \
} while (0)

#endif // MARROW_VEKTOR_H
