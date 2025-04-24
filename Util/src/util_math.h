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

typedef struct vec4_t {
    union {
        struct {
            float r, g, b, a;
        };
        struct {
            float x, y, z, w;
        };
        float rgba[4];
    };
} vec4_t;

typedef struct mat4_t {
    float m[16];
} mat4_t;

inline vec2_t vec2_add(vec2_t a, vec2_t b) {
    return (vec2_t) { a.x + b.x, a.y + b.y };
}
inline vec2_t vec2_sub(vec2_t a, vec2_t b) {
    return (vec2_t) { a.x - b.x, a.y - b.y };
}
inline float vec2_dot_pro(vec2_t a, vec2_t b) {
	return a.x * b.x + a.y * b.y;
}
inline vec2_t vec2_mul(vec2_t a, float b) {
	return (vec2_t) { a.x * b, a.y * b };
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
inline vec2_t vec2_rotate(vec2_t v, float rotation_rad) {
	return (vec2_t) {
		cosf(rotation_rad) * v.x - sinf(rotation_rad) * v.y,
		sinf(rotation_rad) *v.x + cosf(rotation_rad) * v.y
	};
}

inline mat4_t mat4_identity() {
	mat4_t out = { 0 };
	out.m[0] = 1;
	out.m[5] = 1;
	out.m[10] = 1;
	out.m[15] = 1;
	return out;
}
inline mat4_t mat4_translate_v3(mat4_t mat, vec3_t a) {
	if (a.x == 0 && a.y == 0 && a.z == 0) return mat;

    mat.m[12] += a.x;
    mat.m[13] += a.y;
    mat.m[14] += a.z;
    
    return mat;
}
inline mat4_t mat4_scale_v3(mat4_t mat, vec3_t a) {
    mat.m[0] *= a.x;
    mat.m[1] *= a.x;
    mat.m[2] *= a.x;
    mat.m[3] *= a.x;

    mat.m[4] *= a.y;
    mat.m[5] *= a.y;
    mat.m[6] *= a.y;
    mat.m[7] *= a.y;

    mat.m[8] *= a.z;
    mat.m[9] *= a.z;
    mat.m[10] *= a.z;
    mat.m[11] *= a.z;

    return mat;
}

inline mat4_t mat4_ortho(float left, float right, float bottom, float top, float n, float f) {
	mat4_t out = { 0 };

	out.m[0 * 4 + 0] = 2 / (right - left);
	out.m[1 * 4 + 1] = 2 / (top - bottom);
	out.m[2 * 4 + 2] = -2 / (f - n);

	out.m[3 * 4 + 0] = -(right + left) / (right - left);
	out.m[3 * 4 + 1] = -(top + bottom) / (top - bottom);
	out.m[3 * 4 + 2] = -(f + n) / (f - n);
	out.m[3 * 4 + 3] = 1;

	return out;
}