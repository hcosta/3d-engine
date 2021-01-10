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

///////////////////////////////////////////////////////////////////////////////
// Global variables for execution status and game loop
///////////////////////////////////////////////////////////////////////////////
bool is_running = false;
int previous_frame_time = 0;
float delta_time = 0;

///////////////////////////////////////////////////////////////////////////////
// Array to store triangles that should be rendered each frame
///////////////////////////////////////////////////////////////////////////////
#define MAX_TRIANGLES 10000
triangle_t triangles_to_render[MAX_TRIANGLES];
int num_triangles_to_render = 0;

///////////////////////////////////////////////////////////////////////////////
// Declaration of our global transformation matrices
///////////////////////////////////////////////////////////////////////////////
mat4_t proj_matrix;
mat4_t world_matrix;
mat4_t view_matrix;

///////////////////////////////////////////////////////////////////////////////
// Setup function to initialize variables and game objects
///////////////////////////////////////////////////////////////////////////////
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
    float z_far = 50.0;
    proj_matrix = mat4_make_perspective(fov_y, aspect_y, z_near, z_far);

    // Inicializamos los planos del frustum con un punto a y una normal a
    init_frustum_planes(fov_x, fov_y, z_near, z_far);

    // Cargamos un numero limitado de meshes con sus texturas y vectores de escalado, traslación y rotación individual
    load_mesh(
        "./assets/runway.obj",  // mesh objects
        "./assets/runway.png",  // mesh texture
        vec3_new(1, 1, 1),      // scalation vector
        vec3_new(0, -1.5, +23), // translation vector
        vec3_new(0, 0, 0));     // rotation vector

    load_mesh("./assets/f117.obj", "./assets/f117.png", vec3_new(1, 1, 1), vec3_new(0, -1.3, +5), vec3_new(0, -M_PI / 2, 0));
    load_mesh("./assets/f22.obj", "./assets/f22.png", vec3_new(1, 1, 1), vec3_new(-2, -1.3, +9), vec3_new(0, -M_PI / 2, 0));
    load_mesh("./assets/efa.obj", "./assets/efa.png", vec3_new(1, 1, 1), vec3_new(+2, -1.3, +9), vec3_new(0, -M_PI / 2, 0));
}

///////////////////////////////////////////////////////////////////////////////
// Poll system events and handle keyboard input
///////////////////////////////////////////////////////////////////////////////
void process_input(void)
{
    const uint8_t *keystates = SDL_GetKeyboardState(NULL);

    if (keystates[SDL_SCANCODE_W]) // forward
    {
        update_camera_forward_velocity(vec3_mul(get_camera_direction(), 2.0 * delta_time));
        update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
    }
    if (keystates[SDL_SCANCODE_S]) // backward
    {
        update_camera_forward_velocity(vec3_mul(get_camera_direction(), 2.0 * delta_time));
        update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
    }
    if (keystates[SDL_SCANCODE_A]) // rotation radians/sec
        rotate_camera_yaw(-1.50 * delta_time);
    if (keystates[SDL_SCANCODE_D]) // rotation radians/secX
        rotate_camera_yaw(+1.50 * delta_time);
    if (keystates[SDL_SCANCODE_UP]) // move up
        rotate_camera_pitch(-1.50 * delta_time);
    if (keystates[SDL_SCANCODE_DOWN]) // move down
        rotate_camera_pitch(+1.50 * delta_time);

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

///////////////////////////////////////////////////////////////////////////////
// Process the graphics pipeline stages for all the mesh triangles
///////////////////////////////////////////////////////////////////////////////
// +-------------+
// | Model space |  <-- original mesh vertices
// +-------------+
// |   +-------------+
// `-> | World space |  <-- multiply by world matrix
//     +-------------+
//     |   +--------------+
//     `-> | Camera space |  <-- multiply by view matrix
//         +--------------+
//         |    +------------+
//         `--> |  Clipping  |  <-- clip against the six frustum planes
//              +------------+
//              |    +------------+
//              `--> | Projection |  <-- multiply by projection matrix
//                   +------------+
//                   |    +-------------+
//                   `--> | Image space |  <-- apply perspective divide
//                        +-------------+
//                        |    +--------------+
//                        `--> | Screen space |  <-- ready to render
//                             +--------------+
///////////////////////////////////////////////////////////////////////////////
void process_graphics_pipeline_stages(mesh_t *mesh)
{
    // Crear matriz de escalado, rotación y traslación que utilizará el multiplicador del mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);

    // Actualizamos el camaera look at para crear la matriz de vista
    vec3_t target = get_camera_lookat_target();
    vec3_t up_direction = vec3_new(0, 1, 0);
    view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

    // Iteramos todas las caras de la malla
    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++)
    {
        face_t mesh_face = mesh->faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[mesh_face.a];
        face_vertices[1] = mesh->vertices[mesh_face.b];
        face_vertices[2] = mesh->vertices[mesh_face.c];

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

        // Calculamos la normal de los triángulos
        vec3_t face_normal = get_triangle_normal(transformed_vertices);

        // Si el triángulo no está alineado con la cámara saltamos la iteración
        if (is_cull_backface())
        {
            // Buscamos el vector entre un punto del trángulo y el origen de la cámara
            // Figura "docs/15 camera raycast.png"
            vec3_t camera_ray = vec3_sub(vec3_new(0, 0, 0), vec3_from_vec4(transformed_vertices[0]));

            // Calculamos cuán alineado está el camera_ray respecto al vector normal
            // Utilizando para ello el producto escalar (el orden de los vectores no importa)
            float dot_normal_camera = vec3_dot(face_normal, camera_ray);

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
            float light_intensity_factor = -vec3_dot(face_normal, get_light_direction());

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
                .color = triangle_color,
                .texture = mesh->texture,
            };

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

///////////////////////////////////////////////////////////////////////////////
// Update function frame by frame with a fixed time step
///////////////////////////////////////////////////////////////////////////////
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

    // Iteramos todos los meshes de la escena
    for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++)
    {
        mesh_t *mesh = get_mesh(mesh_index);

        // Cambiamos los valores del mesh scale/rotation en cada frame
        // mesh.rotation.x += 0.0 * delta_time; // 1 pixel por segundo
        // mesh.rotation.y += 0.0 * delta_time;
        // mesh.rotation.z += 0.0 * delta_time;
        // mesh.translation.z = 5.0;

        // Process graphics pipeline stages for each mesh of our 3D scene
        process_graphics_pipeline_stages(mesh);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Render function to draw objects on the display
///////////////////////////////////////////////////////////////////////////////
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
                triangle.texture);
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

///////////////////////////////////////////////////////////////////////////////
// Free the memory that was dynamically allocated by the program
///////////////////////////////////////////////////////////////////////////////
void free_resources(void)
{
    free_meshes();
}

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////
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