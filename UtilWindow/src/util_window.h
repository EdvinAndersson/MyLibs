#pragma once

#include <Windows.h>

#include "Util/src/util_memory_arena.h"
#include "Util/src/util_assert.h"
#include "Util/src/util_string.h"

#include "util_event.h"

struct WindowEvent {
    WindowEventType type;
    void *event;
} typedef WindowEvent;

struct WindowEventQueue {
    struct WindowEventQueue *next;
    WindowEvent window_event;
} typedef WindowEventQueue;

struct Window {
    HWND hwnd;
    MemoryArena refreshed_arena;
    WindowEventQueue *event_queue;
} typedef Window;

Window* window_create(MemoryArena *arena, str_t title);
void window_poll_message();
int window_event_exists();
WindowEvent window_event_pop();

void _window_enqueue(WindowEventQueue *queue);
void* _window_invoke_event(WindowEventType type, void *event);