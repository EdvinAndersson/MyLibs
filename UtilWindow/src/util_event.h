#pragma once

#define InvokeEvent(type, event)

enum WindowEventType {
    WindowEventType_Close,
    WindowEventType_Resize
} typedef WindowEventType;

struct WindowEvent_WindowEventType_Close {
    int i;
} typedef WindowEvent_WindowEventType_Close;

struct WindowEvent_WindowEventType_Resize {
    int width, height;
} typedef WindowEvent_WindowEventType_Resize;