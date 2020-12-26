#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"

triangle_t *triangles_to_render = NULL;

vec3_t camera_position = {0, 0, 0};

float fov_factor = 640;
bool is_running = false;
int previous_frame_time = 0;

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

    // Carga los valores del cubo en la estructura de mallas
    // load_cube_mesh_data();
    load_obj_file_data("./assets/cube.obj");
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

    // Si no dividimos por la distancia, tendremos una perspectiva sin profundidad
    // como la que se utilza en juegos ortográficos e isométricos
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z};

    return projected_point;
}

void update(void)
{
    // Esto genera un bucle para capar los FPS, pero consume toda la CPU
    // while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));
    // En su lugar usaremos SDL_Delay a nivel de SO para poner en IDLE el proceso un tiempo
    // Esperamos algo de tiempo hasta alcanzar el objetivo en milisegundos
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
        SDL_Delay(time_to_wait);

    // Cuantos milisegundos han pasado desde que empieza el juego
    previous_frame_time = SDL_GetTicks();

    // Inicializamos el array de triángulos a renderizar
    triangles_to_render = NULL;

    // Añadimos rotación
    mesh.rotation.x += 0.005;
    mesh.rotation.y += 0.005;
    mesh.rotation.z += 0.005;

    // Iteramos todas las caras de la malla
    int num_faces = array_length(mesh.faces);

    for (int i = 0; i < num_faces; i++)
    {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec3_t transformed_vertices[3];

        // TRANSFORMACIONES: Iteramos los 3 vértices de la cara actual
        for (int j = 0; j < 3; j++)
        {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // Trasladamos el vértice de profundidad lejos de la cámara
            transformed_vertex.z += 5;

            // Guardamos el vértice transformado en el array
            transformed_vertices[j] = transformed_vertex;
        }

        // Comprobamos el backface culling (si el triángulo mira la cámara para dibujarlo)
        vec3_t vector_a = transformed_vertices[0]; /*   A   */
        vec3_t vector_b = transformed_vertices[1]; /*  / \  */
        vec3_t vector_c = transformed_vertices[2]; /* C---B */

        // 1. Extraer los vectores B-A y C-A
        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);

        // 2. Calculamos el vector normal usando producto vectorial (face normal)
        // Prestar atención al engine, si lo hacemos left-handed el eje Z se
        // incrementa con la profundidad (el orden de los vectores sí importa)
        vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // 3. Buscamos el vector entre un punto del trángulo y el origen de la cámara
        // Figura "docs/15 camera raycast.png"
        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        // 4. Calculamos cuán alineado está el camera_ray respecto al vector normal
        // Utilizando para ello el producto escalar (el orden de los vectores no importa)
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        // 5. Si el triángulo no está alineado con la cámara saltamos la iteración
        // Esto ahorrará muchos cálculos al no dibujar los triángulos no alineados
        if (dot_normal_camera < 0)
        {
            continue; //
        }

        triangle_t projected_triangle;

        // PROYECCIONES: Iteramos los 3 vértices de la cara actual
        for (int j = 0; j < 3; j++)
        {
            // Proyectamos el vértice
            vec2_t projected_point = project(transformed_vertices[j]);

            // Escalamos y trasladamos los puntos proyectados al centro de la pantalla
            projected_point.x += (window_width / 2);
            projected_point.y += (window_height / 2);

            projected_triangle.points[j] = projected_point;
        }

        // Guardamos el triángulo proyectado en el array de triángulos a renderizar
        array_push(triangles_to_render, projected_triangle);
    }
}

void render(void)
{
    // Dibujamos la cuadrícula
    draw_grid();

    int num_triangles = array_length(triangles_to_render);

    // Iteramos los triángulos a renderizar
    for (int i = 0; i < num_triangles; i++)
    {
        triangle_t triangle = triangles_to_render[i];

        // Renderizamos cada uno de los tres vértices uno por uno
        draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 2, 0xFFFFFF00);
        draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);

        // Renderizamos los lados de cada triángulo
        draw_triangle(
            triangle.points[0].x,
            triangle.points[0].y,
            triangle.points[1].x,
            triangle.points[1].y,
            triangle.points[2].x,
            triangle.points[2].y,
            0xFF008000);
    }

    // Liberamos el array de triángulos en cada frame
    array_free(triangles_to_render);

    // Copiamos el color buffer a la textura y lo limpiamos
    render_color_buffer();
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

// Función encargada de liberar la memoria reservada por el programa
void free_resources(void)
{
    array_free(mesh.faces);
    array_free(mesh.vertices);
    free(color_buffer); // Si liberas algo que ya ha sido liberado da un error (lo tenía duplicado en display.c)
    // https://cboard.cprogramming.com/c-programming/176238-need-help-understanding-error-1073741819-a.html
    // Convert -1073741819 to hex - you get C0000005. This is windows generic "access violation".
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
    free_resources();
    return 0;
}