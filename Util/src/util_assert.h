#pragma once

#include <Windows.h>
#include <stdio.h>

#define UTIL_ASSERT(expr, msg) if (!(expr)) { _UTIL_ASSERT(#expr, __FILE__, __LINE__, msg); DebugBreak(); }

inline void _UTIL_ASSERT(const char* expr_str, const char* file, int line, const char* msg) {
    char buffer[1024] = {0};
    sprintf_s(buffer, 1024, "\nAssert failed: %s\nExpected: %s\nSource: %s, line %i\n", msg, expr_str, file, line);

    printf(buffer);
    //MessageBoxA(0, buffer, "Assertion", MB_OK | MB_ICONERROR);
}