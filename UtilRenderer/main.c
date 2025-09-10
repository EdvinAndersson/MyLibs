#include "UtilWindow/src/util_window.h"
#include "src/util_renderer2d.h"
#include "src/util_user_interface.h"
#include "src/util_framebuffer.h"

int main() {
    MemoryArena arena = arena_init(1024*1024*10);
    arena_init_scratch_arenas(1024*10);

    MemoryArena frame_arena = arena_init(1024*10);

    Window *window = window_create(str_create("Renderer Test"), 1024, 600);
    window_vsync(0);

    r2d_init(&arena);

    Texture sword_texture = r2d_texture_array_add(str_lit("UtilRenderer/res/Sword.png"), 0);
    Texture sword_texture2 = r2d_texture_array_add(str_lit("UtilRenderer/res/Sword.png"), 1);;

    FrameBuffer framebuffer = framebuffer_create(500, 500);

    Texture temp_texture = texture_2d_load(str_lit("UtilRenderer/res/Sword.png"));

    int running = 1;

    double second_timer = 0, previous_time = 0;
    double fps_counter = 0;

    do {
        double time = window_get_time();
        double delta_time = time - previous_time;
        double fps = 1 / delta_time;

        second_timer += delta_time;
            
        if (second_timer >= 1) {
            second_timer = 0;
            printf("FPS: %f\n", fps);
            fps_counter = fps;
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

        ui_begin(window);
        UIStyle style;
        style.background_color = (vec4_t) {1,0,0,1};
        ui_state.show_debug_lines = 1;

        static int arr[4] = {0};
        static uint16_t mask = 0b01;

        for (int i = 0; i < 4; i++) {
            int selected = (1 << i & mask) >> i;
            if (ui_checkbox(GEN_ID + i, 10 + i*30, 10, 25, 25, &selected).mouse_pressed) {
                mask = 1 << i;
            }
        }
        
        //r2d_render_quad((vec2_t) {400, 400}, framebuffer.texture);

        if (mask & (1 << 0)) {
            UIStyle style = ui_copy_style();
            style.text_size = 50;
            ui_push_style(style);
            ui_text(str_lit("AabclL||,[](){}"), 400, 50);
            ui_text(str_lit("Hello Edvin!llllllllllllllllllll"), 400, 100);
            ui_pop_style();

            str_t result = str_create_fmt(&frame_arena, 30, "FPS: %.f", fps_counter);
            ui_text(result, 10, 50);

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

            r2d_render_thick_line((vec2_t) { 200, 400}, (vec2_t) {500, 200}, 3, (vec4_t) {1, 1, 0, 1});

            //r2d_render_text(str_lit("Hello world"), (vec2_t) {600, 150}, 1.0f / 64.0f * 30, (vec3_t) {0.7, 0.7, 0.7});
            //r2d_render_text(str_lit("This is some text!!"), (vec2_t) {600, 250}, 1.2, (vec3_t) {1.0, 0.7, 0.7});
        }
        if (mask & (1 << 1)) {
            static int y = 0;

            ui_panel(40, 40, 400, 400);

            if (ui_button(GEN_ID, 50 + y, 50, 100, 30).mouse_pressed) {
            }
            if (ui_slider(GEN_ID, 50, 100, 24, 200, 50, &y)) {
                printf("Value: %i\n", y);
            }

            static float slider = 0;
            if (ui_slider_horizontal(GEN_ID, 300, 300, 0, 100, &slider)) {
            }

            static vec2_t v1 = {0}, v2 = {0};
            if (ui_slider_vec2(GEN_ID, 300, 360, 0, 100, &v1)) {
            }

            if (ui_slider_vec2(GEN_ID, 300, 400, 0, 100, &v2)) {
            }

            str_t slider_value = str_create_fmt(&frame_arena, 20, "Value: %i", y);
            ui_text(slider_value, 50, 100 + 200 + 10);

            static int tof = 0;
            if (ui_checkbox(GEN_ID, 100, 100, 25, 25, &tof).mouse_pressed) {
            }
            if (ui_checkbox(GEN_ID, 150, 100, 50, 50, &tof).mouse_pressed) {
            }
            ui_state.show_debug_lines = tof;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
        //glClearColor(0.1f, 0.1f, 1.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        r2d_render_quad((vec2_t) {0, 0}, temp_texture);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        r2d_render_quad((vec2_t) {0, 0}, framebuffer.texture);

        ui_end();
        r2d_flush();
        
        arena_free(&frame_arena);
        window_swap_buffers();
    } while (running > 0);

    return 0;
}