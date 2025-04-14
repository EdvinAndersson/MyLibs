#include "util_file.h"

str_t file_load(MemoryArena *arena, str_t file_name) {
    FILE *file;
    
    file = fopen(str_to_cstr(arena, file_name), "r");

    fseek(file, 0L, SEEK_END);
    uint32_t len = ftell(file);
    fseek(file, 0L, SEEK_SET);
    
    str_t result = str_create_empty(arena, len);
    fread(result.buf, sizeof(char), len, file);

    return result;
}