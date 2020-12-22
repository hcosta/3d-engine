#include "display.h"

// Globales
bool is_fullscreen = false;
int window_width = 800;
int window_height = 600;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
uint32_t *color_buffer = NULL;            // Puntero para array uint32 (32 bits / 4 bytes por entero)
SDL_Texture *color_buffer_texture = NULL; // Textura SDL utilizada para mostrar en el color buffer

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

void destroy_window(void)
{
    // El array sabe cuantos elementos lo forman, aqui liberamos la memoria
    free(color_buffer);

    // C no tiene recolector de basura... tenemos que liberar memoria nosotros
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
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
void render_color_buffer(void)
{
    SDL_UpdateTexture(
        color_buffer_texture,
        NULL, // https://wiki.libsdl.org/SDL_Rect
        color_buffer,
        (int)(window_width * sizeof(uint32_t)));

    SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
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

void draw_rectangle(int x, int y, int width, int height, uint32_t color)
{
    // Mi solución
    // for (int j = y; j < y + height; j++)
    // {
    //     for (int i = x; i < x + width; i++)
    //     {
    //         color_buffer[(window_width * j) + i] = color;
    //     }
    // }

    // Solución con bucles en cero
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int current_x = x + i;
            int current_y = y + j;
            color_buffer[(window_width * current_y) + current_x] = color;
        }
    }
}
