#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "matrix.h"
#include "mesh.h"
#include "light.h"

// Array de triangulos que debo renderizar frame por frame
triangle_t *triangles_to_render = NULL;

// Variables globales de estado y bucle de juego
bool is_running = false;
int previous_frame_time = 0;
char *model_file = "./assets/cube.obj";

vec3_t camera_position = {0, 0, 0};
mat4_t proj_matrix;

// La función setup inicializa variables y objetos del juego
void setup(void)
{
    // Inicializamos el modo de renderizado y el culling
    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;

    // Asigno bytes requeridos en memoria para el color buffer
    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

    // Creo la textura donde copiaremos el color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height);

    // Inicilizamos la matrix de projección de la perspectiva
    float fov = M_PI / 3.0; // esto es lo mismo que 180/3, o 60deg (en radianos)
    float aspect = (float)window_height / (float)window_width;
    float znear = 0.1, zfar = 100.0;
    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

    // Carga los valores del cubo en la estructura de mallas
    //load_cube_mesh_data();
    load_obj_file_data(model_file);
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
        else if (event.key.keysym.sym == SDLK_1)
        {
            // displays the wireframe and a small red dot for each triangle vertex
            render_method = RENDER_WIRE_VERTEX;
        }
        else if (event.key.keysym.sym == SDLK_2)
        {
            // displays only the wireframe lines
            render_method = RENDER_WIRE;
        }
        else if (event.key.keysym.sym == SDLK_3)
        {
            // displays filled triangles with a solid color
            render_method = RENDER_FILL_TRIANGLE;
        }
        else if (event.key.keysym.sym == SDLK_4)
        {
            // displays both filled triangles and wireframe lines
            render_method = RENDER_FILL_TRIANGLE_WIRE;
        }
        else if (event.key.keysym.sym == SDLK_c)
        {
            // we should enable back-face culling
            cull_method = CULL_BACKFACE;
        }
        else if (event.key.keysym.sym == SDLK_d)
        {
            // we should disable the back-face culling
            cull_method = CULL_NONE;
        }
        break;
    }
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

    // Cambiamos los valores del mesh scale/rotation en cada frame
    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0;
    mesh.rotation.z += 0;
    mesh.scale.x += 0;
    mesh.scale.y += 0;
    mesh.scale.z += 0;
    mesh.translation.x += 0;
    mesh.translation.y += 0;
    mesh.translation.z = 5.0; // Trasladamos el vértice de profundidad lejos de la cámara

    // Crear una matriz de escalado, rotación y traslación que utilizará el multiplicador del mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    // Iteramos todas las caras de la malla
    int num_faces = array_length(mesh.faces);

    for (int i = 0; i < num_faces; i++)
    {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec4_t transformed_vertices[3];

        // TRANSFORMACIONES: Iteramos los 3 vértices de la cara actual
        for (int j = 0; j < 3; j++)
        {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // Creamos una matriz de mundo combinando escalado, rotación y traslación de matrices
            mat4_t world_matrix = mat4_identity();

            // Multiplicamos todas las matrices para cargar la matriz de mundo
            // La matriz de la izquierda es la que transforma la matriz de la derecha
            // IMPORTANTE: El orden de las transformaciones debe tenerse en cuenta
            //             1. Escalar  2. Rotar  3. Trasladar
            //                    [T] * [R] * [S] * v
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            // Multiplicamos la matriz de mundo por el vector original del vertice
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
            // Guardamos el vértice transformado en el array
            transformed_vertices[j] = transformed_vertex;
        }

        // Comprobamos el backface culling y la luz (si el triángulo mira la cámara para dibujarlo)
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]); /*   A   */
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]); /*  / \  */
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]); /* C---B */

        // 1. Extraer los vectores B-A y C-A (solo nos interesa la dirección)
        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);

        // Como añadido podemos normalizarlos
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        // 2. Calculamos el vector normal usando producto vectorial (face normal)
        // Prestar atención al engine, si lo hacemos left-handed el eje Z se
        // incrementa con la profundidad (el orden de los vectores sí importa)
        vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // Es normal normalizar el vector normal (lo pasamos por referencia para optimizar)
        vec3_normalize(&normal);

        // 3. Buscamos el vector entre un punto del trángulo y el origen de la cámara
        // Figura "docs/15 camera raycast.png"
        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        // 4. Calculamos cuán alineado está el camera_ray respecto al vector normal
        // Utilizando para ello el producto escalar (el orden de los vectores no importa)
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        // 5. Si el triángulo no está alineado con la cámara saltamos la iteración
        if (cull_method == CULL_BACKFACE)
        {
            // Backface culling, bypassing triangles that are looking away from the camera
            if (dot_normal_camera < 0)
                continue;
        }

        // PROYECCIONES: Iteramos los 3 vértices de la cara actual
        vec4_t projected_points[3];

        for (int j = 0; j < 3; j++)
        {
            // Proyectamos el vértice
            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            // Escalamos en la vista
            projected_points[j].x *= (window_width / 2.0);
            projected_points[j].y *= (window_height / 2.0);

            // Invertimos los valores 'y' debido a los valores invertidos de la pantalla
            // Figura 34 valores invertidos.png
            projected_points[j].y *= -1;

            // Escalamos y trasladamos los puntos proyectados al centro de la pantalla
            projected_points[j].x += (window_width / 2.0);
            projected_points[j].y += (window_height / 2.0);
        }

        // Calculamos la profundidad media de cada cara basada en los vértices después de transformarlos (forma un poco "hacky")
        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

        // Calculamos la intensidad del sombreado basándonos en cuán alineados están la normal de la cara del triángulo y la inversa de la luz (lo negamos por lo de que la profundidad va hacia dentro en nuestro modelo, y en cambio la luz se refleja hacia fuera a nuestra cámara, por eso si no lo negamos se nos oscurece al revés)
        float light_intensity_factor = -vec3_dot(normal, light.direction);

        // Calculamos el color del triángulo basados en el ángulo de la luz
        uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

        triangle_t projected_triangle = {
            .points = {
                {projected_points[0].x, projected_points[0].y},
                {projected_points[1].x, projected_points[1].y},
                {projected_points[2].x, projected_points[2].y},
            },
            .color = triangle_color,
            .avg_depth = avg_depth};

        // Guardamos el triángulo proyectado en el array de triángulos a renderizar
        array_push(triangles_to_render, projected_triangle);
    }

    // Ordenamos los triángulos a renderizar en base a su profundidad media
    // Podemos hacer un típico algoritmo de burbuja intercambiando valores
    for (int i = 0; i < array_length(triangles_to_render); i++)
    {
        for (int j = 0; j < array_length(triangles_to_render); j++)
        {
            if (triangles_to_render[j].avg_depth > triangles_to_render[i].avg_depth)
            {
                triangle_t tmp = triangles_to_render[i];
                triangles_to_render[i] = triangles_to_render[j];
                triangles_to_render[j] = tmp;
            }
        }
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

        // Renderizamos el relleno de cada triángulo (fill)
        if (
            render_method == RENDER_FILL_TRIANGLE ||
            render_method == RENDER_FILL_TRIANGLE_WIRE)
        {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                triangle.color);
        }

        // Dibujamos los bordes de cada triángulo (wireframe)
        if (
            render_method == RENDER_WIRE ||
            render_method == RENDER_WIRE_VERTEX ||
            render_method == RENDER_FILL_TRIANGLE_WIRE)
        {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFFFFFFFF);
        }

        // Renderizamos cada uno de los tres vértices uno por uno
        if (render_method == RENDER_WIRE_VERTEX)
        {
            draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFF0000);
            draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFF0000);
            draw_rect(triangle.points[2].x, triangle.points[2].y, 5, 3, 0xFFFF0000);
        }
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
    free(color_buffer); // Si liberas algo que ya ha sido liberado da un error de memoria
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