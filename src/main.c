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
vec2_t projected_points[N_POINTS];
vec3_t camera_position = {.x = 0, .y = 0, .z = -5};

// Field of view factor (factor de reescalado)
float fov_factor = 640;

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

// Transforma un vec3 a vec2, proyección paralela ortográfica
vec2_t project(vec3_t point)
{

    // El ratio entre los lados de triángulos similares es el mismo
    // Cuanto mayor es la profundidad Z, menor es el escalado
    // Eso genera el efecto de reducir el tamaño y dar la profundidad
    // Fórmulas: docs/01 Proyeccion_Perspectiva.png
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z};

    return projected_point;
}

void update(void)
{
    for (int i = 0; i < N_POINTS; i++)
    {
        vec3_t point = cube_points[i];

        // Movemos los puntos lejos de la cámara
        point.z -= camera_position.z;

        // Proyectamos el punto actual de 3D a 2D
        vec2_t projected_point = project(point);

        // Guardamos el vector 2D proyectado en el array de puntos proyectados
        projected_points[i] = projected_point;
    }
}

void render(void)
{
    // No necesitamos esto al usar el color buffer
    // SDL_SetRenderDrawColor(renderer, 150, 150, 0, 255);
    // SDL_RenderClear(renderer);

    // Dibujamos la cuadrícula
    draw_grid();

    // draw_pixel(20, 20, 0xFFFFFF00);
    // draw_rect(100, 100, 250, 125, 0xFFFA68D8);

    // Iteramos los puntos proyectados y los renderizamos
    for (int i = 0; i < N_POINTS; i++)
    {
        vec2_t projected_point = projected_points[i];
        // Dibujamos un pequeño rectángulo en lugar de un punto
        // Le añadiremos un offset para empezar en medio de la pantalla
        draw_rect(
            projected_point.x + (window_width / 2),
            projected_point.y + (window_height / 2),
            4, 4,
            0xFFFFFF00);
    }

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