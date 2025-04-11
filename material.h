#ifndef MATERIAL_H
#define MATERIAL_H
#include "hittable.h"

class material {
  public:
    virtual ~material() = default;

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const {
      return false;
    }
};

class lambertian : public material {
  public:
    lambertian(const color& albedo) : m_albedo(albedo) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
      auto scatter_direction = rec.normal + random_unit_vector();
      
      if (scatter_direction.near_zero()) {
        scatter_direction = rec.normal;
      }
      
      scattered = ray(rec.p, scatter_direction);
      attenuation = m_albedo;

      return true;
    }

  private:
    color m_albedo;
};


class metal : public material {
  public:
    metal(const color& albedo, double fuzz) : m_albedo(albedo), m_fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
      vec3 reflected = reflect(r_in.direction(), rec.normal);
      reflected = unit_vector(reflected) + (m_fuzz * random_unit_vector());
      scattered = ray(rec.p, reflected);
      attenuation = m_albedo;
      return (dot(scattered.direction(), rec.normal) > 0);
    }

  private:
    color m_albedo;
    double m_fuzz;
};

#endif