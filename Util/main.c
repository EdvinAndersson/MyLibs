#include "src/util_memory_arena.h"
#include "src/util_string.h"

int main() {
    MemoryArena arena = arena_init(1024);

    arena_log_info(&arena);

    StackMemoryArena temp = arena_push_stack_arena(&arena);
    for (int i = 0; i < 10; i++) {
        int *j = arena_alloc(temp.arena, 1, int);
        *j = i+10;
    }
    arena_pop_stack_arena(&temp);

    int *i = arena_alloc_no_zero(&arena, 1, int);
    printf("v: %i\n", *i);

    str_t str1 = str_create("Hello ");
    str_t str2 = str_create("world!");
    str_t str3 = str_cat(&arena, str1, str2);
    str_t str4 = str_cat(&arena, str_copy(&arena, str1), str_create("guys!"));

    str_t str5 = str_substr(str3, 0, 8);

    printf("%s, len=%zi\n", str_to_cstr(&arena, str1), str1.size);
    printf("%s, len=%zi\n", str_to_cstr(&arena, str3), str3.size);
    printf("%s, len=%zi\n", str_to_cstr(&arena, str4), str4.size);

    printf("Equal: %i\n", str_equal(str1, str_create("Hello")));
    printf("Equal: %i\n", str_equal(str1, str_create("Hello ")));

    printf("Sub string: %s\n", str_to_cstr(&arena, str5));
    
    arena_log_info(&arena);
    return 0;
}