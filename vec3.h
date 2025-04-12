#ifndef VEC_H
#define VEC_H

#include <cmath>
#include <iostream>

double drandom();
double drandom(double min, double max);

class vec3 {
  public:
    double e[3];

    vec3() : e{0, 0, 0} { }
    vec3(double e1, double e2, double e3) : e{e1, e2, e3} {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }

    vec3& operator+=(const vec3& other) {
      e[0] += other.e[0];
      e[1] += other.e[1];
      e[2] += other.e[2];
      return *this;
    }

    vec3& operator*=(double t) {
      e[0] *= t;
      e[1] *= t;
      e[2] *= t;
      return *this;
    }

    vec3& operator/=(double t) {
      return *this *= 1/t;
    }

    double length() const {
      return std::sqrt(length_squared());
    }

    double length_squared() const {
      // x^2 + y^2 + z^2
      return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
    }

    static vec3 random() {
      return vec3(drandom(), drandom(), drandom());
    }

    static vec3 random(double min, double max) {
      return vec3(drandom(min, max), drandom(min, max), drandom(min, max));
    }

    bool near_zero() const {
      auto s = 1e-8;
      return (std::fabs(e[0] < 0) && std::fabs(e[1] < 0) && std::fabs(e[2] < 0));
    }
};

// Just for geometry clarity
using point3 = vec3;

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
  return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
  return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
  return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
  return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3& v) {
  return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3& v, double t) {
  return t * v;
}

inline vec3 operator/(const vec3& v, double t) {
  return (1 / t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
  return u.e[0] * v.e[0] + 
         u.e[1] * v.e[1] +
         u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
  return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
              u.e[2] * v.e[0] - u.e[0] * v.e[2],
              u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3& v) {
  return v / v.length();
}

inline vec3 random_unit_vector() {
  while (true) {
    auto p = vec3::random(-1, 1);
    auto lensq = p.length_squared();
    if (1e-160 < lensq && lensq <= 1)
      return p / sqrt(lensq);
  }
}

inline vec3 random_on_hemisphere(const vec3& normal) {
  vec3 onUnitSphere = random_unit_vector();
  if (dot(onUnitSphere, normal) > 0.0f)
    return onUnitSphere;
  else 
    return -onUnitSphere;
}

inline vec3 reflect(const vec3& v, const vec3& n) {
  return v - 2 * dot(v, n) * n;
}

inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
  auto cos_tetha = std::fmin(dot(-uv, n), 1.0f);
  vec3 r_out_perp = etai_over_etat * (uv + cos_tetha * n);
  vec3 r_out_parallel = -std::sqrt(std::fabs(1.0f - r_out_perp.length_squared())) * n;
  return r_out_perp + r_out_parallel;
}

inline vec3 random_in_unit_disk() {
  while (true) {
    auto p = vec3(drandom(-1, 1), drandom(-1, 1), 0.0f);
    if (p.length_squared() < 1) return p;
  }
}

#endif