#pragma once

#include "util_string.h"

str_t str_create(char *str) {
    size_t count = 0;
    while (str[count] != 0)
        count++;

    str_t result = {0};
    result.size = count;
    result.buf = str;

    return result;
}

str_t str_create_empty(MemoryArena *arena, int size) {
    str_t result = {0};
    result.size = size;
    result.buf = arena_alloc(arena, size, char);
    
    return result;
}

str_t str_cat(MemoryArena *arena, str_t str1, str_t str2) {
    size_t count = str1.size + str2.size;

    str_t result = {0};
    result.size = count;
    result.buf = arena_alloc_no_zero(arena, count, char);
    memcpy(result.buf, str1.buf, str1.size);
    memcpy(result.buf + str1.size, str2.buf, str2.size);

    return result;
}

str_t str_copy(MemoryArena *arena, str_t str) {
    str_t result = {0};
    result.size = str.size;
    result.buf = arena_alloc_no_zero(arena, str.size, char);
    memcpy(result.buf, str.buf, str.size);

    return result;
}

int str_equal(str_t str1, str_t str2) {
    if (str1.size != str2.size)
        return 0;
    
    for (size_t i = 0; i < str1.size; i++) {
        if (str1.buf[i] != str2.buf[i])
            return 0;
    }

    return 1;
}

str_t str_substr(str_t str, size_t start, size_t end) {
    UTIL_ASSERT(start <= end, "Start should be less than or equal to end");
    UTIL_ASSERT(end <= str.size, "End is outside the size of string");

    str_t result = {0};
    result.size = end - start;
    result.buf = &str.buf[start];

    return result;
}

char* str_to_cstr(MemoryArena *arena, str_t str) {
    char *result = arena_alloc_no_zero(arena, str.size + 1, char);
    memcpy(result, str.buf, str.size);
    result[str.size] = 0;
    return result;
}