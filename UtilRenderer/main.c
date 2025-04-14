#include "UtilWindow/src/util_window.h"
#include "src/util_renderer2d.h"

int main() {
    MemoryArena arena = arena_init(1024*1024);

    Window *window = window_create(&arena, str_create("Renderer Test"));
    r2d_init(&arena);

    Texture sword_texture = r2d_create_texture_from_file(&arena, str_lit("UtilRenderer/Sword.png"), 0);
    Texture sword_texture2 = r2d_create_texture_from_file(&arena, str_lit("UtilRenderer/Sword.png"), 1);

    int running = 1;
    do {
        window_poll_message();

        while (window_event_exists() != 0) {
            WindowEvent window_event = window_event_pop();
            switch (window_event.type)
            {
                case WindowEventType_Close: {
                    WindowEvent_WindowEventType_Close *e = window_event.event;
                    running = 0;
                } break;
            }
        }
        r2d_update_projection(window->window_size);
        r2d_clear((vec4_t) {0.1,0.1,0.1,1});

        r2d_render_rect((vec2_t) {100,100}, (vec2_t) {50, 50}, (vec4_t) {1,1,1,1}, 0, (vec2_t) {0.5f, 0.5f});
        r2d_render_rect((vec2_t) {200,100}, (vec2_t) {50, 50}, (vec4_t) {0,0,1,1}, 25.5, (vec2_t) {0.5f, 0.5f});
        r2d_render_rect((vec2_t) {100,200}, (vec2_t) {50, 50}, (vec4_t) {1,0,0,1}, 0, (vec2_t) {0.5f, 0.5f});
        r2d_render_rect((vec2_t) {200,200}, (vec2_t) {50, 50}, (vec4_t) {0,1,0,1}, 45, (vec2_t) {0.5f, 0.5f});

        static float x = 400;

        r2d_render_sprite((vec2_t) {x, 400}, (vec2_t) {150, 150}, sword_texture, (vec4_t) {1,1,1,1}, 0, (vec2_t) {0.5f, 0.5f});
        r2d_render_sprite((vec2_t) {x + 200,400}, (vec2_t) {150, 150}, sword_texture2, (vec4_t) {1,1,1,1}, 0, (vec2_t) {0.5f, 0.5f});

        if (window_key_input(KEY_A)) {
            x -= 1.0f;
        }
        if (window_key_input(KEY_D)) {
            x += 1.0f;
        }

        r2d_flush();

        window_swap_buffers();
    } while (running > 0);

    return 0;
}