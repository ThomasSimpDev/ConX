#ifndef CONX_MATH_H
#define CONX_MATH_H

#include <math.h>

// 2D Vector
typedef struct {
  float x, y;
} Vec2;

// 3D Vector
typedef struct {
  float x, y, z;
} Vec3;

// 4D Vector
typedef struct {
  float x, y, z, w;
} Vec4;

// 4x4 Matrix
typedef struct {
  float m[4][4];
} Mat4;

// Vector operations
Vec2 vec2_create(float x, float y);
Vec3 vec3_create(float x, float y, float z);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_subtract(Vec3 a, Vec3 b);
Vec3 vec3_multiply(Vec3 v, float scalar);
float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
float vec3_length(Vec3 v);
Vec3 vec3_normalize(Vec3 v);

// Matrix operations
Mat4 mat4_identity(void);
Mat4 mat4_multiply(Mat4 a, Mat4 b);
Mat4 mat4_translate(Mat4 m, Vec3 translation);
Mat4 mat4_rotate(Mat4 m, float angle, Vec3 axis);
Mat4 mat4_scale(Mat4 m, Vec3 scale);
Mat4 mat4_perspective(float fov, float aspect, float near, float far);
Mat4 mat4_orthographic(float left, float right, float bottom, float top,
                       float near, float far);

#endif
