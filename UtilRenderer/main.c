#include "UtilWindow/src/util_window.h"
#include "src/util_renderer2d.h"
#include "src/util_user_interface.h"

UIState ui_state = { 0 };

int main() {
    MemoryArena arena = arena_init(1024*1024*10);
    arena_init_scratch_arenas(1024*10);

    Window *window = window_create(str_create("Renderer Test"), 1024, 600);
    window_vsync(0);

    r2d_init(&arena);

    Texture sword_texture = r2d_create_texture_from_file(str_lit("UtilRenderer/Sword.png"), 0);
    Texture sword_texture2 = r2d_create_texture_from_file(str_lit("UtilRenderer/Sword.png"), 1);

    int running = 1;

    double second_timer = 0, previous_time = 0;

    do {
        double time = window_get_time();
        double delta_time = time - previous_time;
        double fps = 1 / delta_time;

        second_timer += delta_time;
            
        if (second_timer >= 1) {
            second_timer = 0;
            printf("FPS: %f\n", fps);
            arena_log_info(&arena);
        }
        previous_time = time;

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
        r2d_render_rect_rounded((vec2_t) {200,200}, (vec2_t) {50, 50}, (vec4_t) {0,1,0,1}, 45, (vec2_t) {0.5f, 0.5f}, 10);
        r2d_render_rect_rounded((vec2_t) {200,300}, (vec2_t) {200, 200}, (vec4_t) {1,0,0,1}, 0, (vec2_t) {0.5f, 0.5f}, 50);

        static float x = 400;

        r2d_render_sprite((vec2_t) {x, 300}, (vec2_t) {150, 150}, sword_texture, (vec4_t) {1,1,1,1}, 0, (vec2_t) {0.5f, 0.5f});
        r2d_render_sprite((vec2_t) {x + 200, 300}, (vec2_t) {150, 150}, sword_texture2, (vec4_t) {1,1,1,1}, 0, (vec2_t) {0.5f, 0.5f});

        if (window_key_input(KEY_A)) {
            x -= 1.0f;
        }
        if (window_key_input(KEY_D)) {
            x += 1.0f;
        }

        r2d_render_line((vec2_t) { 000, 000}, (vec2_t) {400, 300}, (vec4_t) {1, 0, 0, 1}); 
        r2d_render_line((vec2_t) { 100, 100}, (vec2_t) {400, 300}, (vec4_t) {1, 1, 0, 1}); 
        r2d_render_line((vec2_t) { 200, 200}, (vec2_t) {400, 300}, (vec4_t) {1, 0, 1, 1}); 
        r2d_render_line((vec2_t) { 300, 200}, (vec2_t) {400, 300}, (vec4_t) {1, 1, 0, 1}); 

        r2d_render_thick_line((vec2_t) { 200, 400}, (vec2_t) {500, 200}, 3, (vec4_t) {1, 1, 0, 1}); 

        ui_begin(window);

        if (ui_button(GEN_ID, 500, 50, 100, 30)) {
            
        }
        static int y = 0;
        if (ui_slider(GEN_ID, 500, 100, 30, 200, 50, &y)) {
            printf("Value: %i\n", y);
        }

        ui_end();

        r2d_flush_lines();
        r2d_flush();
        
        window_swap_buffers();
    } while (running > 0);

    return 0;
}