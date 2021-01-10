#include "display.h"

/////// Globales
/////// Las variables estáticas son visibles solo en el fichero actual

static uint32_t *color_buffer = NULL; // array uint32 (32 bits / 4 bytes entero)
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *color_buffer_texture = NULL; // Para el color buffer
static float *z_buffer = NULL;
static bool is_fullscreen = false;
static int window_width = 1000;
static int window_height = 500;
static int render_method = 0;
static int cull_method = 0;

int get_window_width(void)
{
    return window_width;
}

int get_window_height(void)
{
    return window_height;
}

void set_render_method(int method)
{
    render_method = method;
}
void set_cull_method(int method)
{
    cull_method = method;
}

float get_zbuffer_at(int x, int y)
{
    if (x < 0 || x >= window_width - 1 || y < 0 || y > window_height - 1)
    {
        return 1.0;
    }
    return z_buffer[(window_width * y) + x];
}
void update_zbuffer_at(int x, int y, float value)
{
    if (x < 0 || x >= window_width - 1 || y < 0 || y > window_height - 1)
    {
        return;
    }
    z_buffer[(window_width * y) + x] = value;
}

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
        int fullscreen_width = display_mode.w;
        int fullscreen_height = display_mode.h;

        // Simular resolución más pequeña
        window_width = fullscreen_width / 2;
        window_height = fullscreen_height / 2;

        // Crear ventana SDL
        window = SDL_CreateWindow(
            NULL,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            fullscreen_width,
            fullscreen_height,
            SDL_WINDOW_BORDERLESS);
    }
    else
    {

        // Crear ventana SDL
        window = SDL_CreateWindow(
            NULL,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_width,
            window_height,
            SDL_WINDOW_RESIZABLE);
    }

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

    // Asigno bytes requeridos en memoria para el color buffer y el z-buffer
    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);
    z_buffer = (float *)malloc(sizeof(float) * window_width * window_height);

    // There is a possibility that malloc fails to allocate that number of bytes in memory maybe the
    // machine does not have enough free memory, if that happens malloc will return a NULL pointer.
    if (color_buffer != NULL)
    {
        // Creo la textura donde copiaremos el color buffer
        color_buffer_texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            get_window_width(),
            get_window_height());
    }

    return true;
}

bool is_cull_backface(void)
{
    return cull_method == CULL_BACKFACE;
}

// C no tiene recolector de basura... tenemos que liberar memoria nosotros
void destroy_window(void)
{
    free(color_buffer); // Si liberas algo que ya ha sido liberado da un error de memoria
    free(z_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void clear_color_buffer(uint32_t color)
{
    for (int i = 0; i < window_height * window_width; i++)
    {
        color_buffer[i] = color;
    }
}

void clear_z_buffer()
{
    for (int i = 0; i < window_height * window_width; i++)
    {
        z_buffer[i] = 1.0; // estandar de la industria, crece hacia adentro
    }
}

bool should_render_filled_triangle(void)
{
    return (render_method == RENDER_FILL_TRIANGLE ||
            render_method == RENDER_FILL_TRIANGLE_WIRE);
}
bool should_render_textured_triangle(void)
{
    return (render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE);
}

bool should_render_wireframe(void)
{
    return (render_method == RENDER_WIRE ||
            render_method == RENDER_WIRE_VERTEX ||
            render_method == RENDER_FILL_TRIANGLE_WIRE ||
            render_method == RENDER_TEXTURED_WIRE);
}

bool should_render_wire_vertex(void)
{
    return render_method == RENDER_WIRE_VERTEX;
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
    SDL_RenderPresent(renderer);
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
                color_buffer[(window_width * y) + x] = 0xFF444444;
        }
    }
}

void draw_pixel(int x, int y, uint32_t color)
{
    // Dibujamos el píxel si está dentro de la ventana
    if (x < 0 || x >= window_width || y < 0 || y >= window_height)
        return;
    color_buffer[(window_width * y) + x] = color;
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
