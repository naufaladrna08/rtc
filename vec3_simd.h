#ifndef VEC_H
#define VEC_H

#include <cmath>
#include <iostream>
#include <immintrin.h> // For AVX intrinsics

double drandom();
double drandom(double min, double max);

class alignas(32) vec3 { 
public:
  union {
    __m256d e_simd;
    double e[4];
  };

  vec3() : e_simd(_mm256_setzero_pd()) {}
  vec3(double e1, double e2, double e3) : e_simd(_mm256_setr_pd(e1, e2, e3, 0.0)) {}
  explicit vec3(__m256d v) : e_simd(v) {}

  double x() const { return e[0]; }
  double y() const { return e[1]; }
  double z() const { return e[2]; }

  vec3 operator-() const { 
      return vec3(_mm256_sub_pd(_mm256_setzero_pd(), e_simd)); 
  }

  double operator[](int i) const { return e[i]; }
  double& operator[](int i) { return e[i]; }

  vec3& operator+=(const vec3& other) {
      e_simd = _mm256_add_pd(e_simd, other.e_simd);
      return *this;
  }

  vec3& operator*=(double t) {
      e_simd = _mm256_mul_pd(e_simd, _mm256_set1_pd(t));
      return *this;
  }

  vec3& operator/=(double t) {
      return *this *= 1/t;
  }

  double length() const {
      return std::sqrt(length_squared());
  }

  double length_squared() const {
      __m256d sq = _mm256_mul_pd(e_simd, e_simd);
      __m128d low = _mm256_castpd256_pd128(sq);
      __m128d high = _mm256_extractf128_pd(sq, 1);
      __m128d sum_xy = _mm_hadd_pd(low, low);
      __m128d result = _mm_add_sd(sum_xy, high);
      return _mm_cvtsd_f64(result);
  }

  static vec3 random() {
      return vec3(drandom(), drandom(), drandom());
  }

  static vec3 random(double min, double max) {
      return vec3(drandom(min, max), drandom(min, max), drandom(min, max));
  }

  bool near_zero() const {
      const double s = 1e-8;
      __m256d abs_mask = _mm256_castsi256_pd(_mm256_set1_epi64x(0x7FFFFFFFFFFFFFFF));
      __m256d abs_e = _mm256_and_pd(e_simd, abs_mask);
      __m256d cmp = _mm256_cmp_pd(abs_e, _mm256_set1_pd(s), _CMP_LT_OQ);
      int mask = _mm256_movemask_pd(cmp);
      return (mask & 7) == 7;
  }
};

using point3 = vec3;

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
  return out << v[0] << ' ' << v[1] << ' ' << v[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
  return vec3(_mm256_add_pd(u.e_simd, v.e_simd));
}

inline vec3 operator-(const vec3& u, const vec3& v) {
  return vec3(_mm256_sub_pd(u.e_simd, v.e_simd));
}

inline vec3 operator*(const vec3& u, const vec3& v) {
  return vec3(_mm256_mul_pd(u.e_simd, v.e_simd));
}

inline vec3 operator*(double t, const vec3& v) {
  return vec3(_mm256_mul_pd(v.e_simd, _mm256_set1_pd(t)));
}

inline vec3 operator*(const vec3& v, double t) {
  return t * v;
}

inline vec3 operator/(const vec3& v, double t) {
  return (1/t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
  __m256d mul = _mm256_mul_pd(u.e_simd, v.e_simd);
  __m128d low = _mm256_castpd256_pd128(mul);
  __m128d high = _mm256_extractf128_pd(mul, 1);
  __m128d sum_xy = _mm_hadd_pd(low, low);
  __m128d result = _mm_add_sd(sum_xy, high);
  return _mm_cvtsd_f64(result);
}

inline vec3 cross(const vec3& u, const vec3& v) {
  __m256d a1 = _mm256_permute4x64_pd(u.e_simd, 0b11001001); // 0xC9
  __m256d b1 = _mm256_permute4x64_pd(v.e_simd, 0b11010110); // 0xD6
  __m256d mul1 = _mm256_mul_pd(a1, b1);

  __m256d a2 = _mm256_permute4x64_pd(u.e_simd, 0b11010110); // 0xD6
  __m256d b2 = _mm256_permute4x64_pd(v.e_simd, 0b11001001); // 0xC9
  __m256d mul2 = _mm256_mul_pd(a2, b2);

  return vec3(_mm256_sub_pd(mul1, mul2));
}

inline vec3 unit_vector(const vec3& v) {
  return v / v.length();
}

inline vec3 random_unit_vector() {
  while (true) {
      auto p = vec3::random(-1, 1);
      auto lensq = p.length_squared();
      if (1e-160 < lensq && lensq <= 1)
          return p / std::sqrt(lensq);
  }
}

inline vec3 random_on_hemisphere(const vec3& normal) {
  vec3 onUnitSphere = random_unit_vector();
  if (dot(onUnitSphere, normal) > 0.0)
      return onUnitSphere;
  else 
      return -onUnitSphere;
}

inline vec3 reflect(const vec3& v, const vec3& n) {
  return v - 2 * dot(v, n) * n;
}

inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
  auto cos_theta = std::fmin(dot(-uv, n), 1.0);
  vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
  vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
  return r_out_perp + r_out_parallel;
}

inline vec3 random_in_unit_disk() {
  while (true) {
    auto p = vec3(drandom(-1, 1), drandom(-1, 1), 0.0);
    if (p.length_squared() < 1) return p;
  }
}

#endif