#ifndef VECTOR_H
#define VECTOR_H

typedef struct
{
    float x, y;
} vec2_t;

typedef struct
{
    float x, y, z;
} vec3_t;

// Definición Funciones Vectores 2D
float vec_2_length(vec2_t v);
// TODO: vec2_add, vec2_sub, vec2_mul, etc

// Definición Funciones Vectores 3D
float vec_3_length(vec3_t v);
// TODO: vec3_add, vec3_sub, vec3_mul, etc

vec3_t vec3_rotate_x(vec3_t v, float angle);
vec3_t vec3_rotate_y(vec3_t v, float angle);
vec3_t vec3_rotate_z(vec3_t v, float angle);

#endif