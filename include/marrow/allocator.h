#ifndef MARROW_ALLOCATOR_H
#define MARROW_ALLOCATOR_H

#include "marrow.h"

typedef void* (allocator_alloc_func)(void* allocator, usize size, usize align);
typedef void* (allocator_realloc_func)(void* allocator, void* ptr, usize old_size, usize new_size, usize align);
typedef void  (allocator_free_func)(void* allocator, void* ptr, usize size);

typedef struct
{
    void* context;
    allocator_alloc_func* alloc;
    allocator_realloc_func* realloc;
    allocator_free_func* free;
} Allocator;

static inline void* _allocator_default_alloc(void *ctx, usize size, usize align) { return malloc(size); }
static inline void* _allocator_default_realloc(void* ctx, void* ptr, usize old_size, usize new_size, usize align) { return realloc(ptr, new_size); }
static inline void  _allocator_default_free(void* ctx, void* ptr, usize size) { free(ptr); }

thread_local Allocator _default_allocator = {.alloc = &_allocator_default_alloc, .realloc = &_allocator_default_realloc, .free = &_allocator_default_free };

bool allocator_debug = false;

static inline void* allocator_alloc(Allocator* allocator, usize size, usize align)
{
    allocator = allocator ? allocator : &_default_allocator;
    return allocator->alloc(allocator, size, align);
}

static inline void* allocator_make_copy(Allocator* allocator, void* ptr, usize size, usize align)
{
    void* new_ptr = allocator_alloc(allocator, size, align);
    buf_copy(new_ptr, ptr, size);
    return new_ptr;
}

static inline void* allocator_realloc(Allocator* allocator, void* ptr, usize old_size, usize new_size, usize align)
{
    allocator = allocator ? allocator : &_default_allocator;
    return allocator->realloc(allocator, ptr, old_size, new_size, align);
}

#define allocator_alloc_t(alloc, T) \
    ((T*) allocator_alloc((alloc), sizeof(T), alignof(T)))

#define allocator_array_t(alloc, T, count) \
    ((T*) allocator_alloc((alloc), sizeof(T) * (count), alignof(T)))

#define allocator_realloc_t(alloc, ptr, old_count, new_count, T) \
    ((T*) allocator_realloc((alloc), (ptr), \
        sizeof(T) * (old_count), sizeof(T) * (new_count), alignof(T)))

#define allocator_make_copy_t(alloc, src, count, T) \
    ((T*) allocator_make_copy((alloc), (src), sizeof(T) * (count), alignof(T)))

static inline void allocator_free(Allocator* allocator, void* ptr, usize size)
{
    allocator = allocator ? allocator : &_default_allocator;
    allocator->free(allocator, ptr, size);
}

static inline void* align_up(void* x, size_t a) {
    return (void*)(((usize)x + (a-1)) & ~(uintptr_t)(a-1));
}

typedef struct {
    char* data;
    usize capacity;
    usize used;
} StringAllocator;

static inline void* string_allocator_alloc(void* allocator, usize size, usize align)
{
    StringAllocator* a = allocator;
    if (a->used + size > a->capacity) mrw_abort("string allocator out of space");
    void* p = (void*)(a->data + a->used);
    a->used += size;
    return p;
}

static inline void string_allocator_free(void* allocator, void* ptr, usize size) { return; }

static inline void string_allocator_reset(StringAllocator* a)
{
    a->used = 0;
}

// BUMP ARENA
typedef struct BumpAllocatorBlock {
    struct BumpAllocatorBlock* next;
    usize capacity, used;
    u8 data[];
} BumpAllocatorBlock;

typedef struct {
    Allocator allocator;
    BumpAllocatorBlock* first, *last;
    Allocator* inner_allocator;
} BumpAllocator;

static inline void* bump_allocator_alloc(void* a, usize size, usize align);

static inline BumpAllocatorBlock* bump_allocator_block_new(usize capacity, Allocator* allocator)
{
    BumpAllocatorBlock* block = (BumpAllocatorBlock*)allocator_alloc(allocator, sizeof(BumpAllocatorBlock) + capacity, 1);
    block->next = nullptr;
    block->capacity = capacity;
    block->used = 0;
    return block;
}

static inline void* bump_allocator_alloc(void* allocator, usize size, usize align)
{
    BumpAllocator* a = allocator;

    if (!a->last) {
        a->last = a->first = bump_allocator_block_new(1024, a->inner_allocator);
    }

    BumpAllocatorBlock* b = a->last;
    loop {
        u8* ptr = b->data + b->used;
        u8* aligned_ptr = align_up(ptr, align);
        usize used = (usize)(aligned_ptr - b->data) + size;

        if (used < b->capacity)
        {
            b->used = used;
            a->last = b;
            return aligned_ptr;
        }

        if (!b->next)
            b->next = bump_allocator_block_new(b->capacity * 2, a->inner_allocator);

        b = b->next;
    }
}

static inline void bump_allocator_free(void* allocator, void* ptr, usize size) { return; }

static inline BumpAllocator bump_allocator_create(void)
{
    return (BumpAllocator) {
        .allocator.alloc = bump_allocator_alloc,
        .allocator.free = bump_allocator_free,
    };
}

static inline void bump_allocator_reset(BumpAllocator* a)
{
    for (BumpAllocatorBlock* b = a->first; b; b = b->next) b->used = 0;
    a->last = a->first;
}

#endif // MARROW_ALLOCATOR_H
