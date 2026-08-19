#ifndef MARROW_ALLOC_H
#define MARROW_ALLOC_H

#include "marrow.h"

#define mrw_alloc(alloc, T)\
    ((T*)_mrw_alloc((alloc), sizeof(T), alignof(T)))

#define mrw_alloc_n(alloc, T, n)\
    ((T*)_mrw_alloc((alloc), sizeof(T) * (n), alignof(T)))

#define mrw_realloc(alloc, ptr, old_count, new_count, T)\
    ((T*)_mrw_realloc((alloc), (ptr), sizeof(T) * (old_count), sizeof(T) * (new_count), alignof(T)))

#define mrw_alloc_copy(alloc, src, count, T)\
    ((T*)_mrw_alloc_copy((alloc), (src), sizeof(T) * (count), alignof(T)))

#define mrw_free(alloc, src) _mrw_free((alloc), (src), sizeof(*(src)))

typedef struct Allocator Allocator;

typedef void* (_mrw_alloc_alloc_func)(Allocator* allocator, usize size, usize align);
typedef void* (_mrw_alloc_realloc_func)(Allocator* allocator, void* ptr, usize old_size, usize new_size, usize align);
typedef void  (_mrw_alloc_free_func)(Allocator* allocator, void* ptr, usize size);

typedef struct Allocator {
    _mrw_alloc_alloc_func* alloc;
    _mrw_alloc_realloc_func* realloc;
    _mrw_alloc_free_func* free;
} Allocator;

static inline void* _mrw_default_alloc(Allocator* allocator, usize size, usize align) { return malloc(size); }
static inline void* _mrw_default_realloc(Allocator* allocator, void* ptr, usize old_size, usize new_size, usize align) { return realloc(ptr, new_size); }
static inline void  _mrw_default_free(Allocator* allocator, void* ptr, usize size) { free(ptr); }
thread_local Allocator _mrw_default_allocator = {.alloc = &_mrw_default_alloc, .realloc = &_mrw_default_realloc, .free = &_mrw_default_free };

static inline void* _mrw_alloc(Allocator* allocator, usize size, usize align) {
    void* ptr = (allocator ? allocator : &_mrw_default_allocator)->alloc(allocator, size, align);
    if (!ptr) mrw_abort("allocation failed !!!");
    return ptr;
}
static inline void* _mrw_realloc(Allocator* allocator, void* ptr, usize old_size, usize new_size, usize align) {
    return (allocator ? allocator : &_mrw_default_allocator)->realloc(allocator, ptr, old_size, new_size, align);
}
static inline void _mrw_free(Allocator* allocator, void* ptr, usize size) {
    if (!ptr) return;
    (allocator ? allocator : &_mrw_default_allocator)->free(allocator, ptr, size);
}
static inline void* _mrw_alloc_copy(Allocator* allocator, void* ptr, usize size, usize align)
{
    void* new_ptr = _mrw_alloc(allocator, size, align);
    buf_copy(new_ptr, ptr, size);
    return new_ptr;
}

static inline void* _mrw_fake_realloc(Allocator* allocator, void* ptr, usize old_size, usize new_size, usize align) {
    void* new_ptr = _mrw_alloc(allocator, new_size, align);
    buf_copy(new_ptr, ptr, old_size);
    return new_ptr;
}

static inline void _mrw_fake_free(Allocator* allocator, void* ptr, usize size) { return; }

typedef struct {
    Allocator _impl;
    char* data;
    usize capacity;
    usize used;
} Arena;
#define MRW_ARENA_IMPL ._impl = { .alloc = _mrw_arena_alloc, .realloc = _mrw_fake_realloc, .free = _mrw_fake_free }

static inline void mrw_arena_reset(Arena* a) { a->used = 0; }

static inline void* _mrw_arena_alloc(Allocator* allocator, usize size, usize align)
{
    Arena* a = (Arena*)allocator;
    if (a->used + size > a->capacity) mrw_abort("string allocator out of space");
    void* p = (void*)(a->data + a->used);
    a->used += size;
    return p;
}

typedef struct BumpAllocatorBlock {
    struct BumpAllocatorBlock* next;
    usize capacity, used;
    u8 data[];
} BumpAllocatorBlock;

typedef struct {
    Allocator _impl;
    BumpAllocatorBlock* first;
    Allocator* allocator;
} BumpAllocator;
#define MRW_BUMP_IMPL ._impl = { .alloc = _mrw_bump_alloc, .realloc = _mrw_fake_realloc, .free = _mrw_fake_free }

static inline void mrw_bump_reset(BumpAllocator* a) {
    for (BumpAllocatorBlock* b = a->first; b; b = b->next) b->used = 0;
}

static inline void* _mrw_bump_alloc(Allocator* allocator, usize size, usize align)
{
    BumpAllocator* a = (BumpAllocator*)allocator;
    loop {
        for (BumpAllocatorBlock* b = a->first; b; b = b->next) {
            u8* ptr = b->data + b->used;
            u8* aligned_ptr = (u8*)ptr_align_up(ptr, align);
            usize new_used = (usize)(aligned_ptr - b->data) + size;
            if (new_used <= b->capacity) {
                b->used = new_used;
                return aligned_ptr;
            }
        }

        BumpAllocatorBlock* prev = a->first;
        if (size + align < size) mrw_abort("uhh");
        usize capacity = max(prev ? (prev->capacity + prev->capacity / 2) : 1024, size + align);
        a->first = (BumpAllocatorBlock*)_mrw_alloc(a->allocator, sizeof(BumpAllocatorBlock) + capacity, alignof(BumpAllocatorBlock));
        *a->first = (BumpAllocatorBlock){ .next = prev, .capacity = capacity };
    }
}

#endif // MARROW_ALLOCATOR_H
