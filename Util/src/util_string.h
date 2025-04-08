#pragma once

#include "util_memory_arena.h"
#include "util_assert.h"

struct str_t {
    char *buf;
    size_t size;
} typedef str_t;

str_t str_create(char *str);
str_t str_cat(MemoryArena *arena, str_t str1, str_t str2);
str_t str_copy(MemoryArena *arena, str_t str);
str_t str_substr(str_t str, size_t start, size_t end);
int str_equal(str_t str1, str_t str2);
char* str_to_cstr(MemoryArena *arena, str_t str);
