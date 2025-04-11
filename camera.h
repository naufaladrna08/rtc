#ifndef CAMERA_H
#define CAMERA_H
#include "hittable.h"
#include "material.h"

class camera {
  public: 
    double aspectRatio = 1.0f;
    int imageWidth = 0;
    int samplePerPixel = 1;
    int* progress;
    
    /*
     * Maximum number of ray bounces into scene
     */
    int maxDepth = 10; 

    camera(std::string fileName) {
      m_fileStream.open(fileName);
      if (!m_fileStream.is_open()) {
        std::cout << "File is not open!" << std::endl;
      } else {
        std::cout << "File opened: " << fileName << std::endl;
      }
    }

    ~camera() {
      m_fileStream.close();
    }

    void render(const hittable& world) {
      initialize();

      m_fileStream << "P3\n" << imageWidth << " " << m_imageHeight << "\n255\n";

      for (int j = 0; j < m_imageHeight; ++j) {
        std::clog << "\rScanlines remaining: " << m_imageHeight - j << " " << std::flush;
        
        for (int i = 0; i < imageWidth; i++) {
          color pixelColor(0.0f, 0.0f, 0.0f);
          for (int sample = 0; sample < samplePerPixel; sample++) {
            ray r = get_ray(i, j);
            pixelColor += ray_color(r, maxDepth, world);
          }

          write_color(m_fileStream, m_pixelSampleScale * pixelColor);
        }
      }
    
      std::clog << "\rDone\n";
    }

    void render(const hittable& world, std::vector<unsigned char>& outBuffer) {
      initialize();
      m_fileStream << "P3\n" << imageWidth << " " << m_imageHeight << "\n255\n";

      for (int j = 0; j < m_imageHeight; ++j) {
        std::clog << "\rScanlines remaining: " << m_imageHeight - j << " " << std::flush;
        
        for (int i = 0; i < imageWidth; i++) {
          color pixelColor(0.0f, 0.0f, 0.0f);
          for (int sample = 0; sample < samplePerPixel; sample++) {
            ray r = get_ray(i, j);
            pixelColor += ray_color(r, maxDepth, world);
          }

          pixelColor *= m_pixelSampleScale;
          static const interval intensity(0.000f, 0.999f);
          int rbyte = int(256 * intensity.clamp(linear_to_gamma(pixelColor.x())));
          int gbyte = int(256 * intensity.clamp(linear_to_gamma(pixelColor.y())));
          int bbyte = int(256 * intensity.clamp(linear_to_gamma(pixelColor.z())));

          int index = (j * imageWidth + i) * 3;
          outBuffer[index] = rbyte;
          outBuffer[index + 1] = gbyte;
          outBuffer[index + 2] = bbyte;
          write_color(m_fileStream, m_pixelSampleScale * pixelColor);
          
          *progress += 1;
        }
      }
    
      std::clog << "\rDone\n";
    }

  private:
    std::ofstream m_fileStream;
    int m_imageHeight = 2;
    point3 m_center;
    point3 m_pixel00Location;
    vec3 m_pixelDeltaU;
    vec3 m_pixelDeltaV;
    double m_pixelSampleScale;

    void initialize() {
      /*
       * Calculating the image height
       *
       * The book want us to use consistent aspect ratio and to make the width and 
       * height always follow the aspect ratio. However the book want the aspect
       * ratio to be 16 / 9. In this code, I will use old TV aspect ratio, which 
       * is square 4/3. 
       */
      m_imageHeight = int(imageWidth / aspectRatio); // 800 : 1.33 = 601.50375
      m_imageHeight = m_imageHeight < 1 ? 1 : m_imageHeight; // lower is one

      /*
       * Virtual Viewport 
       * 
       * We need to make our 3D scene accessible with a form of point3 and the
       * viewport will bound the rendered image.
       */
      double viewportHeight = 2.0f; // arbitary viewport height 
      double viewportWidth = viewportHeight * (double(imageWidth) / m_imageHeight);

      double focal_length = 1.0f; // The distance between eye and viewport
      m_center = point3(0.0f, 0.0f, 0.0f);

      // right and up direction
      auto viewportU = vec3(viewportWidth, 0.0f, 0.0f); 
      auto viewportV = vec3(0.0f, -viewportHeight, 0.0f);
    
      m_pixelDeltaU = viewportU / imageWidth;
      m_pixelDeltaV = viewportV / m_imageHeight;

      // center - direction to the viewport - half of viewport size
      auto viewport_upper_left = m_center - vec3(0, 0, focal_length) - viewportU / 2
        - viewportV / 2;
        m_pixel00Location = viewport_upper_left + 0.5f * (m_pixelDeltaU + m_pixelDeltaV);

      m_pixelSampleScale = 1.0f / samplePerPixel;
    }

    ray get_ray(int i, int j) const {
      auto offset = sampleSquare();
      auto pixelSample = m_pixel00Location
        + ((i + offset.x()) * m_pixelDeltaU)
        + ((j + offset.y()) * m_pixelDeltaV);

      auto rayOrigin = m_center;
      auto rayDirection = pixelSample - rayOrigin;
      
      return ray(rayOrigin, rayDirection);
    }

    vec3 sampleSquare() const {
      return vec3(drandom() - 0.5f, drandom() - 0.5f, 0.0f);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
      // If we've exceeded the ray bounce limit, no more light is gathered
      if (depth <= 0) {
        return color(0.0f, 0.0f, 0.0f);
      }
      
      hit_record rec;
      if (world.hit(r, interval(0.001, infinity), rec)) {
        ray scattered;
        color attenuation;

        if (rec.mat->scatter(r, rec, attenuation, scattered)) {
          return attenuation * ray_color(scattered, depth - 1, world);
        }

        return color(0.0f, 0.0f, 0.0f);
      }

      vec3 unit_direction = unit_vector(r.direction());
      auto a = 0.5 * (unit_direction.y() + 1.0);
      return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }
};

#endif