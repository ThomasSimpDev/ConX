#ifndef CONX_3D_H
#define CONX_3D_H

#include "conx_math.h"
#include <stdbool.h>

// 3D Camera
typedef struct {
  Vec3 position;
  Vec3 target;
  Vec3 up;
  float fov;
  float aspect;
  float near_plane;
  float far_plane;
} ConXCamera;

// 3D Mesh
typedef struct {
  float *vertices;
  unsigned int *indices;
  int vertex_count;
  int index_count;
  unsigned int VAO, VBO, EBO;
} ConXMesh;

// 3D Object
typedef struct {
  Vec3 position;
  Vec3 rotation;
  Vec3 scale;
  Vec4 color;
  ConXMesh *mesh;
} ConXObject3D;

// 3D subsystem
bool conx_3d_init(void);
void conx_3d_shutdown(void);
void conx_3d_set_camera(ConXCamera *camera);
ConXCamera *conx_3d_get_camera(void);

// Mesh creation
ConXMesh *conx_create_cube_mesh(void);
ConXMesh *conx_create_sphere_mesh(int segments);
void conx_free_mesh(ConXMesh *mesh);

// 3D drawing functions
void conx_draw_cube(Vec3 position, Vec3 size, Vec4 color);
void conx_draw_sphere(Vec3 position, float radius, Vec4 color);
void conx_draw_object_3d(ConXObject3D *object);

// Camera utilities
ConXCamera conx_camera_create(Vec3 position, Vec3 target, float fov);
void conx_camera_look_at(ConXCamera *camera, Vec3 position, Vec3 target, Vec3 up);

#endif