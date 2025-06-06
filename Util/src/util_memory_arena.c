#pragma once

#include "util_memory_arena.h"

MemoryArena scratch_arenas[SCRATCH_ARENA_COUNT];

MemoryArena arena_init(size_t size) {
    MemoryArena arena;

    arena.base_memory = calloc(1, size);
    arena.alloc_pos = arena.base_memory;
    arena.size = size;

    return arena;
}
void arena_init_scratch_arenas(size_t size_per_arena) {
    for (size_t i = 0; i < SCRATCH_ARENA_COUNT; i++) {
        scratch_arenas[i] = arena_init(size_per_arena);
    }
}
StackMemoryArena arena_get_scratch(MemoryArena *current) {
    UTIL_ASSERT(scratch_arenas[0].base_memory != 0, "Scratch arenas are not initialized");
    
    if (current == 0)
        return arena_push_stack_arena(&scratch_arenas[0]);

    for (size_t i = 0; i < SCRATCH_ARENA_COUNT; i++) {
        if (scratch_arenas[i].base_memory != current->base_memory) {
            return arena_push_stack_arena(&scratch_arenas[i]);
        }
    }
    UTIL_ASSERT(0, "No available scratch arenas");
    return (StackMemoryArena) {0};
}
void arena_release_scratch(StackMemoryArena *arena) {
    arena_pop_stack_arena(arena);
}
void* _arena_alloc(MemoryArena *arena, size_t size, size_t align) {
    void *res = _arena_alloc_no_zero(arena, size, align);
    memset(res, 0, size);

    return res;
}
void* _arena_alloc_no_zero(MemoryArena *arena, size_t size, size_t align) {
    ptrdiff_t ialigned = (ptrdiff_t) arena->alloc_pos;
    ialigned += (-ialigned) & (align - 1);
    void *result = (void *) ialigned;

    arena->alloc_pos = (void*) ((char*)ialigned + size);
    return result;
}
void arena_destory(MemoryArena *arena) {
    free(arena->base_memory);
}
void arena_free(MemoryArena *arena) {
    arena->alloc_pos = arena->base_memory;
}
StackMemoryArena arena_push_stack_arena(MemoryArena *arena) {
    StackMemoryArena stack_arena = {0};
    stack_arena.arena = arena;
    stack_arena.init_alloc_pos = arena->alloc_pos;
    
    return stack_arena;
}
void arena_pop_stack_arena(StackMemoryArena *stack_arena) {
    stack_arena->arena->alloc_pos = stack_arena->init_alloc_pos;
    stack_arena->arena = 0;
}
void arena_log_info(MemoryArena *arena) {
    printf("-------Arena Info-------\n");
    printf("Size: %zi\n", arena->size);
    size_t usage = ((char*)arena->alloc_pos - (char*)arena->base_memory);
    float usage_procentage = usage / (float) arena->size;
    printf("Usage: %zi/%zi=%.2f%% \n", usage, arena->size, usage_procentage*100);
    printf("------------------------\n");
}