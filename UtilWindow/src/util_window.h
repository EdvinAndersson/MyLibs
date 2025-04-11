#pragma once

#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>

#include "Util/src/util_memory_arena.h"
#include "Util/src/util_assert.h"
#include "Util/src/util_string.h"
#include "Util/src/util_math.h"

#include "util_event.h"

typedef struct WindowEvent {
    WindowEventType type;
    void *event;
} WindowEvent;

typedef struct WindowEventQueue {
    struct WindowEventQueue *next;
    WindowEvent window_event;
} WindowEventQueue;

typedef enum WindowMouseInput {
    WINDOW_MOUSE1_DOWN = 1 << 0,
    WINDOW_MOUSE1_RELEASE = 1 << 1,
    WINDOW_MOUSE2_DOWN = 1 << 2,
    WINDOW_MOUSE2_RELEASE = 1 << 3,
    WINDOW_MOUSE3_DOWN = 1 << 4,
    WINDOW_MOUSE3_RELEASE = 1 << 5,
} WindowMouseInput;

typedef struct Window {
    HWND hwnd;
    MemoryArena refreshed_arena;
    WindowEventQueue *event_queue;
    vec2_t mouse_pos;
    uint32_t input_mask;
} Window;

Window* window_create(MemoryArena *arena, str_t title);
void window_poll_message();
int window_event_exists();
WindowEvent window_event_pop();
int window_mouse_input(WindowMouseInput input_type);

void _window_enqueue(WindowEventQueue *queue);
void* _window_invoke_event(WindowEventType type, void *event);