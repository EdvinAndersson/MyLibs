#include "util_window.h"

Window *g_window;

#define _WINDOW_INVOKE_EVENT(type) _window_invoke_event(type, arena_alloc(&g_window->refreshed_arena, 1, WindowEvent_##type))

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SIZE: {
            WindowEvent_WindowEventType_Resize *e = _WINDOW_INVOKE_EVENT(WindowEventType_Resize);
            e->width = LOWORD(lParam);
            e->height = HIWORD(lParam);
            g_window->window_size = (vec2_t) { (float) e->width, (float) e->height };
        } break;
        case WM_CLOSE: {
            WindowEvent_WindowEventType_Close* e = _WINDOW_INVOKE_EVENT(WindowEventType_Close);
            e->i = 0;
        } break;
        case WM_MOUSEMOVE: {
            g_window->mouse_pos = (vec2_t) { (float) GET_X_LPARAM(lParam), (float) GET_Y_LPARAM(lParam) };
        } break;
        case WM_DESTROY: {
            WindowEvent_WindowEventType_Close* e = _WINDOW_INVOKE_EVENT(WindowEventType_Close);
            e->i = 0;
        } break;
        case WM_LBUTTONDOWN: {
            g_window->input_mask |= WINDOW_MOUSE1_PRESSED;
            g_window->input_mask |= WINDOW_MOUSE1_HELD;
        } break;
        case WM_LBUTTONUP: {
            g_window->input_mask |= WINDOW_MOUSE1_RELEASE;
            g_window->input_mask = ~(~g_window->input_mask | WINDOW_MOUSE1_HELD);
        } break;
        case WM_RBUTTONDOWN: {
            g_window->input_mask |= WINDOW_MOUSE2_PRESSED;
            g_window->input_mask |= WINDOW_MOUSE2_HELD;
        } break;
        case WM_RBUTTONUP: {
            g_window->input_mask |= WINDOW_MOUSE2_RELEASE;
            g_window->input_mask = ~(~g_window->input_mask | WINDOW_MOUSE2_HELD);
        } break;
        case WM_MBUTTONDOWN: {
            g_window->input_mask |= WINDOW_MOUSE3_PRESSED;
            g_window->input_mask |= WINDOW_MOUSE3_HELD;
        } break;
        case WM_MBUTTONUP: {
            g_window->input_mask |= WINDOW_MOUSE3_RELEASE;
            g_window->input_mask = ~(~g_window->input_mask | WINDOW_MOUSE3_HELD);
        } break;
        case WM_QUIT: {
        } break;
        default: {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
    return 0;
}

Window* window_create(str_t title, uint32_t width, uint32_t height) {
    MemoryArena window_arena = arena_init(1024*32);

    g_window = arena_alloc(&window_arena, 1, Window);
    g_window->window_arena = window_arena;
    g_window->refreshed_arena = arena_init(1024*512);

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = "Sample Window Class";
    wc.hCursor = LoadCursorA(0, IDC_ARROW);

    RegisterClass(&wc);

    g_window->hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        str_to_cstr(&g_window->refreshed_arena, title),
        WS_OVERLAPPEDWINDOW,

        //Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL,
        NULL,
        GetModuleHandle(0),
        NULL
    );

    UTIL_ASSERT(g_window->hwnd != 0, "Window handle coult not be created!");

    g_window->hdc = GetDC(g_window->hwnd);

    _window_create_opengl_context();
    _window_load_opengl_functions();

    ShowWindow(g_window->hwnd, 1);

    LARGE_INTEGER temp_cpu_frequency;
    QueryPerformanceFrequency(&temp_cpu_frequency);
    g_window->cpu_frequency = (double) temp_cpu_frequency.QuadPart;

    LARGE_INTEGER temp;
    QueryPerformanceCounter(&temp);
    g_window->process_start_time = (double) temp.QuadPart / g_window->cpu_frequency;

    return g_window;
}

void window_poll_message() {
    arena_free(&g_window->refreshed_arena);
    g_window->event_queue = 0;

    uint32_t mask = g_window->input_mask & (WINDOW_MOUSE1_HELD | WINDOW_MOUSE2_HELD | WINDOW_MOUSE3_HELD);
    g_window->input_mask = mask;
    
    MSG msg = { 0 };
    
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

void window_swap_buffers() {
    SwapBuffers(g_window->hdc);
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
    return (g_window->input_mask & (uint32_t) input_type) == (uint32_t) input_type;
}
int window_key_input(uint32_t key_code) {
    return GetKeyState(key_code) & 0x8000;
}
vec2_t window_get_size() {
    return g_window->window_size;
}
void window_vsync(uint8_t enable) {
    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(enable);
}
double window_get_time() {
    LARGE_INTEGER current_counter;
    QueryPerformanceCounter(&current_counter);

    return (double) current_counter.QuadPart / g_window->cpu_frequency;
}

void _window_create_opengl_context() {
    _window_load_wgl_functions();

    int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     24,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,
        0,
    };
    int format;
    UINT formats;
    if (!g_window->wglChoosePixelFormatARB(g_window->hdc, pixel_format_attribs, NULL, 1, &format, &formats) || formats == 0) {
        UTIL_ASSERT(0, "OpenGL does not support required pixel format!");
    }

    PIXELFORMATDESCRIPTOR desc;
    desc.nSize = sizeof(desc);
    BOOL ok = DescribePixelFormat(g_window->hdc, format, sizeof(desc), &desc);
    UTIL_ASSERT(ok, "Failed to describe OpenGL pixel format");

    if (!SetPixelFormat(g_window->hdc, format, &desc)) {
        UTIL_ASSERT(0, "Cannot set OpenGL selected pixel format!");
    }

    // Create modern OpenGL context
    int attrib[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifndef NDEBUG
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0,
    };

    HGLRC rc = g_window->wglCreateContextAttribsARB(g_window->hdc, NULL, attrib);
    if (!rc) {
        UTIL_ASSERT(0, "Cannot create modern OpenGL context! OpenGL version 4.5 not supported?");
    }

    ok = wglMakeCurrent(g_window->hdc, rc);
    UTIL_ASSERT(ok, "Failed to make current OpenGL context");

    printf("OPENGL VERSION: %s\n", (char *)glGetString(GL_VERSION));
}
void _window_load_wgl_functions() {
    // To get WGL functions we need valid GL context, so create dummy window for dummy GL contetx
    HWND dummy = CreateWindowExW(
        0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, NULL, NULL);
    UTIL_ASSERT(dummy, "Failed to create dummy window");

    HDC dc = GetDC(dummy);
    UTIL_ASSERT(dc, "Failed to get device context for dummy window");

    PIXELFORMATDESCRIPTOR desc;
    desc.nSize = sizeof(desc);
    desc.nVersion = 1;
    desc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desc.iPixelType = PFD_TYPE_RGBA;
    desc.cColorBits = 24;
    
    int format = ChoosePixelFormat(dc, &desc);
    if (!format) {
        UTIL_ASSERT(0, "Cannot choose OpenGL pixel format for dummy window!");
    }

    int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
    UTIL_ASSERT(ok, "Failed to describe OpenGL pixel format");

    // reason to create dummy window is that SetPixelFormat can be called only once for the window
    if (!SetPixelFormat(dc, format, &desc)) {
        UTIL_ASSERT(0, "Cannot set OpenGL pixel format for dummy window!");
    }

    HGLRC rc = wglCreateContext(dc);
    UTIL_ASSERT(rc, "Failed to create OpenGL context for dummy window");

    ok = wglMakeCurrent(dc, rc);
    UTIL_ASSERT(ok, "Failed to make current OpenGL context for dummy window");

    // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_extensions_string.txt
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC) wglGetProcAddress("wglGetExtensionsStringARB");
    if (!wglGetExtensionsStringARB) {
        UTIL_ASSERT(0, "OpenGL does not support WGL_ARB_extensions_string extension!");
    }

    const char* ext = wglGetExtensionsStringARB(dc);
    UTIL_ASSERT(ext, "Failed to get OpenGL WGL extension string");

    const char* start = ext;
    for (;;)
    {
        while (*ext != 0 && *ext != ' ')
        {
            ext++;
        }

        size_t length = ext - start;
        if (_string_are_equal("WGL_ARB_pixel_format", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
            g_window->wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");
        }
        else if (_string_are_equal("WGL_ARB_create_context", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
            g_window->wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
        }

        if (*ext == 0)
        {
            break;
        }

        ext++;
        start = ext;
    }

    if (!g_window->wglChoosePixelFormatARB || !g_window->wglCreateContextAttribsARB) {
        UTIL_ASSERT(0, "OpenGL does not support required WGL extensions for modern context!");
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(rc);
    ReleaseDC(dummy, dc);
    DestroyWindow(dummy);
}

void _window_load_opengl_functions() {
    #undef X
    #define X(proc, name) name=(proc)wglGetProcAddress(#name);
    GLFUNCTIONS
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

int _string_are_equal(const char* src, const char* dst, size_t dstlen)
	{
		while (*src && dstlen-- && *dst)
		{
			if (*src++ != *dst++)
			{
				return 0;
			}
		}

		return (dstlen && *src == *dst) || (!dstlen && *src == 0);
	}