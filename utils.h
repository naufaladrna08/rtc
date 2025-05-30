#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <iostream> 
#include <limits>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <cstdlib>

// constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.142857142857142857142857143;

inline double ddegtorad(double degrees) {
  return degrees * pi / 180.0f;
}

inline double drandom() {
  return std::rand() / (RAND_MAX + 1.0f);
}

inline double drandom(double min, double max) {
  return min + (max - min) * drandom();
}

#ifdef OPTIMIZED_VECTOR
  #include "vec3_simd.h"
#else
  #include "vec3.h"
#endif
#include "color.h"
#include "ray.h"
#include "interval.h"

#endif