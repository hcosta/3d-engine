#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>

// Fotogramas que vamos a forzar por segundo
#define FPS 30

// Cuantos milisegundos deben pasar entre cada fotograma
#define FRAME_TARGET_TIME (1000 / FPS)

enum cull_method
{
    CULL_NONE,
    CULL_BACKFACE
} cull_method;

enum render_method
{
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE
} render_method;

extern bool is_fullscreen;
extern int window_width;
extern int window_height;
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern uint32_t *color_buffer;
extern SDL_Texture *color_buffer_texture;

bool initialize_window(void);
void destroy_window(void);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void draw_grid(void);
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);

#endif