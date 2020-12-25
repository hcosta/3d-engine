#include <stdio.h>  // para cargar NULL
#include <stdlib.h> /* strtof */
#include <string.h>
#include <stdbool.h>
#include "mesh.h"
#include "array.h"

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = {0, 0, 0}};

// Leemos el contenido del fichero .obj y cargamos los vértices
// y las caras en nuestro mesh.vertices y mesh.faces
void load_obj_file_data(char *filename)
{

    FILE *file = fopen(filename, "r"); /* should check the result */
    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        // Vertices
        if (strncmp(line, "v ", 2) == 0)
        {
            memmove(line, line + 2, strlen(line)); // remove first two characters
            vec3_t vertex;                         // then string to double (float)
            vertex.x = strtod(strtok(line, " "), NULL);
            vertex.y = strtod(strtok(NULL, " "), NULL);
            vertex.z = strtod(strtok(NULL, " "), NULL);
            array_push(mesh.vertices, vertex);
            //printf("%f %f %f\n", vertex.x, vertex.y, vertex.z);
        }

        // Faces
        if (strncmp(line, "f ", 2) == 0)
        {
            memmove(line, line + 2, strlen(line)); // remove first two characters
            face_t face;                           // then string to long (int)
            // For each face split " " then split "/" and get only first number and converse to int
            char *end_str, *end_token, *end;
            face.a = strtol(strtok_r(strtok_r(line, " ", &end_str), "/", &end_token), &end, 10);
            face.b = strtol(strtok_r(strtok_r(NULL, " ", &end_str), "/", &end_token), &end, 10);
            face.c = strtol(strtok_r(strtok_r(NULL, " ", &end_str), "/", &end_token), &end, 10);
            array_push(mesh.faces, face);
        }
    }
    fclose(file);
}

// Versión usando sscanf en porciones
void load_obj_file_data_sscanf(char *filename)
{
    FILE *file;
    file = fopen(filename, "r");
    char line[1024];

    while (fgets(line, 1024, file))
    {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0)
        {
            vec3_t vertex;
            sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            array_push(mesh.vertices, vertex);
        }
        // Face information
        if (strncmp(line, "f ", 2) == 0)
        {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];
            sscanf(
                line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0], &texture_indices[0], &normal_indices[0],
                &vertex_indices[1], &texture_indices[1], &normal_indices[1],
                &vertex_indices[2], &texture_indices[2], &normal_indices[2]);
            face_t face = {
                .a = vertex_indices[0],
                .b = vertex_indices[1],
                .c = vertex_indices[2]};
            array_push(mesh.faces, face);
        }
    }
}

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    {.x = -1, .y = -1, .z = -1}, // 1
    {.x = -1, .y = 1, .z = -1},  // 2
    {.x = 1, .y = 1, .z = -1},   // 3
    {.x = 1, .y = -1, .z = -1},  // 4
    {.x = 1, .y = 1, .z = 1},    // 5
    {.x = 1, .y = -1, .z = 1},   // 6
    {.x = -1, .y = 1, .z = 1},   // 7
    {.x = -1, .y = -1, .z = 1},  // 8
};

face_t cube_faces[N_CUBE_FACES] = {
    // frontal
    {.a = 1, .b = 2, .c = 3},
    {.a = 1, .b = 3, .c = 4},
    // derecha
    {.a = 4, .b = 3, .c = 5},
    {.a = 4, .b = 5, .c = 6},
    // trasera
    {.a = 6, .b = 5, .c = 7},
    {.a = 6, .b = 7, .c = 8},
    // izquierda
    {.a = 8, .b = 7, .c = 2},
    {.a = 8, .b = 2, .c = 1},
    // superior
    {.a = 2, .b = 7, .c = 5},
    {.a = 2, .b = 5, .c = 3},
    // inferior
    {.a = 6, .b = 8, .c = 1},
    {.a = 6, .b = 1, .c = 4},
};

void load_cube_mesh_data(void)
{
    for (int i = 0; i < N_CUBE_VERTICES; i++)
    {
        vec3_t cube_vertex = cube_vertices[i];
        array_push(mesh.vertices, cube_vertex);
    }

    for (int i = 0; i < N_CUBE_FACES; i++)
    {
        face_t cube_face = cube_faces[i];
        array_push(mesh.faces, cube_face);
    }
}