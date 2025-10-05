#ifndef CONX_PHYSICS_H
#define CONX_PHYSICS_H

#include "conx_math.h"
#include <stdbool.h>

// Forward declaration
typedef struct ConXRigidBody ConXRigidBody;

// Collision callback
typedef void (*ConXCollisionCallback)(int body1_id, int body2_id, Vec3 normal);

// Physics body
typedef struct ConXRigidBody {
  Vec3 position;
  Vec3 velocity;
  Vec3 acceleration;
  float mass;
  float restitution;
  bool is_static;
  ConXCollisionCallback collision_callback;
} ConXRigidBody;

// Collision shapes
typedef enum {
  CONX_SHAPE_SPHERE,
  CONX_SHAPE_BOX
} ConXShapeType;

typedef struct {
  ConXShapeType type;
  union {
    float radius;           // for sphere
    Vec3 half_extents;      // for box
  };
} ConXCollisionShape;

// Physics world
typedef struct {
  ConXRigidBody *bodies;
  ConXCollisionShape *shapes;
  int body_count;
  int max_bodies;
  Vec3 gravity;
} ConXPhysicsWorld;

// Physics API
bool conx_physics_init(int max_bodies);
void conx_physics_shutdown(void);
void conx_physics_update(float dt);
void conx_physics_set_gravity(Vec3 gravity);

int conx_physics_create_body(Vec3 position, float mass);
void conx_physics_set_body_velocity(int body_id, Vec3 velocity);
void conx_physics_set_body_static(int body_id, bool is_static);
void conx_physics_add_sphere_shape(int body_id, float radius);
void conx_physics_add_box_shape(int body_id, Vec3 half_extents);

ConXRigidBody* conx_physics_get_body(int body_id);
ConXPhysicsWorld* conx_physics_get_world(void);
void conx_physics_set_collision_callback(int body_id, ConXCollisionCallback callback);

#endif