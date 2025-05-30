#ifndef RAY_H
#define RAY_H
#ifdef OPTIMIZED_VECTOR
  #include "vec3_simd.h"
#else
  #include "vec3.h"
#endif

class ray {
  public: 
    ray() {}
    ray(const point3& origin, const point3& direction) : m_orig(origin), m_dir(direction) { }

    point3 at(double t) const {
      return m_orig + t * m_dir;
    }

    const point3& origin() const { return m_orig; }
    const vec3& direction() const { return m_dir; }

  private:
    point3 m_orig;
    vec3 m_dir;
};

#endif