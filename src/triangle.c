#include "triangle.h"
#include "display.h"
#include "swap.h"
#include <stdint.h>

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

// Dibujamos el triángulo con el lado plano inferior (flat-bottom)
//
//        (x0,y0)
//          / \
//         /   \
//        /     \
//       /       \
//      /         \
//  (x1,y1)------(Mx,My) <-- (x2,y2)
//
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // Buscamos las dos pendientes (las dos patas del triángulo)
    // La relación de alto entre ancho, aunque en este caso buscamos
    // la inversa pues incrementaremos x (ancho entre alto)
    float inv_slope_1 = (float)(x1 - x0) / (y1 - y0);
    float inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

    // Definimos x_start y x_end desde el vértice superior (x0, y0)
    float x_start = x0;
    float x_end = x0;

    // Iteramos todos los scanlines (líneas horizontales) del arriba a abajo
    for (int y = y0; y <= y2; y++)
    {
        draw_line(x_start, y, x_end, y, color);
        // Incrementamos (al bajar) las x en cada scanline
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}

// Dibujamos el triángulo con el lado plano superior (flat-top)
//
//  (x0,y0)------(Mx,My) <-- (x1,y1)
//      \_           \
//         \_         \
//            \_       \
//               \_     \
//                  \    \
//                    \_  \
//                       \_\
//                          \
//                        (x2,y2)
//
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // Buscamos las dos pendientes invertidas
    float inv_slope_1 = (float)(x2 - x0) / (y2 - y0);
    float inv_slope_2 = (float)(x2 - x1) / (y2 - y1);

    // Definimos x_start y x_end desde el vértice inferior (x0, y0)
    float x_start = x2;
    float x_end = x2;

    // Iteramos todos los scanlines (líneas horizontales) de abajo a arriba
    for (int y = y2; y >= y0; y--)
    {
        draw_line(x_start, y, x_end, y, color);
        // Decrementamos (al subir) las x en cada scanline
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}

// Para rasterizar un triángulo lo partiremos en dos mitades trazando un plano de y1 a My
// Conseguiremos así un triángulo con un lado plano superior y otro con un lado plano inferior
//
//          (x0,y0)
//            / \
//           /   \
//          /     \
//         /       \
//        /         \
//   (x1,y1)------(Mx,My)
//       \_           \
//          \_         \
//             \_       \
//                \_     \
//                   \    \
//                     \_  \
//                        \_\
//                           \
//                         (x2,y2)
//
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // 1. Ordenamos los vértics por la cordenada-y de forma ascendente (y0 < y1 < y2)
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    if (y1 > y2)
    {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }

    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    // Comprobamos casos de concordancias para evitar divisiones entre cero
    if (y1 == y2)
    {
        // Si 'y1' e 'y2' están a la misma altura, no necesitamos renderizar el triángulo de
        // abajo, podemos dibujar el triángulo completo únicamente desde el lado inferior:
        //        (x0,y0)
        //          / \
        //         /   \
        //        /     \
        //       /       \
        //      /         \
        //  (x1,y1)------(x2,y2)
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else if (y0 == y1)
    {
        // Si 'y0' e 'y1' están a la misma altura, no necesitamos renderizar el triángulo de
        // arriba, podemos dibujar el triángulo completo únicamente desde el lado superior:
        //  (x1,y1)------(Mx,My)
        //     \          /
        //      \        /
        //       \      /
        //        \    /
        //         \  /
        //       (x2,y2)
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else
    {
        // 2. Calculamos el nuevo vértice (Mx, My) utilizando semejanza de triángulos
        int My = y1;
        int Mx = ((float)((x2 - x0) * (y1 - y0)) / (float)(y2 - y0)) + x0;

        // 3. Dibujamos el triángulo con el lado plano inferior (flat-bottom)
        fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

        // 4. Dibujamos el triángulo con el lado plano superior (flat-top)
        fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
    }
}

void draw_textured_triangle(
    int x0, int y0, float u0, float v0,
    int x1, int y1, float u1, float v1,
    int x2, int y2, float u2, float v2,
    uint32_t *texture)
{
    // Necesitamos ordenar los vértices a partir de la coordenada Y ascendente (y0 < y1 < y2)
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2)
    {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // Renderizamos la parte superior del triángulo (flat-bottom)
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0)
        inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0)
        inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y1 - y0 != 0)
    {
        for (int y = y0; y <= y1; y++)
        {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start)
            {
                // intercambiamos si x_start está a la derecha de x_end
                // así garantizamos que se puede dibujar correctamente
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++)
            {
                // Dibujamos nuestro pixel personalizado
                draw_pixel(x, y, (x % 2 == 0 && y % 2 == 0) ? 0xFFFF00FF : 0x00000000);
            }
        }
    }

    // Renderizamos la parte inferior del triangle (flat-top)
    inv_slope_1 = 0;
    inv_slope_2 = 0;

    if (y2 - y1 != 0)
        inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
    if (y2 - y0 != 0)
        inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y2 - y1 != 0)
    {
        for (int y = y1; y <= y2; y++)
        {
            // Aquí es donde vamos recorriendo cada píxel de la línea
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start)
            {
                int_swap(&x_start, &x_end); // intercambiamos si x_start está a la derecha de x_end
            }

            for (int x = x_start; x < x_end; x++)
            {
                // Dibujamos nuestro pixel personalizado
                draw_pixel(x, y, (x % 2 == 0 && y % 2 == 0) ? 0xFFFF00FF : 0x00000000);
            }
        }
    }
}
