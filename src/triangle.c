#include "triangle.h"
#include "display.h"
#include <stdint.h>

// Intercambio para ordenamiento típico de la burbuja
void int_swap(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
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
