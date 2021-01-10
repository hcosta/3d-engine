#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <stdint.h>
#include "vector.h"
#include "texture.h"
#include "upng.h"

typedef struct face_t
{
    int a;
    int b;
    int c;
    tex2_t a_uv;
    tex2_t b_uv;
    tex2_t c_uv;
    int32_t color;
} face_t;

typedef struct triangle_t
{
    vec4_t points[3];
    tex2_t texcoords[3];
    int32_t color;
    upng_t *texture;
} triangle_t;

void int_swap(int *a, int *b);

void draw_triangle(
    int x0, int y0,
    int x1, int y1,
    int x2, int y2,
    uint32_t color);

void draw_triangle_texel(
    int x, int y, upng_t *texture,
    vec4_t point_a, vec4_t point_b, vec4_t point_c,
    tex2_t a_uv, tex2_t b_uv, tex2_t c_uv);

void draw_triangle_pixel(
    int x, int y, uint32_t color,
    vec4_t point_a, vec4_t point_b, vec4_t point_c);

void draw_filled_triangle(
    int x0, int y0, float z0, float w0,
    int x1, int y1, float z1, float w1,
    int x2, int y2, float z2, float w2,
    uint32_t color);

void draw_textured_triangle(
    int x0, int y0, float z0, float w0, float u0, float v0,
    int x1, int y1, float z1, float w1, float u1, float v1,
    int x2, int y2, float z2, float w2, float u2, float v2,
    upng_t *texture);

vec3_t get_triangle_normal(vec4_t vertices[3]);

#endif

// void fill_flat_bottom_triangle(
//     int x0, int y0,
//     int x1, int y1,
//     int x2, int y2,
//     uint32_t color);

// void fill_flat_top_triangle(
//     int x0, int y0,
//     int x1, int y1,
//     int x2, int y2,
//     uint32_t color);