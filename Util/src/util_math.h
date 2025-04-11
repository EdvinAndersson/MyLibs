#pragma once
#pragma warning(disable:4201)

#include <math.h>

#define PI 3.14159265358979323846264338327950288

typedef struct vec2_t {
    union {
        struct {
            float x, y;
        };
        struct {
            float width, height;
        };
        float xy[2];
    };
} vec2_t;

typedef struct vec3_t {
    union {
        struct {
            float x, y, z;
        };
        struct {
            float r, g, b;
        };
        float xyz[3];
        float rgb[3];
    };
} vec3_t;

inline vec2_t vec2_add(vec2_t a, vec2_t b) {
    return (vec2_t) { a.x + b.x, a.y + b.y };
}
inline vec2_t vec2_sub(vec2_t a, vec2_t b) {
    return (vec2_t) { a.x - b.x, a.y - b.y };
}
inline float vec2_dot_pro(vec2_t a, vec2_t b) {
	return a.x * b.x + a.y * b.y;
}
inline vec2_t vec2_div(vec2_t a, float b) {
	return (vec2_t) { a.x / b, a.y / b };
}
inline float vec2_length(vec2_t a) {
	return sqrtf(powf(a.x, 2) + powf(a.y, 2));
}
inline vec2_t vec2_normalize(vec2_t a) {
	return vec2_div(a, vec2_length(a));
}