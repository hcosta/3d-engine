#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// Fotogramas que vamos a forzar por segundo
#define FPS 60

// Cuantos milisegundos deben pasar entre cada fotograma
#define FRAME_TARGET_TIME (1000 / FPS)

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

#endif