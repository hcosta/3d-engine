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

// Retorna las masas baricentricas alfa, beta y gama para el punto P
//          A
//         /|\
//        / | \
//       /  |  \
//      /  (p)  \
//     /  /   \  \
//    / /       \ \
//   B-------------C
vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p)
{
    // Buscamos los vectores entre los vértices ABC y el punto P
    vec2_t ab = vec2_sub(b, a);
    vec2_t bc = vec2_sub(c, b);
    vec2_t ac = vec2_sub(c, a);
    vec2_t ap = vec2_sub(p, a);
    vec2_t bp = vec2_sub(p, b);

    // Calculamos la masa del triángulo completo ABC usando producto vectorial (área del paralelogramo)
    float area_triangle_abc = (ab.x * ac.y - ab.y * ac.x);

    // La masa alpha es el área del subtriángulo BCP dividio entre el área del triángulo completo ABC
    float alpha = (bc.x * bp.y - bp.x * bc.y) / area_triangle_abc;

    // La masa beta es el área del subtriángulo ACP dividio entre el área del triángulo completo ABC
    float beta = (ap.x * ac.y - ac.x * ap.y) / area_triangle_abc;

    // La masa gamma la encontramos fácilmente pues la suma de las tres masas es siempre 1 (área total)
    float gamma = 1 - alpha - beta;

    vec3_t weights = {alpha, beta, gamma};
    return weights;
}

// Esta función es para dibujar el pixel texturizado en la posición x,y usando interpolación
void draw_texel(
    int x, int y, uint32_t *texture,
    vec4_t point_a, vec4_t point_b, vec4_t point_c,
    tex2_t a_uv, tex2_t b_uv, tex2_t c_uv)
{
    vec2_t p = {x, y};
    vec2_t a = vec2_from_vec4(point_a);
    vec2_t b = vec2_from_vec4(point_b);
    vec2_t c = vec2_from_vec4(point_c);
    vec3_t weights = barycentric_weights(a, b, c, p);

    float alpha = weights.x; // en este contexto usamos el vector3
    float beta = weights.y;
    float gamma = weights.z;

    // Primera protección
    const float EPSILON = 0.00001;
    if (alpha < -EPSILON || beta < -EPSILON || gamma < -EPSILON)
        return;

    // Variables para almacenar valores interpolados de U, V y 1/W para el píxel actual
    float interpolated_u;
    float interpolated_v;
    float interpolated_reciprocal_w;

    // Realizamos la interpolaciones de todos los valores U/w y V/w usando masas baricéntricas y el factor de 1/w
    interpolated_u = (a_uv.u / point_a.w) * alpha + (b_uv.u / point_b.w) * beta + (c_uv.u / point_c.w) * gamma;
    interpolated_v = (a_uv.v / point_a.w) * alpha + (b_uv.v / point_b.w) * beta + (c_uv.v / point_c.w) * gamma;

    // También voy a interpolar el valor de 1/w para el pixel actual
    interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

    // Ahora podemos dividir de vuelta ambos valores interpolados por 1/w
    interpolated_u /= interpolated_reciprocal_w;
    interpolated_v /= interpolated_reciprocal_w;

    // Mapeamos la coordenada UV al alto y ancho de la textura completa
    int tex_x = (int)abs((interpolated_u * texture_width)) % texture_width;
    int tex_y = (int)abs((interpolated_v * texture_height)) % texture_height;

    //draw_pixel(x, y, texture[(texture_width * tex_y) * tex_x]);
    int tex_index = ((texture_width * tex_y) + tex_x) % (texture_width * texture_height);
    draw_pixel(x, y, texture[tex_index]);
}

// Dibujamos la textura del triángulo basada en el array texturizado de colores
// Partimos el triángulo original en dos, el que es plano abajo y el que es plano arriba
void draw_textured_triangle(
    int x0, int y0, float z0, float w0, float u0, float v0,
    int x1, int y1, float z1, float w1, float u1, float v1,
    int x2, int y2, float z2, float w2, float u2, float v2,
    uint32_t *texture)
{
    // Necesitamos ordenar los vértices a partir de la coordenada Y ascendente (y0 < y1 < y2)
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2)
    {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&z1, &z2);
        float_swap(&w1, &w2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&z0, &z1);
        float_swap(&w0, &w1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // Volteamos el componente V para las coordenadas UV invertidas (V crece hacia abajo)
    v0 = 1.0 - v0;
    v1 = 1.0 - v1;
    v2 = 1.0 - v2;

    // Aquí creamos puntos de vector y coordenadas de texturas después de ordenar los vértices
    vec4_t point_a = {x0, y0, z0, w0};
    vec4_t point_b = {x1, y1, z1, w1};
    vec4_t point_c = {x2, y2, z2, w2};

    tex2_t a_uv = {u0, v0};
    tex2_t b_uv = {u1, v1};
    tex2_t c_uv = {u2, v2};

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

            for (int x = x_start; x <= x_end; x++)
            {
                // Dibujamos el texel de la sección pertinente interpolado
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
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
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start)
                int_swap(&x_start, &x_end); // intercambiamos si x_start está a la derecha de x_end

            for (int x = x_start; x <= x_end; x++)
                // Dibujamos el texel de la sección pertinente interpolado
                draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
        }
    }
}
