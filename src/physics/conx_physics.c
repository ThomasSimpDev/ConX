#include "conx_physics.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static ConXPhysicsWorld physics_world = {0};

bool conx_physics_init(int max_bodies) {
  physics_world.bodies = malloc(sizeof(ConXRigidBody) * max_bodies);
  physics_world.shapes = malloc(sizeof(ConXCollisionShape) * max_bodies);
  
  if (!physics_world.bodies || !physics_world.shapes) {
    conx_physics_shutdown();
    return false;
  }
  
  physics_world.max_bodies = max_bodies;
  physics_world.body_count = 0;
  physics_world.gravity = vec3_create(0.0f, -9.81f, 0.0f);
  
  return true;
}

void conx_physics_shutdown(void) {
  if (physics_world.bodies) {
    free(physics_world.bodies);
    physics_world.bodies = NULL;
  }
  if (physics_world.shapes) {
    free(physics_world.shapes);
    physics_world.shapes = NULL;
  }
  physics_world.body_count = 0;
  physics_world.max_bodies = 0;
}

void conx_physics_set_gravity(Vec3 gravity) {
  physics_world.gravity = gravity;
}

int conx_physics_create_body(Vec3 position, float mass) {
  if (physics_world.body_count >= physics_world.max_bodies) {
    return -1;
  }
  
  int id = physics_world.body_count++;
  ConXRigidBody *body = &physics_world.bodies[id];
  
  body->position = position;
  body->velocity = vec3_create(0.0f, 0.0f, 0.0f);
  body->acceleration = vec3_create(0.0f, 0.0f, 0.0f);
  body->mass = mass;
  body->restitution = 0.5f;
  body->is_static = false;
  body->collision_callback = NULL;
  
  // Initialize shape as sphere with radius 0.5
  physics_world.shapes[id].type = CONX_SHAPE_SPHERE;
  physics_world.shapes[id].radius = 0.5f;
  
  return id;
}

void conx_physics_set_body_velocity(int body_id, Vec3 velocity) {
  if (body_id >= 0 && body_id < physics_world.body_count) {
    physics_world.bodies[body_id].velocity = velocity;
  }
}

void conx_physics_set_body_static(int body_id, bool is_static) {
  if (body_id >= 0 && body_id < physics_world.body_count) {
    physics_world.bodies[body_id].is_static = is_static;
  }
}

void conx_physics_add_sphere_shape(int body_id, float radius) {
  if (body_id >= 0 && body_id < physics_world.body_count) {
    physics_world.shapes[body_id].type = CONX_SHAPE_SPHERE;
    physics_world.shapes[body_id].radius = radius;
  }
}

void conx_physics_add_box_shape(int body_id, Vec3 half_extents) {
  if (body_id >= 0 && body_id < physics_world.body_count) {
    physics_world.shapes[body_id].type = CONX_SHAPE_BOX;
    physics_world.shapes[body_id].half_extents = half_extents;
  }
}

ConXRigidBody* conx_physics_get_body(int body_id) {
  if (body_id >= 0 && body_id < physics_world.body_count) {
    return &physics_world.bodies[body_id];
  }
  return NULL;
}

ConXPhysicsWorld* conx_physics_get_world(void) {
  return &physics_world;
}

void conx_physics_set_collision_callback(int body_id, ConXCollisionCallback callback) {
  if (body_id >= 0 && body_id < physics_world.body_count) {
    physics_world.bodies[body_id].collision_callback = callback;
  }
}

static bool check_sphere_collision(Vec3 pos1, float r1, Vec3 pos2, float r2, Vec3 *normal) {
  Vec3 diff = vec3_subtract(pos1, pos2);
  float distance = vec3_length(diff);
  if (distance < (r1 + r2)) {
    *normal = vec3_normalize(diff);
    return true;
  }
  return false;
}

static bool check_sphere_box_collision(Vec3 sphere_pos, float radius, Vec3 box_pos, Vec3 half_extents, Vec3 *normal) {
  Vec3 closest = {
    fmaxf(box_pos.x - half_extents.x, fminf(sphere_pos.x, box_pos.x + half_extents.x)),
    fmaxf(box_pos.y - half_extents.y, fminf(sphere_pos.y, box_pos.y + half_extents.y)),
    fmaxf(box_pos.z - half_extents.z, fminf(sphere_pos.z, box_pos.z + half_extents.z))
  };
  
  Vec3 diff = vec3_subtract(sphere_pos, closest);
  float distance = vec3_length(diff);
  
  if (distance < radius) {
    *normal = distance > 0 ? vec3_normalize(diff) : vec3_create(0, 1, 0);
    return true;
  }
  return false;
}

static bool check_box_collision(Vec3 pos1, Vec3 half1, Vec3 pos2, Vec3 half2, Vec3 *normal) {
  Vec3 diff = vec3_subtract(pos1, pos2);
  Vec3 abs_diff = {fabsf(diff.x), fabsf(diff.y), fabsf(diff.z)};
  Vec3 overlap = vec3_subtract(vec3_add(half1, half2), abs_diff);
  
  if (overlap.x > 0 && overlap.y > 0 && overlap.z > 0) {
    // Find axis with minimum overlap
    if (overlap.x < overlap.y && overlap.x < overlap.z) {
      *normal = vec3_create(diff.x > 0 ? 1 : -1, 0, 0);
    } else if (overlap.y < overlap.z) {
      *normal = vec3_create(0, diff.y > 0 ? 1 : -1, 0);
    } else {
      *normal = vec3_create(0, 0, diff.z > 0 ? 1 : -1);
    }
    return true;
  }
  return false;
}

static void resolve_collision(ConXRigidBody *body1, ConXRigidBody *body2, Vec3 normal) {
  if (body1->is_static && body2->is_static) return;
  
  Vec3 relative_velocity = vec3_subtract(body1->velocity, body2->velocity);
  float velocity_along_normal = vec3_dot(relative_velocity, normal);
  
  if (velocity_along_normal > 0) return;
  
  float restitution = (body1->restitution + body2->restitution) * 0.5f;
  float impulse_scalar = -(1 + restitution) * velocity_along_normal;
  
  if (!body1->is_static && !body2->is_static) {
    impulse_scalar /= (1.0f / body1->mass + 1.0f / body2->mass);
  } else if (body1->is_static) {
    impulse_scalar /= (1.0f / body2->mass);
  } else {
    impulse_scalar /= (1.0f / body1->mass);
  }
  
  Vec3 impulse = vec3_multiply(normal, impulse_scalar);
  
  if (!body1->is_static) {
    body1->velocity = vec3_add(body1->velocity, vec3_multiply(impulse, 1.0f / body1->mass));
  }
  if (!body2->is_static) {
    body2->velocity = vec3_subtract(body2->velocity, vec3_multiply(impulse, 1.0f / body2->mass));
  }
}

void conx_physics_update(float dt) {
  // Apply gravity and integrate
  for (int i = 0; i < physics_world.body_count; i++) {
    ConXRigidBody *body = &physics_world.bodies[i];
    
    if (body->is_static) continue;
    
    // Apply gravity
    body->acceleration = vec3_add(body->acceleration, physics_world.gravity);
    
    // Integrate velocity
    body->velocity = vec3_add(body->velocity, vec3_multiply(body->acceleration, dt));
    
    // Integrate position
    body->position = vec3_add(body->position, vec3_multiply(body->velocity, dt));
    
    // Reset acceleration
    body->acceleration = vec3_create(0.0f, 0.0f, 0.0f);
  }
  
  // Check collisions
  for (int i = 0; i < physics_world.body_count; i++) {
    for (int j = i + 1; j < physics_world.body_count; j++) {
      ConXRigidBody *body1 = &physics_world.bodies[i];
      ConXRigidBody *body2 = &physics_world.bodies[j];
      ConXCollisionShape *shape1 = &physics_world.shapes[i];
      ConXCollisionShape *shape2 = &physics_world.shapes[j];
      
      Vec3 normal;
      bool collision = false;
      
      if (shape1->type == CONX_SHAPE_SPHERE && shape2->type == CONX_SHAPE_SPHERE) {
        collision = check_sphere_collision(body1->position, shape1->radius, 
                                         body2->position, shape2->radius, &normal);
      } else if (shape1->type == CONX_SHAPE_SPHERE && shape2->type == CONX_SHAPE_BOX) {
        collision = check_sphere_box_collision(body1->position, shape1->radius,
                                             body2->position, shape2->half_extents, &normal);
      } else if (shape1->type == CONX_SHAPE_BOX && shape2->type == CONX_SHAPE_SPHERE) {
        collision = check_sphere_box_collision(body2->position, shape2->radius,
                                             body1->position, shape1->half_extents, &normal);
        normal = vec3_multiply(normal, -1.0f);
      } else if (shape1->type == CONX_SHAPE_BOX && shape2->type == CONX_SHAPE_BOX) {
        collision = check_box_collision(body1->position, shape1->half_extents,
                                      body2->position, shape2->half_extents, &normal);
      }
      
      if (collision) {
        // Call collision callbacks
        if (body1->collision_callback) {
          body1->collision_callback(i, j, normal);
        }
        if (body2->collision_callback) {
          body2->collision_callback(j, i, vec3_multiply(normal, -1.0f));
        }
        
        resolve_collision(body1, body2, normal);
      }
    }
  }
}