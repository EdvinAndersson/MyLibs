#pragma once

#include "Util/src/util_math.h"

#include "UtilWindow/src/util_window.h"
#include "util_renderer2d.h"

#ifdef IMGUI_SRC_ID
    #define GEN_ID ((IMGUI_SRC_ID) + (__LINE__))
#else
    #define GEN_ID (__LINE__)
#endif

typedef struct UIState {
    vec2_t mouse_pos;
    int mouse_down;

    int hot_item;
    int active_item;

    uint32_t shadow_offset;
} UIState;

extern UIState ui_state;

inline void ui_begin(Window *window) {
    ui_state.mouse_pos = window->mouse_pos;

    if (window_mouse_input(WINDOW_MOUSE1_PRESSED)) {
        ui_state.mouse_down = 1;
    }
    if (window_mouse_input(WINDOW_MOUSE1_RELEASE)) {
        ui_state.mouse_down = 0;
    }
    ui_state.shadow_offset = 5;
    ui_state.hot_item = 0;
}
inline void ui_end() {
    if (ui_state.mouse_down == 0) {
        ui_state.active_item = 0;
    } else {
        if (ui_state.active_item == 0)
            ui_state.active_item = -1;
    }
}
inline int ui_region_hit(int x, int y, int width, int height) {
    return (ui_state.mouse_pos.x >= x && ui_state.mouse_pos.x < x + width && ui_state.mouse_pos.y >= y && ui_state.mouse_pos.y < y + height);
}
inline int ui_button(int id, int x, int y, int width, int height) {
    if (ui_region_hit(x, y, width, height)) {
        ui_state.hot_item = id;
        
        if (ui_state.active_item == 0 && ui_state.mouse_down) {
            ui_state.active_item = id;
        }
    }
    r2d_render_rect_rounded((vec2_t){ x + ui_state.shadow_offset, y + ui_state.shadow_offset },  (vec2_t){ width, height }, (vec4_t){ 0, 0, 0, 1 }, 0,  (vec2_t){0,0}, 10);
    if (ui_state.hot_item == id) {
        if (ui_state.active_item == id) {
            r2d_render_rect_rounded((vec2_t){ x + 2.0f, y + 2.0f }, (vec2_t){ width, height }, (vec4_t){ 1, 1, 1, 1 }, 0, (vec2_t){0,0}, 10);
        } else {
            r2d_render_rect_rounded((vec2_t) { (float) x, (float) y }, (vec2_t) { width, height }, (vec4_t) { 0.6f, 0.6f, 0.6f, 1 }, 0, (vec2_t){0,0}, 10);
        }
    } else {
        r2d_render_rect_rounded((vec2_t) { (float) x, (float) y }, (vec2_t) { width, height }, (vec4_t) { 0.5f, 0.5f, 0.5f, 1.0f }, 0, (vec2_t){0,0}, 10);
    }

    if (ui_state.mouse_down == 0 && ui_state.hot_item == id && ui_state.active_item == id)
        return 1;

    return 0;
}
inline int ui_slider(int id, int x, int y, int width, int height, int max, int *value) {
    int btn_size = width / 2;
    int y_pos = ((height - btn_size * 2) * (*value)) / max + btn_size / 2;

    if (ui_region_hit(x, y + btn_size/2, width, height-btn_size)) {
        ui_state.hot_item = id;
        if (ui_state.active_item == 0 && ui_state.mouse_down)
            ui_state.active_item = id;
    }
    r2d_render_rect_rounded((vec2_t){ x + ui_state.shadow_offset, y + ui_state.shadow_offset },  (vec2_t){ width, height }, (vec4_t){ 0, 0, 0, 1 }, 0,  (vec2_t) {0,0}, 10);
    
    if (ui_state.hot_item == id) {
        r2d_render_rect_rounded((vec2_t) { (float) x, (float) y }, (vec2_t) { width, height }, (vec4_t) { 0.6f, 0.6f, 0.6f, 1.0f }, 0, (vec2_t) {0,0}, btn_size / 2);
    } else {
        r2d_render_rect_rounded((vec2_t) { (float) x, (float) y }, (vec2_t) { width, height }, (vec4_t) { 0.4f, 0.4f, 0.4f, 1.0f }, 0, (vec2_t) {0,0}, btn_size / 2);
    }
    
    if (ui_state.hot_item == id) {
        if (ui_state.active_item == id) {
            r2d_render_rect_rounded((vec2_t) { (float) x + btn_size / 2 - 1, (float) y - 1 + y_pos  }, (vec2_t) { btn_size + 2, btn_size + 2 }, (vec4_t) { 1, 1, 1, 1.0f }, 0, (vec2_t) {0,0}, 10);
        } else {
            r2d_render_rect_rounded((vec2_t) { (float) x + btn_size / 2, (float) y + y_pos  }, (vec2_t) { btn_size, btn_size }, (vec4_t) { 0.8f, 0.8f, 0.8f, 1.0f }, 0, (vec2_t) {0,0}, 10);
        }
    } else {
        r2d_render_rect_rounded((vec2_t) { (float) x + btn_size / 2, (float) y + y_pos  }, (vec2_t) { btn_size, btn_size }, (vec4_t) { 0.6f, 0.6f, 0.6f, 1.0f }, 0, (vec2_t) {0,0}, 10);
    }

    if (ui_state.active_item == id) {
        int mouse_pos = ui_state.mouse_pos.y - y;
        if (mouse_pos < btn_size) mouse_pos = btn_size;
        if (mouse_pos > height-btn_size) mouse_pos = height-btn_size;
        int v = ((mouse_pos - btn_size) * max) / (height-btn_size * 2);
        if (v != *value) {
            *value = v;
            return 1;
        }
    }
    return 0;
}