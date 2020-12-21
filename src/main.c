#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// Globales
bool is_running = false;
bool is_fullscreen = false;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int window_width = 800;
int window_height = 600;

// Puntero para array uint32 (32 bits / 4 bytes por entero garantizados)
uint32_t *color_buffer = NULL;

// Textura SDL utilizada para mostrar en el color buffer
SDL_Texture *color_buffer_texture = NULL;

bool initialize_window(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        // Debug en el buffer de errores
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    // Establecemos ancho y alto de la ventana SDL a la resolución máxima de la pantalla
    if (is_fullscreen)
    {
        SDL_DisplayMode display_mode;
        SDL_GetCurrentDisplayMode(0, &display_mode);
        window_width = display_mode.w;
        window_height = display_mode.h;
    }

    // Crear ventana SDL
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width, window_height,
        SDL_WINDOW_BORDERLESS);

    if (!window)
    {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    // Crear renderer SDL
    renderer = SDL_CreateRenderer(window, -1, 0); // -1 primer output disponible

    if (!renderer)
    {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    return true;
}

void setup(void)
{
    // Asigno bytes requeridos en memoria para el color buffer
    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

    // Creo la textura donde copiaremos el color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width, window_height);
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

void draw_grid(void)
{
    // Dibujar una cuadrícula que rellena el espacio
    // Las líneas deben concordar con las filas y columnas múltiples de 10

    // Forma con condicionales
    // for (int y = 0; y < window_height; y++)
    // {
    //     for (int x = 0; x < window_width; x++)
    //     {
    //         if (x % 10 == 0 || y % 10 == 0)
    //             color_buffer[(window_width * y) + x] = 0xFF333333;
    //     }
    // }

    // Forma alternativa
    for (int y = 0; y < window_height; y += 10)
    {
        for (int x = 0; x < window_width; x += 10)
        {
            if (x != 0 && y != 0) // Esto esconde la primera fila y columna
                color_buffer[(window_width * y) + x] = 0xFF333333;
        }
    }

    // Forma sin condicionales con offset
    //int offset = 10;

    // Draw Horizontal Lines
    // for (int y = offset / 2; y < window_height; y += offset)
    // {
    //     for (int x = 0; x < window_width; x += 1)
    //     {
    //         color_buffer[(window_width * y) + x] = 0xFF333333;
    //     }
    // }

    // // Draw Vertical Lines
    // for (int y = 0; y < window_height; y += 1)
    // {
    //     for (int x = offset / 2; x < window_width; x += offset)
    //     {
    //         color_buffer[(window_width * y) + x] = 0xFF333333;
    //     }
    // }
}

void clear_color_buffer(uint32_t color)
{
    for (int y = 0; y < window_height; y++)
    {
        for (int x = 0; x < window_width; x++)
        {
            color_buffer[(window_width * y) + x] = color;
        }
    }
}

// Renderiza el array color buffer en la textura y la muestra
void render_color_buffer()
{
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL, // https://wiki.libsdl.org/SDL_Rect
        color_buffer,
        (int)(window_width * sizeof(uint32_t)));

    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void render(void)
{
    SDL_SetRenderDrawColor(renderer, 150, 150, 0, 255);
    SDL_RenderClear(renderer);

    // Dibujamos la cuadrícula
    draw_grid();

    // Copiamos el color buffer a la textura y lo limpiamos
    render_color_buffer();
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

void destroy_window(void)
{
    // El array sabe cuantos elementos lo forman, aqui liberamos la memoria
    free(color_buffer);

    // C no tiene recolector de basura... tenemos que liberar memoria nosotros
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    is_running = initialize_window();

    setup();

    while (is_running)
    {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}