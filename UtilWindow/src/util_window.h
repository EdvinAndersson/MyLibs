#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>

#include "Util/src/util_memory_arena.h"
#include "Util/src/util_assert.h"
#include "Util/src/util_string.h"
#include "Util/src/util_math.h"

#include "util_opengl_bindings.h"
#include "util_event.h"
#include "util_keycodes.h"

typedef struct WindowEvent {
    WindowEventType type;
    void *event;
} WindowEvent;

typedef struct WindowEventQueue {
    struct WindowEventQueue *next;
    WindowEvent window_event;
} WindowEventQueue;

typedef enum WindowMouseInput {
    WINDOW_MOUSE1_PRESSED = 1 << 0,
    WINDOW_MOUSE1_HELD = 1 << 1,
    WINDOW_MOUSE1_RELEASE = 1 << 2,
    WINDOW_MOUSE2_PRESSED = 1 << 3,
    WINDOW_MOUSE2_HELD = 1 << 4,
    WINDOW_MOUSE2_RELEASE = 1 << 5,
    WINDOW_MOUSE3_PRESSED = 1 << 6,
    WINDOW_MOUSE3_HELD = 1 << 7,
    WINDOW_MOUSE3_RELEASE = 1 << 8,
} WindowMouseInput;

typedef struct Window {
    HWND hwnd;
    HDC hdc;
    MemoryArena refreshed_arena;
    WindowEventQueue *event_queue;

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;

    //Time
    double cpu_frequency;
    double process_start_time;

    //Input
    vec2_t mouse_pos;
    uint32_t input_mask;

    vec2_t window_size;
} Window;

Window* window_create(MemoryArena *arena, str_t title, uint32_t width, uint32_t height);
void window_poll_message();
void window_swap_buffers();
int window_event_exists();
WindowEvent window_event_pop();
int window_mouse_input(WindowMouseInput input_type);
int window_key_input(uint32_t key_code);
vec2_t window_get_size();
void window_vsync(uint8_t enable);
double window_get_time();

void _window_create_opengl_context();
void _window_load_wgl_functions();
void _window_load_opengl_functions();

void _window_enqueue(WindowEventQueue *queue);
void* _window_invoke_event(WindowEventType type, void *event);
int _string_are_equal(const char* src, const char* dst, size_t dstlen);