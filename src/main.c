#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"

// Globales
bool is_running = false;

// Array de vectores/puntos
// const int N_POINTS = 9 * 9 * 9; // 9x9x9 cube
#define N_POINTS (9 * 9 * 9)
vec3_t cube_points[N_POINTS];

void setup(void)
{
    // Asigno bytes requeridos en memoria para el color buffer
    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

    // Creo la textura donde copiaremos el color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height);

    int point_count = 0;

    // Empezar a cargar el array de vectores
    // De -1 a 1 (en el cubo 9x9x9)
    for (float x = -1; x <= 1; x += 0.25)
    {
        for (float y = -1; y <= 1; y += 0.25)
        {
            for (float z = -1; z <= 1; z += 0.25)
            {
                vec3_t new_point = {.x = x, .y = y, .z = z};
                cube_points[point_count++] = new_point;
            }
        }
    }
}

void process_input(void)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
    case SDL_QUIT:
        is_running = false;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
            is_running = false;
        break;
    }
}

void update(void)
{
    // TODO:
}

void render(void)
{
    SDL_SetRenderDrawColor(renderer, 150, 150, 0, 255);
    SDL_RenderClear(renderer);

    // Dibujamos la cuadrícula
    draw_grid();
    draw_pixel(20, 20, 0xFFFFFF00);
    draw_rectangle(100, 100, 250, 125, 0xFFFA68D8);

    // Copiamos el color buffer a la textura y lo limpiamos
    render_color_buffer();
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    is_running = initialize_window();

    setup();

    // vec3_t myvector = {2.0, 3.0, -4.0};

    while (is_running)
    {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}