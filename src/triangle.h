#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"
#include <stdint.h>

typedef struct
{
    int a;
    int b;
    int c;
    int32_t color;
} face_t;

typedef struct
{
    vec2_t points[3];
    int32_t color;
} triangle_t;

void int_swap(int *a, int *b); //
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void draw_filled_triangle(int x, int y0, int x1, int y1, int x2, int y2, uint32_t color);

#endif