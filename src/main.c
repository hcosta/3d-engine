#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "upng.h"
#include "array.h"
#include "display.h"
#include "clipping.h"
#include "vector.h"
#include "matrix.h"
#include "light.h"
#include "camera.h"
#include "triangle.h"
#include "texture.h"
#include "mesh.h"

// Array de triangulos que debo renderizar frame por frame
#define MAX_TRIANGLES 10000
triangle_t triangles_to_render[MAX_TRIANGLES];
int num_triangles_to_render = 0;

// Variables globales de estado y bucle de juego
bool is_running = false;
int previous_frame_time = 0;
float delta_time = 0;
char *model_file = "./assets/f22.obj";
char *texture_file = "./assets/f22.png";

// Matrices de transformación globales
mat4_t proj_matrix;
mat4_t world_matrix;
mat4_t view_matrix;

// La función setup inicializa variables y objetos del juego
void setup(void)
{
    // Inicializamos el modo de renderizado y el culling
    set_render_method(RENDER_TEXTURED);
    set_cull_method(CULL_BACKFACE);

    // Inicializar la dirección de luz de la escena
    init_light(vec3_new(0, 0, 1));
    // Inicilizamos la matrix de projección de la perspectiva
    float aspect_x = (float)get_window_width() / (float)get_window_height();
    float aspect_y = (float)get_window_height() / (float)get_window_width();
    float fov_y = M_PI / 3.0; // esto es lo mismo que 180/3, o 60deg (en radianos)
    float fov_x = atan(tan(fov_y / 2) * aspect_x) * 2;
    float z_near = 1.0;
    float z_far = 12.0;
    proj_matrix = mat4_make_perspective(fov_y, aspect_y, z_near, z_far);

    // Inicializamos los planos del frustum con un punto a y una normal a
    init_frustum_planes(fov_x, fov_y, z_near, z_far);

    // Carga los valores del fichero de mallas
    load_obj_file_data(model_file);

    // Cargamos la informacion de la textura desde un PNG externo
    load_png_texture_data(texture_file);
}

void process_input(void)
{
    const uint8_t *keystates = SDL_GetKeyboardState(NULL);

    if (keystates[SDL_SCANCODE_W]) // forward
    {
        update_camera_forward_velocity(vec3_mul(get_camera_direction(), 1.0 * delta_time));
        update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
    }
    if (keystates[SDL_SCANCODE_S]) // backward
    {
        update_camera_forward_velocity(vec3_mul(get_camera_direction(), 1.0 * delta_time));
        update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
    }
    if (keystates[SDL_SCANCODE_A]) // rotation radians/sec
        rotate_camera_yaw(-0.75 * delta_time);
    if (keystates[SDL_SCANCODE_D]) // rotation radians/secX
        rotate_camera_yaw(0.75 * delta_time);
    if (keystates[SDL_SCANCODE_UP]) // move up
        rotate_camera_pitch(-1.0 * delta_time);
    if (keystates[SDL_SCANCODE_DOWN]) // move down
        rotate_camera_pitch(1.0 * delta_time);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                is_running = false;
                break;
            }
            if (event.key.keysym.sym == SDLK_1) // wireframe and each triangle vertex
            {
                set_render_method(RENDER_WIRE_VERTEX);
                break;
            }
            if (event.key.keysym.sym == SDLK_2) // only the wireframe lines
            {
                set_render_method(RENDER_WIRE);
                break;
            }
            if (event.key.keysym.sym == SDLK_3) // filled triangles with a solid color
            {
                set_render_method(RENDER_FILL_TRIANGLE);
                break;
            }
            if (event.key.keysym.sym == SDLK_4) // filled triangles and wireframe lines
            {
                set_render_method(RENDER_FILL_TRIANGLE_WIRE);
                break;
            }
            if (event.key.keysym.sym == SDLK_5) // render triangle texture
            {
                set_render_method(RENDER_TEXTURED);
                break;
            }
            if (event.key.keysym.sym == SDLK_6) // render triangle texture and wireframe
            {
                set_render_method(RENDER_TEXTURED_WIRE);
                break;
            }
            if (event.key.keysym.sym == SDLK_c) // we should enable back-face culling
            {
                set_cull_method(CULL_BACKFACE);
                break;
            }
            if (event.key.keysym.sym == SDLK_x) // we should disable the back-face culling
            {
                set_cull_method(CULL_NONE);
                break;
            }
            break;
        }
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

    // Diferencia de tiempo entre fotogramas en milisegundos
    // Lo transformaremos a segundos para actualizar nuestros objetos de juego
    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

    // Cuantos milisegundos han pasado desde que empieza el juego
    previous_frame_time = SDL_GetTicks();

    // Reiniciamos el numero de triángulos a dibujar en el frame actual
    num_triangles_to_render = 0;

    // Cambiamos los valores del mesh scale/rotation en cada frame
    mesh.rotation.x += 0.0 * delta_time; // 1 pixel por segundo
    mesh.rotation.y += 0.0 * delta_time;
    mesh.rotation.z += 0.0 * delta_time;

    // mesh.scale.x += 0.0 * delta_time;
    // mesh.scale.y += 0.0 * delta_time;
    // mesh.scale.z += 0.0 * delta_time;

    // mesh.translation.x += 0.0 * delta_time;
    // mesh.translation.y += 0.0 * delta_time;

    mesh.translation.z = 5.0; // Trasladamos el vértice de profundidad lejos de la cámara

    // Cambiamos la posición de la cámara en cada fotograma
    // camera.position.x += 0.0 * delta_time;
    // camera.position.y += 0.0 * delta_time;
    // camera.position.z += 0.0 * delta_time;

    /////// Crear la matriz de vista (view) ///// YA NO mirandolo hacia (lookAt) un punto hardcodado
    /////// vec3_t target = {0, 0, 5.0}; // justo donde renderizamos el modelo

    // Calculamos la rotación de la nueva cámara FPS y su traslación

    // Creamos la matriz de rotación
    vec3_t target = get_camera_lookat_target(); //Inicializamos el target mirando el eje-z positivo
    vec3_t up_direction = vec3_new(0, 1, 0);
    // mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
    // camera.direction = vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(target)));

    // // Offseteamos la posicion de la cámara en la dirección hacia donde ella mira
    // target = vec3_add(camera.position, camera.direction);
    // vec3_t up_direction = {0, 1, 0};

    // // Creamos la matriz de vista
    view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

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
        face_vertices[0] = mesh.vertices[mesh_face.a];
        face_vertices[1] = mesh.vertices[mesh_face.b];
        face_vertices[2] = mesh.vertices[mesh_face.c];

        vec4_t transformed_vertices[3];

        // TRANSFORMACIONES: Iteramos los 3 vértices de la cara actual
        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++)
        {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // Creamos una matriz de mundo combinando escalado, rotación y traslación de matrices
            world_matrix = mat4_identity();

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

            // Multipicamos la matriz de vista por el vector para transformar la escena a al espacio de la cámara
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

            // Guardamos el vértice transformado en el array
            transformed_vertices[j] = transformed_vertex;
        }

        // Comprobamos el backface culling y la luz (si el triángulo mira la cámara para dibujarlo)
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]); /*   A   */
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]); /*  / \  */
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]); /* C---B */

        // 1. Extraer vectores B-A y C-A y normalizamos (solo interesa la dirección)
        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        // 2. Calculamos el vector normal usando producto vectorial (face normal)
        // Prestar atención al engine, si lo hacemos left-handed el eje Z se
        // incrementa con la profundidad (el orden de los vectores sí importa),
        // normalizar el vector normal (lo pasamos por referencia para optimizar)
        vec3_t normal = vec3_cross(vector_ab, vector_ac);
        vec3_normalize(&normal);

        // 3. Buscamos el vector entre un punto del trángulo y el origen de la cámara
        // Figura "docs/15 camera raycast.png"
        vec3_t origin = {0, 0, 0};
        vec3_t camera_ray = vec3_sub(origin, vector_a);

        // 4. Calculamos cuán alineado está el camera_ray respecto al vector normal
        // Utilizando para ello el producto escalar (el orden de los vectores no importa)
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        // 5. Si el triángulo no está alineado con la cámara saltamos la iteración
        if (is_cull_backface())
        {
            // Backface culling, bypassing triangles that are looking away from the camera
            if (dot_normal_camera < 0)
                continue;
        }

        // Clipping!!
        // Creamos un polígono a partir del triángulo original transformado
        polygon_t polygon = polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]),
            vec3_from_vec4(transformed_vertices[1]),
            vec3_from_vec4(transformed_vertices[2]),
            mesh_face.a_uv,
            mesh_face.b_uv,
            mesh_face.c_uv);

        // Clipeamos el polígono y retornmos el nuevo polígono con potenciales nuevos vértices
        clip_polygon(&polygon);

        // Después del clipping tenemos que romper el poligono en triángulos
        triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
        int num_triangles_after_clipping = 0;

        triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

        // Iteramos todos los triángulos ensamblados después del clipping
        for (int t = 0; t < num_triangles_after_clipping; t++)
        {
            triangle_t triangle_after_clipping = triangles_after_clipping[t];

            // PROYECCIONES: Iteramos los 3 vértices de la cara actual
            vec4_t projected_points[3];

            for (int j = 0; j < 3; j++)
            {
                // Proyectamos el vértice
                projected_points[j] = mat4_mul_vec4(proj_matrix, triangle_after_clipping.points[j]);

                // Ejecutamos la división de la perspectiva
                if (projected_points[j].w != 0)
                {
                    projected_points[j].x /= projected_points[j].w;
                    projected_points[j].y /= projected_points[j].w;
                    projected_points[j].z /= projected_points[j].w;
                }

                // Invertimos los valores 'y' debido a los valores invertidos de la pantalla
                // Figura 34 valores invertidos.png
                projected_points[j].y *= -1;

                // Escalamos en la vista
                projected_points[j].x *= (get_window_width() / 2.0);
                projected_points[j].y *= (get_window_height() / 2.0);

                // Trasladamos los puntos proyectados al centro de la pantalla
                projected_points[j].x += (get_window_width() / 2.0);
                projected_points[j].y += (get_window_height() / 2.0);
            }

            // Calculamos la intensidad del sombreado basándonos en cuán alineados están la normal de la cara del triángulo y la inversa de la luz (lo negamos por lo de que la profundidad va hacia dentro en nuestro modelo, y en cambio la luz se refleja hacia fuera a nuestra cámara, por eso si no lo negamos se nos oscurece al revés)
            float light_intensity_factor = -vec3_dot(normal, get_light_direction());

            // Calculamos el color del triángulo basados en el ángulo de la luz
            uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

            triangle_t triangle_to_render = {
                .points = {
                    {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
                    {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
                    {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
                },
                .texcoords = {
                    {triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v},
                    {triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v},
                    {triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v},
                },
                .color = triangle_color};

            // Guardamos el triángulo proyectado en el array de triángulos a renderizar
            // almacenar datos en memoria y borrarlos así es muy cpu dependiente, gasta mucho
            //array_push(triangles_to_render, projected_triangle);
            // mil veces mejor hacerlo en memoria reservada
            if (num_triangles_to_render < MAX_TRIANGLES)
            {
                triangles_to_render[num_triangles_to_render++] = triangle_to_render;
            }
        }
    }
}

void render(void)
{
    // Reseteamos los buffers para preparar el siguiente frame
    clear_color_buffer(0xFF000000);
    clear_z_buffer();

    // Dibujamos la cuadrícula
    draw_grid();

    // Iteramos los triángulos a renderizar
    for (int i = 0; i < num_triangles_to_render; i++)
    {
        triangle_t triangle = triangles_to_render[i];

        // Draw filled triangle
        if (should_render_filled_triangle())
        {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, // vertex A
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, // vertex B
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, // vertex C
                triangle.color);
        }

        // Draw textured triangle
        if (should_render_textured_triangle())
        {
            draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v, // vertex A
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v, // vertex B
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v, // vertex C
                mesh_texture);
        }

        // Draw triangle wireframe
        if (should_render_wireframe())
        {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFFFFFFFF);
        }

        // Draw triangle vertex points
        if (should_render_wire_vertex())
        {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFF0000FF); // vertex A
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFF0000FF); // vertex B
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFF0000FF); // vertex C
        }
    }

    // Copiamos el color buffer a la textura y lo limpiamos
    render_color_buffer();
}

// Función encargada de liberar la memoria reservada por el programa
void free_resources(void)
{
    array_free(mesh.faces);
    array_free(mesh.vertices);
    upng_free(png_texture);
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