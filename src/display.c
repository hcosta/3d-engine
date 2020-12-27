#include "display.h"

// Globales
bool is_fullscreen = false;
int window_width = 1024;
int window_height = 800;
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

// C no tiene recolector de basura... tenemos que liberar memoria nosotros
void destroy_window(void)
{

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
    for (int y = 0; y < window_height; y += 10)
    {
        for (int x = 0; x < window_width; x += 10)
        {
            if (x != 0 && y != 0) // Esto esconde la primera fila y columna
                color_buffer[(window_width * y) + x] = 0xFF333333;
        }
    }
}

void draw_pixel(int x, int y, uint32_t color)
{
    // Dibujamos el píxel si está dentro de la ventana
    if (x >= 0 && x < window_width && y >= 0 && y < window_height)
    {
        color_buffer[window_width * y + x] = color;
    }
}

// Algoritmo DDA: https://es.wikipedia.org/wiki/Analizador_diferencial_digital
void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
    // Alternativamente es mejor usar el algoritmo Bresenham por ser más eficiente
    // al evitar el uso de flotantes y resolver la tarea únicamente con enteros

    int delta_x = (x1 - x0); // longitud horizontal
    int delta_y = (y1 - y0); // longitud vertical

    // Asignamos la longitud más grande para encontrar el número de pasos (píxeles a dibujar)
    int longest_side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

    // Buscamos cuanto debemos incrementar 'x' e 'y' en cada paso
    float x_inc = delta_x / (float)longest_side_length;
    float y_inc = delta_y / (float)longest_side_length;

    float current_x = x0;
    float current_y = y0;

    // Redondeamos a entero las nuevas posiciones en cada paso para dibujarlas ahí
    for (int i = 0; i <= longest_side_length; i++)
    {
        // Redondeamos para dibujar cada píxel ()
        draw_pixel(round(current_x), round(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color); // completamos el polígono
}

void draw_rect(int x, int y, int width, int height, uint32_t color)
{

    // Solución con bucles en cero
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int current_x = x + i;
            int current_y = y + j;

            // color_buffer[(window_width * current_y) + current_x] = color;
            draw_pixel(current_x, current_y, color);
        }
    }
}
