#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "util_assert.h"

struct MemoryArena {
    void *base_memory;
    void *alloc_pos;
    size_t size;
} typedef MemoryArena;

struct StackMemoryArena {
    MemoryArena *arena;
    void *init_alloc_pos;
} typedef StackMemoryArena;

#define arena_alloc(arena, count, type) (type*)_arena_alloc(arena, (count) * sizeof(type), _Alignof(type))
#define arena_alloc_no_zero(arena, count, type) (type*)_arena_alloc_no_zero(arena, (count) * sizeof(type), _Alignof(type))
#define arena_alloc_bytes(arena, size) _arena_alloc(arena, size, 1)

#define SCRATCH_ARENA_COUNT 4

MemoryArena arena_init(size_t size);
void arena_init_scratch_arenas(size_t size_per_arena);
StackMemoryArena arena_get_scratch(MemoryArena *current);
void arena_release_scratch(StackMemoryArena *arena);
void* _arena_alloc(MemoryArena *arena, size_t size, size_t align);
void* _arena_alloc_no_zero(MemoryArena *arena, size_t size, size_t align);
void arena_destory(MemoryArena *arena);
void arena_free(MemoryArena *arena);
void arena_log_info(MemoryArena *arena);

StackMemoryArena arena_push_stack_arena(MemoryArena *arena);
void arena_pop_stack_arena(StackMemoryArena *stack_arena);