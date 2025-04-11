/*
 * Raytracing in One Weekend with C and optimizations 
 *
 * By: Your Name
 * Date: 2023-10-01 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "color.h"
#include "ray.h"
#include "vec3.h"

typedef vec3 color;
typedef vec3 point3;

double hit_sphere(const point3 center, double radius, const Ray r) {
  vec3 oc = vec3_sub(r.origin, center);
  double a = vec3_dot(r.direction, r.direction);
  double b = 2.0 * vec3_dot(oc, r.direction);
  double c = vec3_dot(oc, oc) - radius * radius;
  double discriminant = b * b - 4 * a * c;
  
  if (discriminant < 0) {
    return -1.0f; // No hit
  } else {
    double t = (-b - sqrt(discriminant)) / (2.0 * a);
    return t; // Hit at distance t
  }
}

color ray_color(Ray r) {
  // Check if the ray hits a sphere with center at (0,0,-1) and radius 0.5
  double t = hit_sphere(vec3_new(0.0, 0.0, -1.0), 0.5, r);
  if (t > 0.0) {
    vec3 N = vec3_sub(ray_at(r, t), vec3_new(0.0, 0.0, -1.0));
    N = vec3_normalize(N);
    return vec3_scale(vec3_add(N, vec3_new(1.0, 1.0, 1.0)), 0.5);
  }

  vec3 unit_direction = vec3_normalize(r.direction);
  double a = 0.5 * (unit_direction.y + 1.0);
  return vec3_add(vec3_scale(vec3_new(1.0, 1.0, 1.0), 1.0 - a), vec3_scale(vec3_new(0.5, 0.7, 1.0), a));
}

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <output_file>\n", argv[0]);
    return 1;
  }

  FILE* fp = fopen(argv[1], "w");
  if (!fp) {
    perror("Failed to open output file");
    return 1;
  }

  // [BEGIN] Create a vitual viewport
  double aspect_ratio = 16.0f / 9.0f;
  int image_width = 800;

  // Calculate the image height based on the aspect ratio
  int image_height = (int) (image_width / aspect_ratio);
  if (image_height < 1) image_height = 1;

  double focal_length = 1.0f;
  double viewport_height = 2.0f;
  double viewport_width = viewport_height * ((double) image_width / (double) image_height);
  vec3 camera_center = vec3_new(0.0f, 0.0f, 0.0f);

  // Create the viewport up and right vectors
  vec3 viewport_u = vec3_new(viewport_width, 0.0, 0.0);
  vec3 viewport_v = vec3_new(0.0, -viewport_height, 0.0);
  
  // Calculate the delta between the camera center and the viewport
  vec3 image_width_vec = vec3_new(viewport_width, 0.0f, 0.0f);
  vec3 image_height_vec = vec3_new(image_height, image_height, 0.0f);
  
  vec3 pixel_delta_u = vec3_scale(viewport_u, 1.0 / image_width);
  vec3 pixel_delta_v = vec3_scale(viewport_v, 1.0 / image_height);
  
  vec3 viewport_upper_left = vec3_sub(
      vec3_sub(
          vec3_sub(camera_center, vec3_new(0.0, 0.0, focal_length)),
          vec3_scale(viewport_u, 0.5)
      ),
      vec3_scale(viewport_v, 0.5)
  );
  
  vec3 pixel00_location = vec3_add(viewport_upper_left,
      vec3_add(vec3_scale(pixel_delta_u, 0.5), vec3_scale(pixel_delta_v, 0.5)));
  
  /* Render */
  fprintf(fp, "P3\n%d %d\n255\n", image_width, image_height);

  for (int j = image_height - 1; j >= 0; --j) {
    for (int i = 0; i < image_width; ++i) {
      vec3 pixel_center = vec3_add(pixel00_location, vec3_add(vec3_scale(pixel_delta_u, (double)i), vec3_scale(pixel_delta_v, (double)j)));
      vec3 ray_direction = vec3_sub(pixel_center, camera_center);

      Ray r = ray_new(camera_center, vec3_normalize(ray_direction));
      color pixel_color = ray_color(r);
      write_color(fp, pixel_color);
    }
  }

  fclose(fp);
  printf("Image saved to %s\n", argv[1]);

  return 0;
}


// GLOSSARY
// focal_length: The distance from the viewport and the camera center point.