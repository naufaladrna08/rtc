#ifndef SPHERE_H
#define SPHERE_H
#include "vec3.h"
#include "hittable.h"

class sphere : public hittable {
  public:
    sphere(const vec3& center, double radius, std::shared_ptr<material> mat) : m_center(center), m_radius(std::fmax(0, radius)), m_mat(mat) { }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
      vec3 oc = m_center - r.origin();
      double a = r.direction().length_squared();
      double h = dot(r.direction(), oc);
      double c = oc.length_squared() - m_radius * m_radius;
      auto discriminant = h * h - a * c;
      
      if (discriminant < 0)
        return false;

      auto sqrtd = std::sqrt(discriminant);
      auto root = (h - sqrtd) / a;
      if (!ray_t.surrounds(root)) {
        root = (h + sqrtd) / a;
        if (!ray_t.surrounds(root)) {
          return false;
        }
      }

      rec.t = root;
      rec.p = r.at(rec.t);
      rec.mat = m_mat;
      vec3 outward_normal = (rec.p - m_center) / m_radius;
      rec.set_front_face(r, outward_normal);

      return true;
    }

  private:
    vec3 m_center;
    double m_radius;
    std::shared_ptr<material> m_mat;
};

#endif