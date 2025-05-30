#ifndef COLOR_H
#define COLOR_H
#include <iostream>
#ifdef OPTIMIZED_VECTOR
  #include "vec3_simd.h"
#else
  #include "vec3.h"
#endif
#include "interval.h"

using color = vec3;

inline double linear_to_gamma(double linear_component) {
  if (linear_component > 0) {
    return std::sqrt(linear_component);
  }

  return 0;
}

void write_color(std::ostream& out, const color& pixel_color) {
  static const interval intensity(0.000f, 0.999f);
  int rbyte = int(256 * intensity.clamp(linear_to_gamma(pixel_color.x())));
  int gbyte = int(256 * intensity.clamp(linear_to_gamma(pixel_color.y())));
  int bbyte = int(256 * intensity.clamp(linear_to_gamma(pixel_color.z())));

  out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif