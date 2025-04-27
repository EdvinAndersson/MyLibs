#pragma once

#include "util_user_interface.h"

UIState ui_state = { 0 };

void ui_begin(Window *window) {
    if (ui_state.box_arena.base_memory == 0) {
        ui_state.box_arena = arena_init(1024*10);
        ui_state.style_arena = arena_init(1024*10);
    }
    ui_state.mouse_pos = window->mouse_pos;

    if (window_mouse_input(WINDOW_MOUSE1_PRESSED)) {
        ui_state.mouse_down = 1;
    }
    if (window_mouse_input(WINDOW_MOUSE1_RELEASE)) {
        ui_state.mouse_down = 0;
    }

    if (ui_state.active_item == 0)
        ui_state.hot_item = 0;

    arena_free(&ui_state.style_arena);
    ui_state.style_stack = 0;
    arena_free(&ui_state.box_arena);
    ui_state.ui_box_count = 0;

    UIStyle style;
    style.background_color = (vec4_t) { 0.5f, 0.5f, 0.5f, 1.0f };
    style.hot_color = (vec4_t) { 0.6f, 0.6f, 0.6f, 1.0f };
    style.active_color = (vec4_t) { 1.0f, 1.0f, 1.0f, 1.0f };
    style.panel_color = (vec4_t) { 0.2f, 0.2f, 0.2f, 1.0f };
    style.shadow_offset = 5;
    ui_push_style(style);
}
void ui_end() {

    for (uint32_t i = 0; i < ui_state.ui_box_count; i++) {
        ui_box_t *box = ((ui_box_t*) ui_state.box_arena.base_memory + i);
        
        vec2_t pos = (vec2_t) { box->bounds.x, box->bounds.y };
        vec2_t size = (vec2_t) { box->bounds.z, box->bounds.w };

        if (box->flags & UIBoxFlags_DrawDropShadow) {
            r2d_render_rect_rounded(vec2_add(pos, (vec2_t) { box->style.shadow_offset, box->style.shadow_offset }),  size, (vec4_t){ 0, 0, 0, 1 }, 0,  (vec2_t){0,0}, 10);
        }

        if (box->flags & UIBoxFlags_Clickable) {
            if (ui_region_hit(box->bounds) && (ui_state.hot_item == 0 || ui_state.hot_item == box->id)) {
                ui_state.hot_item = box->id;
                
                if (ui_state.active_item == 0 && ui_state.mouse_down) {
                    ui_state.active_item = box->id;
                }
            }
        }
        if (box->flags & UIBoxFlags_DrawBackground) {
            if (box->flags & UIBoxFlags_ActiveAnimation && ui_state.active_item == box->id){
                r2d_render_rect_rounded(pos, size, box->style.active_color, 0, (vec2_t){0,0}, 10);
            } else if (box->flags & UIBoxFlags_HotAnimation && ui_state.hot_item == box->id){
                r2d_render_rect_rounded(pos, size, box->style.hot_color, 0, (vec2_t){0,0}, 10);
            } else {
                r2d_render_rect_rounded(pos, size, box->style.background_color, 0, (vec2_t){0,0}, 10);
            }
        }
        
        if (ui_state.show_debug_lines) {
            r2d_render_thick_line((vec2_t){box->bounds.x, box->bounds.y}, (vec2_t){box->bounds.x + box->bounds.z, box->bounds.y}, 1, (vec4_t){1, 0, 0, 1});
            r2d_render_thick_line((vec2_t){box->bounds.x, box->bounds.y+box->bounds.w}, (vec2_t){box->bounds.x + box->bounds.z, box->bounds.y + box->bounds.w}, 1, (vec4_t){1, 0, 0, 1});
            r2d_render_thick_line((vec2_t){box->bounds.x, box->bounds.y}, (vec2_t){box->bounds.x, box->bounds.y + box->bounds.w}, 1, (vec4_t){1, 0, 0, 1});
            r2d_render_thick_line((vec2_t){box->bounds.x+box->bounds.z, box->bounds.y}, (vec2_t){box->bounds.x + box->bounds.z, box->bounds.y + box->bounds.w}, 1, (vec4_t){1, 0, 0, 1});
        }
    }

    if (ui_state.mouse_down == 0) {
        ui_state.active_item = 0;
    } else {
        if (ui_state.active_item == 0)
            ui_state.active_item = -1;
    }

    ui_pop_style();
}
ui_box_t* ui_box(int32_t id, int32_t x, int32_t y, int32_t width, int32_t height, UIBoxFlags flags) {
    ui_box_t *box = arena_alloc(&ui_state.box_arena, 1, ui_box_t);
    box->id = id;
    box->bounds = (vec4_t) { x, y, width, height };
    box->flags = flags;
    box->style = ui_state.style_stack->style;

    ui_state.ui_box_count++;

    return box;
}
UIInput ui_button(int32_t id, int32_t x, int32_t y, int32_t width, int32_t height) {
    UIBoxFlags flags =  UIBoxFlags_DrawBackground | 
                        UIBoxFlags_DrawDropShadow |
                        UIBoxFlags_Clickable |
                        UIBoxFlags_HotAnimation |
                        UIBoxFlags_ActiveAnimation;

    ui_box_t *box = ui_box(id, x, y, width, height, flags);

    UIInput result = ui_input_from_box(box);
    return result;
}
UIInput ui_checkbox(int32_t id, int32_t x, int32_t y, int32_t width, int32_t height, int32_t *value) {
    UIBoxFlags flags =  UIBoxFlags_DrawBackground | 
                        UIBoxFlags_DrawDropShadow |
                        UIBoxFlags_Clickable |
                        UIBoxFlags_HotAnimation |
                        UIBoxFlags_ActiveAnimation;

    ui_box_t *box = ui_box(id, x, y, width, height, flags);

    UIInput result = ui_input_from_box(box);

    UIStyle style = ui_copy_style();
    style.background_color = *value ? (vec4_t) { 0.1f, 0.9f, 0.1f, 1 } : (vec4_t) { 0.9f, 0.1f, 0.1f, 1 };
    
    ui_push_style(style);
    ui_box_t *check_box = ui_box(0, x+2, y+2, width-4, height-4, UIBoxFlags_DrawBackground);
    ui_pop_style(style);

    if (result.mouse_pressed)
        *value = !*value;

    return result;
}
int8_t ui_slider(int32_t id, int32_t x, int32_t y, int32_t width, int32_t height, int32_t max, int32_t *value) {
    UIBoxFlags flags =  UIBoxFlags_DrawBackground | 
                        UIBoxFlags_DrawDropShadow |
                        UIBoxFlags_Clickable |
                        UIBoxFlags_HotAnimation |
                        UIBoxFlags_ActiveAnimation;

    ui_box_t *box = ui_box(id, x, y, width, height, flags);

    UIInput result = ui_input_from_box(box);

    int32_t btn_size = width / 2;
    int32_t y_pos = ((height - btn_size * 2) * (*value)) / max + btn_size / 2;

    UIStyle style = ui_copy_style();
    style.background_color = (vec4_t) { 0.9f, 0.1f, 0.1f, 1 };
    
    ui_push_style(style);
    ui_box_t *slider_box = ui_box(0, x + btn_size / 2, y + y_pos, btn_size, btn_size, UIBoxFlags_DrawBackground);
    ui_pop_style(style);

    uint8_t updated = 0;

    if (result.mouse_down){
        int mouse_pos = ui_state.mouse_pos.y - y;
        if (mouse_pos < btn_size) mouse_pos = btn_size;
        if (mouse_pos > height-btn_size) mouse_pos = height-btn_size;
        int v = ((mouse_pos - btn_size) * max) / (height-btn_size * 2);
        if (v != *value) {
            *value = v;
            updated = 1;
        }
    }

    return updated;
}
void ui_panel(int32_t x, int32_t y, int32_t width, int32_t height) {
    UIBoxFlags flags =  UIBoxFlags_DrawBackground | 
                        UIBoxFlags_DrawDropShadow;

    UIStyle style = ui_copy_style();
    style.background_color = style.panel_color;
    ui_push_style(style);
    ui_box_t *box = ui_box(0, x, y, width, height, flags);
    ui_pop_style();
}
void ui_push_style(UIStyle style) {
    UIStyleItem *style_item = arena_alloc(&ui_state.style_arena, 1, UIStyleItem);
    style_item->style = style;
    if (ui_state.style_stack == 0) {
        ui_state.style_stack = style_item;
        return;
    }

    style_item->next = ui_state.style_stack;
    ui_state.style_stack = style_item;
}
void ui_pop_style() {
    UTIL_ASSERT(ui_state.style_stack != 0, "Cannot pop non existing style");

    ui_state.style_stack = ui_state.style_stack->next;
}
UIStyle ui_copy_style() {
    UTIL_ASSERT(ui_state.style_stack != 0, "No styles exists");

    return ui_state.style_stack->style;
}
UIInput ui_input_from_box(ui_box_t *box) {
    UIInput result = { 0 };
    if (ui_region_hit(box->bounds) || ui_state.hot_item == box->id) {
        result.hover = 1;
        
        if (ui_state.active_item == 0 && ui_state.mouse_down) {
            result.mouse_pressed = 1;
        }
        if (ui_state.active_item == box->id && ui_state.mouse_down) {
            result.mouse_down = 1;
        }
    }
    return result;
}
int8_t ui_region_hit(vec4_t bounds) {
    return (ui_state.mouse_pos.x >= bounds.x && ui_state.mouse_pos.x < bounds.x + bounds.z && ui_state.mouse_pos.y >= bounds.y && ui_state.mouse_pos.y < bounds.y + bounds.w);
}