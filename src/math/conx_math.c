#include "conx_math.h"

Vec2 vec2_create(float x, float y) {
  Vec2 v = {x, y};
  return v;
}

Vec3 vec3_create(float x, float y, float z) {
  Vec3 v = {x, y, z};
  return v;
}

Vec3 vec3_add(Vec3 a, Vec3 b) {
  Vec3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
  return result;
}

Vec3 vec3_subtract(Vec3 a, Vec3 b) {
  Vec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
  return result;
}

Vec3 vec3_multiply(Vec3 v, float scalar) {
  Vec3 result = {v.x * scalar, v.y * scalar, v.z * scalar};
  return result;
}

float vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 vec3_cross(Vec3 a, Vec3 b) {
  Vec3 result = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                 a.x * b.y - a.y * b.x};
  return result;
}

float vec3_length(Vec3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

Vec3 vec3_normalize(Vec3 v) {
  float length = vec3_length(v);
  if (length > 0) {
    return vec3_multiply(v, 1.0f / length);
  }
  return v;
}

Mat4 mat4_identity(void) {
  Mat4 result = {0};
  result.m[0][0] = 1.0f;
  result.m[1][1] = 1.0f;
  result.m[2][2] = 1.0f;
  result.m[3][3] = 1.0f;
  return result;
}

Mat4 mat4_multiply(Mat4 a, Mat4 b) {
  Mat4 result = {0};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      result.m[i][j] = 0;
      for (int k = 0; k < 4; k++) {
        result.m[i][j] += a.m[i][k] * b.m[k][j];
      }
    }
  }
  return result;
}

Mat4 mat4_translate(Mat4 m, Vec3 translation) {
  Mat4 translate = mat4_identity();
  translate.m[3][0] = translation.x;
  translate.m[3][1] = translation.y;
  translate.m[3][2] = translation.z;
  return mat4_multiply(m, translate);
}

Mat4 mat4_rotate(Mat4 m, float angle, Vec3 axis) {
  // This is a placeholder implementation
  return m;
}

Mat4 mat4_scale(Mat4 m, Vec3 scale) {
  Mat4 scale_mat = mat4_identity();
  scale_mat.m[0][0] = scale.x;
  scale_mat.m[1][1] = scale.y;
  scale_mat.m[2][2] = scale.z;
  return mat4_multiply(m, scale_mat);
}

Mat4 mat4_perspective(float fov, float aspect, float near, float far) {
  Mat4 result = {0};
  float tan_half_fov = tanf(fov / 2.0f);

  result.m[0][0] = 1.0f / (aspect * tan_half_fov);
  result.m[1][1] = 1.0f / tan_half_fov;
  result.m[2][2] = -(far + near) / (far - near);
  result.m[2][3] = -1.0f;
  result.m[3][2] = -(2.0f * far * near) / (far - near);

  return result;
}

Mat4 mat4_orthographic(float left, float right, float bottom, float top,
                       float near, float far) {
  Mat4 result = mat4_identity();

  result.m[0][0] = 2.0f / (right - left);
  result.m[1][1] = 2.0f / (top - bottom);
  result.m[2][2] = -2.0f / (far - near);
  result.m[3][0] = -(right + left) / (right - left);
  result.m[3][1] = -(top + bottom) / (top - bottom);
  result.m[3][2] = -(far + near) / (far - near);

  return result;
}
