#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"
#include "upng.h"

// Definimos una estructura para mallas de tamaño dinámico con un array de vértices y caras
typedef struct mesh_t
{
    vec3_t *vertices;   // array dinámico de vértices
    face_t *faces;      // array dinámico de caras
    upng_t *texture;    // mesh PNG texture pointer
    vec3_t rotation;    // rotación en x, y, z
    vec3_t scale;       // escalado en x, y, z
    vec3_t translation; // traslación en x, y, z
} mesh_t;

void load_mesh(char *obj_filename, char *png_filename, vec3_t scale, vec3_t translation, vec3_t rotation);
void load_mesh_obj_data(mesh_t *mesh, char *obj_filename);
void load_mesh_png_data(mesh_t *mesh, char *png_filename);

int get_num_meshes(void);
mesh_t *get_mesh(int index);

void free_meshes(void);

#endif