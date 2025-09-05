#pragma once

#include "util_memory_arena.h"
#include "util_assert.h"

#include <stdint.h>

struct str_t {
    char *buf;
    size_t size;
} typedef str_t;

#define str_lit(str) (str_t) { str, sizeof(str) - 1 }
#define str_comp(str) { str, sizeof(str) - 1 }

str_t str_create(char *str);
str_t str_create_fmt(MemoryArena *arena, uint32_t size, char *format, ...);
str_t str_create_empty(MemoryArena *arena, uint32_t size);
str_t str_cat(MemoryArena *arena, str_t str1, str_t str2);
str_t str_copy(MemoryArena *arena, str_t str);
str_t str_substr(str_t str, size_t start, size_t end);
uint32_t str_equal(str_t str1, str_t str2);
char* str_to_cstr(MemoryArena *arena, str_t str);
