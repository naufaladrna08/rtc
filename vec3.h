/*
 * vec3.h
 *
 * vec3 - A simple 3D vector library for C
 */

#ifndef VEC3_H
#define VEC3_H
#include <math.h>

typedef struct {
  float x;
  float y;
  float z;
} vec3;

static inline vec3 vec3_new(float x, float y, float z) {
  vec3 v;
  v.x = x;
  v.y = y;
  v.z = z;
  return v;
}

static inline vec3 vec3_add(vec3 a, vec3 b) {
  return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline vec3 vec3_sub(vec3 a, vec3 b) {
  return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline vec3 vec3_mul(vec3 a, vec3 b) {
  return (vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}

static inline vec3 vec3_div(vec3 a, vec3 b) {
  return (vec3){a.x / b.x, a.y / b.y, a.z / b.z};
}

static inline vec3 vec3_scale(vec3 a, float s) {
  return (vec3){a.x * s, a.y * s, a.z * s};
}

static inline vec3 vec3_normalize(vec3 a) {
  float length = sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
  return (vec3){a.x / length, a.y / length, a.z / length};
}

static inline float vec3_dot(vec3 a, vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline vec3 vec3_cross(vec3 a, vec3 b) {
  return (vec3){
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x
  };
}

static inline vec3 vec3_unit(vec3 a) {
  return vec3_scale(a, 1.0f / sqrtf(vec3_dot(a, a)));
}

#endif