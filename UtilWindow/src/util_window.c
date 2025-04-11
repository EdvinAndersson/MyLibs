#include "util_window.h"

Window *g_window;

#define _WINDOW_INVOKE_EVENT(type) _window_invoke_event(type, arena_alloc(&g_window->refreshed_arena, 1, WindowEvent_##type))

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SIZE: {
            WindowEvent_WindowEventType_Resize *e = _WINDOW_INVOKE_EVENT(WindowEventType_Resize);
            e->width = LOWORD(lParam);
            e->height = HIWORD(lParam);
        } break;
        case WM_CLOSE: {
            WindowEvent_WindowEventType_Close* e = _WINDOW_INVOKE_EVENT(WindowEventType_Close);
            e->i = 0;
        } break;
        case WM_MOUSEMOVE: {
            g_window->mouse_pos = (vec2_t) { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        } break;
        case WM_DESTROY: {
            WindowEvent_WindowEventType_Close* e = _WINDOW_INVOKE_EVENT(WindowEventType_Close);
            e->i = 0;
        } break;
        case WM_LBUTTONDOWN: {
            g_window->input_mask |= WINDOW_MOUSE1_DOWN;
        } break;
        case WM_LBUTTONUP: {
            g_window->input_mask |= WINDOW_MOUSE1_RELEASE;
        } break;
        case WM_RBUTTONDOWN: {
            g_window->input_mask |= WINDOW_MOUSE2_DOWN;
        } break;
        case WM_RBUTTONUP: {
            g_window->input_mask |= WINDOW_MOUSE2_RELEASE;
        } break;
        case WM_MBUTTONDOWN: {
            g_window->input_mask |= WINDOW_MOUSE3_DOWN;
        } break;
        case WM_MBUTTONUP: {
            g_window->input_mask |= WINDOW_MOUSE3_RELEASE;
        } break;
        case WM_QUIT: {
        } break;
        default: {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    return 0;
}

Window* window_create(MemoryArena *arena, str_t title) {
    g_window = arena_alloc(arena, 1, Window);
    g_window->refreshed_arena = arena_init(1024*512);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = "Sample Window Class";

    RegisterClass(&wc);

    g_window->hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        str_to_cstr(arena, title),
        WS_OVERLAPPEDWINDOW,

        //Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandle(0),
        NULL
    );

    UTIL_ASSERT(g_window->hwnd != 0, "Window handle coult not be created!");

    ShowWindow(g_window->hwnd, 1);

    return g_window;
}

void window_poll_message() {
    arena_free(&g_window->refreshed_arena);
    g_window->event_queue = 0;

    g_window->input_mask = 0;
    
    MSG msg = { 0 };
    
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

int window_event_exists() {
    if (g_window->event_queue != 0)
        return 1;
    return 0;
}
WindowEvent window_event_pop() {
    WindowEvent result = g_window->event_queue->window_event;
    if (g_window->event_queue->next != 0){
        g_window->event_queue = g_window->event_queue->next;
    } else {
        g_window->event_queue = 0;
    }
    return result;
}
int window_mouse_input(WindowMouseInput input_type) {
    return (g_window->input_mask & input_type) == input_type;
}

void _window_enqueue(WindowEventQueue *e_queue) {
    if (g_window->event_queue == 0){
        g_window->event_queue = e_queue;
        return;
    }

    WindowEventQueue *previous = 0;
    WindowEventQueue *current = g_window->event_queue;
    while (current != 0) {
        previous = current;
        current = current->next;
    }
    if (previous != 0) {
        previous->next = e_queue;
    }
}

void* _window_invoke_event(WindowEventType type, void *event) {
    WindowEventQueue *e_queue = arena_alloc(&g_window->refreshed_arena, 1, WindowEventQueue);
    e_queue->window_event.type = type;
    e_queue->window_event.event = event;

    _window_enqueue(e_queue);

    return e_queue->window_event.event;
}