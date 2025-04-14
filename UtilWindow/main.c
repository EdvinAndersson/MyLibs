#include "src/util_window.h"

int main() {
    MemoryArena arena = arena_init(1024*1024);

    Window *window = window_create(&arena, str_create("A Title"));
    
    int running = 1;
    do {
        window_poll_message();

        while (window_event_exists() != 0) {
            WindowEvent window_event = window_event_pop();
            switch (window_event.type)
            {
                case WindowEventType_Close: {
                    WindowEvent_WindowEventType_Close *e = window_event.event;
                    printf("WindowEventType_Close: Data: %i\n", e->i);
                    running = 0;
                } break;
                case WindowEventType_Resize: {
                    WindowEvent_WindowEventType_Resize *e = window_event.event;
                    printf("WindowEventType_Resize: Width: %i, height: %i\n", e->width, e->height);
                } break;
            }
        }

        if (window_mouse_input(WINDOW_MOUSE1_PRESSED)) {
            printf("Mouse 1 down\n");
        }
        if (window_mouse_input(WINDOW_MOUSE1_RELEASE)) {
            printf("Mouse 1 release\n");
        }
        if (window_mouse_input(WINDOW_MOUSE1_HELD)) {
            printf("Mouse 1 held\n");
        }
        if (window_key_input(KEY_0)) {
            printf("0\n");
        }
        if (window_key_input(KEY_A)) {
            printf("A\n");
        }

    } while (running > 0);

    printf("DONE\n");
    return 0;
}