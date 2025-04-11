#ifndef HITTABLE_H
#define HITTABLE_H
#include "utils.h"

class material;

class hit_record {
  public:
    point3 p;        // Point hit
    vec3 normal;     // Arah keluar 
    double t;        // Parameter ray untuk hit
    bool front_face; // Apakah terjadi hit pada sisi depan
    std::shared_ptr<material> mat; // Material dari object yang terkena hit

    void set_front_face(const ray& r, const vec3& outward_normal) {
      front_face = dot(r.direction(), outward_normal) < 0;
      normal = front_face ? outward_normal : -outward_normal;
    }
};

/*
 * Kelas abstrak hittable digunakan sebagagi interface untuk segala sesuatu yang
 * dihit oleh ray. Pada parameter ke-3, rec dibutuhkan untuk menentukan warna,
 * materual dan normal dari object yang terkena ray.
 */
class hittable {
  public:
    virtual ~hittable() = default;
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
};

#endif