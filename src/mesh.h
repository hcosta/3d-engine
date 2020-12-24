#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2) // cubo de 6 caras, 2 triángulos por cara

extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_FACES];

// Definimos una estructura para mallas de tamaño dinámico
// Con un array de vértices y caras
typedef struct
{
    vec3_t *vertices; // array dinámico de vértices
    face_t *faces;    // array dinámico de caras
    vec3_t rotation;  // rotación en x, y, z
} mesh_t;

extern mesh_t mesh;

void load_cube_mesh_data(void);

#endif