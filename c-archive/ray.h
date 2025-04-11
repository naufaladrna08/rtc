#ifndef RAY_H
#define RAY_H
#include <stdlib.h>
#include "vec3.h"

typedef struct {
  vec3 origin;
  vec3 direction;
} Ray;

// Function to create a new ray
static inline Ray ray_new(vec3 origin, vec3 direction) {
  Ray ray;
  ray.origin = origin;
  ray.direction = direction;
  return ray;
}

// Function to get the point at a given time along the ray
static inline vec3 ray_at(Ray ray, float t) {
  return vec3_add(ray.origin, vec3_scale(ray.direction, t));
}

// Function to get the origin of the ray
static inline vec3 ray_origin(Ray ray) {
  return ray.origin;
}

// Function to get the direction of the ray
static inline vec3 ray_direction(Ray ray) {
  return ray.direction;
}

#endif