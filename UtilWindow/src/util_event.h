#pragma once

#define InvokeEvent(type, event)

enum WindowEventType {
    WindowEventType_Close,
    WindowEventType_Resize,
    WindowEventType_MouseMove
} typedef WindowEventType;

struct WindowEvent_WindowEventType_Close {
    int i;
} typedef WindowEvent_WindowEventType_Close;

struct WindowEvent_WindowEventType_Resize {
    int width, height;
} typedef WindowEvent_WindowEventType_Resize;

struct WindowEvent_WindowEventType_MouseMove {
    int x, y;
} typedef WindowEvent_WindowEventType_MouseMove;