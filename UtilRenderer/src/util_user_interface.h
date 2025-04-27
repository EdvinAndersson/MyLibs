#pragma once

#include "Util/src/util_math.h"
#include "Util/src/util_memory_arena.h"

#include "UtilWindow/src/util_window.h"
#include "util_renderer2d.h"

#ifdef IMGUI_SRC_ID
    #define GEN_ID ((IMGUI_SRC_ID) + (__LINE__))
#else
    #define GEN_ID (__LINE__)
#endif

typedef struct UIStyle {
    vec4_t background_color;
    vec4_t hot_color;
    vec4_t active_color;
    vec4_t panel_color;
    int32_t shadow_offset;
} UIStyle;

typedef struct UIStyleItem {
    UIStyle style;
    struct UIStyleItem *next;
} UIStyleItem;

typedef struct UIState {
    vec2_t mouse_pos;
    int32_t mouse_down;

    int32_t hot_item;
    int32_t active_item;

    MemoryArena box_arena;
    uint32_t ui_box_count;

    MemoryArena style_arena;
    UIStyleItem *style_stack;

    uint8_t show_debug_lines;
} UIState;

typedef struct UIInput {
    uint8_t hover;
    uint8_t mouse_down;
    uint8_t mouse_pressed;
} UIInput;

typedef enum UIBoxFlags {
    UIBoxFlags_DrawBackground = (1 << 0),
    UIBoxFlags_DrawDropShadow = (1 << 1),
    UIBoxFlags_Clickable = (1 << 2),
    UIBoxFlags_HotAnimation = (1 << 3),
    UIBoxFlags_ActiveAnimation = (1 << 4)
} UIBoxFlags;

typedef struct ui_box {
    int32_t id;
    vec4_t bounds;
    UIBoxFlags flags;
    UIStyle style;
} ui_box_t;

extern UIState ui_state;

void ui_begin(Window *window);
void ui_end();

ui_box_t* ui_box(int32_t id, int32_t x, int32_t y, int32_t width, int32_t height, UIBoxFlags flags);
UIInput ui_button(int32_t id, int32_t x, int32_t y, int32_t width, int32_t height);
UIInput ui_checkbox(int32_t id, int32_t x, int32_t y, int32_t width, int32_t height, int32_t *value);
uint8_t ui_slider(int32_t id, int32_t x, int32_t y, int32_t width, int32_t height, int32_t max, int32_t *value);
void ui_panel(int32_t x, int32_t y, int32_t width, int32_t height);

void ui_push_style(UIStyle style);
void ui_pop_style();
UIStyle ui_copy_style();
UIInput ui_input_from_box(ui_box_t *box);
int8_t ui_region_hit(vec4_t bounds);