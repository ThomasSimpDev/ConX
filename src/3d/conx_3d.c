#include "conx_3d.h"
#include "conx.h"
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ConXCamera current_camera;
static bool is_3d_initialized = false;

bool conx_3d_init(void) {
  if (is_3d_initialized) return true;

  // Initialize default camera
  current_camera = conx_camera_create(
    vec3_create(0.0f, 0.0f, 5.0f),
    vec3_create(0.0f, 0.0f, 0.0f),
    45.0f
  );

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  is_3d_initialized = true;
  printf("ConX 3D subsystem initialized\n");
  return true;
}

void conx_3d_shutdown(void) {
  if (!is_3d_initialized) return;
  
  glDisable(GL_DEPTH_TEST);
  is_3d_initialized = false;
  printf("ConX 3D subsystem shutdown\n");
}

void conx_3d_set_camera(ConXCamera *camera) {
  if (!camera) return;
  current_camera = *camera;
}

ConXCamera *conx_3d_get_camera(void) {
  return &current_camera;
}

ConXCamera conx_camera_create(Vec3 position, Vec3 target, float fov) {
  ConXCamera camera;
  camera.position = position;
  camera.target = target;
  camera.up = vec3_create(0.0f, 1.0f, 0.0f);
  camera.fov = fov;
  camera.aspect = 800.0f / 600.0f; // Default aspect ratio
  camera.near_plane = 0.1f;
  camera.far_plane = 100.0f;
  return camera;
}

void conx_camera_look_at(ConXCamera *camera, Vec3 position, Vec3 target, Vec3 up) {
  if (!camera) return;
  camera->position = position;
  camera->target = target;
  camera->up = up;
}

static void setup_3d_projection(void) {
  // Get current window size for aspect ratio
  ConXEngine *engine = conx_get_engine();
  if (engine && engine->window) {
    int width, height;
    SDL_GetWindowSize((SDL_Window*)engine->window, &width, &height);
    current_camera.aspect = (float)width / (float)height;
  }
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(current_camera.fov, current_camera.aspect, 
                 current_camera.near_plane, current_camera.far_plane);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(current_camera.position.x, current_camera.position.y, current_camera.position.z,
            current_camera.target.x, current_camera.target.y, current_camera.target.z,
            current_camera.up.x, current_camera.up.y, current_camera.up.z);
}

void conx_draw_cube(Vec3 position, Vec3 size, Vec4 color) {
  if (!is_3d_initialized) return;

  setup_3d_projection();
  
  glPushMatrix();
  glTranslatef(position.x, position.y, position.z);
  glScalef(size.x, size.y, size.z);
  glColor4f(color.x, color.y, color.z, color.w);

  // Draw cube using immediate mode (simple implementation)
  glBegin(GL_QUADS);
  
  // Front face
  glVertex3f(-0.5f, -0.5f,  0.5f);
  glVertex3f( 0.5f, -0.5f,  0.5f);
  glVertex3f( 0.5f,  0.5f,  0.5f);
  glVertex3f(-0.5f,  0.5f,  0.5f);
  
  // Back face
  glVertex3f(-0.5f, -0.5f, -0.5f);
  glVertex3f(-0.5f,  0.5f, -0.5f);
  glVertex3f( 0.5f,  0.5f, -0.5f);
  glVertex3f( 0.5f, -0.5f, -0.5f);
  
  // Top face
  glVertex3f(-0.5f,  0.5f, -0.5f);
  glVertex3f(-0.5f,  0.5f,  0.5f);
  glVertex3f( 0.5f,  0.5f,  0.5f);
  glVertex3f( 0.5f,  0.5f, -0.5f);
  
  // Bottom face
  glVertex3f(-0.5f, -0.5f, -0.5f);
  glVertex3f( 0.5f, -0.5f, -0.5f);
  glVertex3f( 0.5f, -0.5f,  0.5f);
  glVertex3f(-0.5f, -0.5f,  0.5f);
  
  // Right face
  glVertex3f( 0.5f, -0.5f, -0.5f);
  glVertex3f( 0.5f,  0.5f, -0.5f);
  glVertex3f( 0.5f,  0.5f,  0.5f);
  glVertex3f( 0.5f, -0.5f,  0.5f);
  
  // Left face
  glVertex3f(-0.5f, -0.5f, -0.5f);
  glVertex3f(-0.5f, -0.5f,  0.5f);
  glVertex3f(-0.5f,  0.5f,  0.5f);
  glVertex3f(-0.5f,  0.5f, -0.5f);
  
  glEnd();
  glPopMatrix();
}

void conx_draw_sphere(Vec3 position, float radius, Vec4 color) {
  if (!is_3d_initialized) return;

  setup_3d_projection();
  
  glPushMatrix();
  glTranslatef(position.x, position.y, position.z);
  glColor4f(color.x, color.y, color.z, color.w);

  // Simple sphere using GLU
  GLUquadric *quad = gluNewQuadric();
  gluSphere(quad, radius, 16, 16);
  gluDeleteQuadric(quad);
  
  glPopMatrix();
}

ConXMesh *conx_create_cube_mesh(void) {
  ConXMesh *mesh = malloc(sizeof(ConXMesh));
  if (!mesh) return NULL;

  // Cube vertices (position only for simplicity)
  float vertices[] = {
    // Front face
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    // Back face
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f
  };

  unsigned int indices[] = {
    0, 1, 2, 2, 3, 0,   // Front
    4, 5, 6, 6, 7, 4,   // Back
    5, 3, 2, 2, 6, 5,   // Top
    4, 7, 1, 1, 0, 4,   // Bottom
    7, 6, 2, 2, 1, 7,   // Right
    4, 0, 3, 3, 5, 4    // Left
  };

  mesh->vertex_count = 8;
  mesh->index_count = 36;
  
  mesh->vertices = malloc(sizeof(vertices));
  mesh->indices = malloc(sizeof(indices));
  
  memcpy(mesh->vertices, vertices, sizeof(vertices));
  memcpy(mesh->indices, indices, sizeof(indices));

  return mesh;
}

ConXMesh *conx_create_sphere_mesh(int segments) {
  // Simplified sphere mesh creation
  ConXMesh *mesh = malloc(sizeof(ConXMesh));
  if (!mesh) return NULL;
  
  // For simplicity, return a basic sphere representation
  // In a full implementation, this would generate proper sphere geometry
  mesh->vertex_count = segments * segments;
  mesh->index_count = segments * segments * 6;
  mesh->vertices = NULL; // Would be allocated and filled
  mesh->indices = NULL;  // Would be allocated and filled
  
  return mesh;
}

void conx_free_mesh(ConXMesh *mesh) {
  if (!mesh) return;
  
  if (mesh->vertices) free(mesh->vertices);
  if (mesh->indices) free(mesh->indices);
  free(mesh);
}

void conx_draw_object_3d(ConXObject3D *object) {
  if (!object || !is_3d_initialized) return;

  setup_3d_projection();
  
  glPushMatrix();
  glTranslatef(object->position.x, object->position.y, object->position.z);
  glRotatef(object->rotation.x, 1.0f, 0.0f, 0.0f);
  glRotatef(object->rotation.y, 0.0f, 1.0f, 0.0f);
  glRotatef(object->rotation.z, 0.0f, 0.0f, 1.0f);
  glScalef(object->scale.x, object->scale.y, object->scale.z);
  glColor4f(object->color.x, object->color.y, object->color.z, object->color.w);

  // Draw the mesh (simplified)
  if (object->mesh && object->mesh->vertices) {
    // Would render the actual mesh here
  }
  
  glPopMatrix();
}