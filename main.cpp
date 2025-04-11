/*
 * Ray Tracing in One Weekend  
 * 
 * This task should be easy because we just need to follow what the book says
 * but at the end, we should do our own way and probably do some optimization
 * code. And after this, I want to implement the ray tracing in C.
 * 
 * By Naufal Adriansyah
 */ 
#define GLAD_GL_IMPLEMENTATION
#include "sphere.h"
#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "external/glad/gl.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <cstdlib>

#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_glfw.h"
#include "external/imgui/backends/imgui_impl_opengl3.h"

/*
 * Multithreading for UI and Render
 */
#include <thread>
#include <mutex>
#include <atomic>

std::atomic<bool> shouldRender;
std::atomic<bool> shouldUpdateTexture = false;
std::atomic<bool> isRendering;
std::mutex renderMutex;

/*
 * We could use stb_image for writing the image but the I'll follow the book
 * for now, which is using PPM format. PPM stands for Portable Pixmap Format
 * and probably the simplest way to write an image with code. 
 */

class application {
  public:
    application() {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

      if (!glfwInit()) {
        throw new std::runtime_error("GLFW is required but you're device seem's to unsupport \
          the library. Go buy yourself a good device or install Linux bro.");
      }

      m_window = glfwCreateWindow(1024, 720, "My Favorite Renderer Gw", nullptr, nullptr);
      if (!m_window) {
        throw new std::runtime_error("Honestly, fuck yourself");
      }

      glfwMakeContextCurrent(m_window);
      glfwSwapInterval(1);

      int version = gladLoadGL(glfwGetProcAddress);
      if (version == 0) {
        throw new std::runtime_error("Disgusting piece of shit. Go to your nearest computer\
           store and buy one of the oldest laptop available because right now you are using ancient laptop");
      }

      std::cout << "Loaded OpenGL " << GLAD_VERSION_MAJOR(version) <<
        GLAD_VERSION_MINOR(version) << std::endl;
      
      /* Setup ImGUI */
      IMGUI_CHECKVERSION();
      ImGui::CreateContext();
      ImGuiIO& io = ImGui::GetIO(); (void) io;
      io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

      ImGui::StyleColorsDark();

      const char* glslVersion = "#version 150";
      ImGui_ImplGlfw_InitForOpenGL(m_window, true);
      ImGui_ImplOpenGL3_Init(glslVersion);

      /* Main Texture */
      glGenTextures(1, &m_renderTexture);
      glBindTexture(GL_TEXTURE_2D, m_renderTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_textureWidth, m_textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, m_renderBuffer.data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      m_camera = new camera("/home/naufal/Hello.ppm");
      m_camera->progress = &m_progressBar;

      m_world = new hittable_list();
    }

    ~application() {
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();

      if (m_window != nullptr) glfwDestroyWindow(m_window);
      glfwTerminate();
    }

    void run() {
      std::thread renderThread([&] () {
        m_camera->aspectRatio = 4.0f / 3.0f;
        m_camera->imageWidth = 800;
        m_camera->maxDepth = 50;

        m_renderBuffer.resize(m_textureWidth * m_textureHeight * 3);
        
        m_world->add(std::make_shared<sphere>(point3(0,-100.5,-1), 100, material_ground));

        while (true) {
          m_camera->samplePerPixel = m_samples;

          if (shouldRender) {
            std::lock_guard<std::mutex> lock(renderMutex);

            std::cout << "Size of spheres: " << m_spheres.size() << std::endl;
            for (const auto& sph : m_spheres) {
              vec3 pos = vec3(sph.position[0], sph.position[1], sph.position[2]);
              m_world->add(std::make_shared<sphere>(pos, sph.radius, material_center));
            }

            m_camera->render(*m_world, m_renderBuffer);
            shouldUpdateTexture = true;
          }

          shouldRender = false;

          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
      });

      while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();

        if (shouldUpdateTexture) {
          std::lock_guard<std::mutex> lock(renderMutex);

          glBindTexture(GL_TEXTURE_2D, m_renderTexture);
          glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_textureWidth, m_textureHeight,
            GL_RGB, GL_UNSIGNED_BYTE, m_renderBuffer.data());
        
          shouldUpdateTexture = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (m_showDemoWindow) {
          ImGui::ShowDemoWindow(&m_showDemoWindow);
        }

        if (ImGui::Begin("Renderer")) {
          ImGui::Text("Render Preview");
          ImGui::Image((ImTextureID)(intptr_t) m_renderTexture, ImVec2(m_textureWidth, m_textureHeight));
        }
        ImGui::End();

        static bool isCreateNewSphere = false;
        if (ImGui::Begin("Control Panel")) {
          ImVec2 windowSize = ImGui::GetWindowSize();

          if (isCreateNewSphere) {
            static float newSize[3] = { 0.0f, 0.0f, -1.0f }; 
            static char name[100] = "Ball";
            std::string id = "sphere-" + std::to_string(m_spheres.size() + 1);

            ImGui::Text("Create new sphere");
            ImGui::InputFloat3(id.c_str(), newSize);
            ImGui::InputText("Name", name, 100);
            
            if (ImGui::Button("Create", ImVec2(windowSize.x - 16, 32.0f))) {
              SphereData sphereData;
              sphereData.position = newSize;
              sphereData.radius = 0.5f;
              sphereData.name = name;
              m_spheres.push_back(sphereData);

              isCreateNewSphere = false;
            }
          } else {
            if (ImGui::Button("Add Sphere", ImVec2(windowSize.x - 16, 32.0f))) {
              isCreateNewSphere = true;  
            }
          }

          for (auto& sph : m_spheres) {
            ImGui::Text("%s", sph.name.c_str());
          }

          ImGui::Text("Render Settings");
          ImGui::InputInt("Samples", &m_samples);
          
          float progress = (float) m_progressBar / (m_textureHeight * m_textureWidth);
          ImGui::ProgressBar(progress, ImVec2(windowSize.x - 16, 32.0f));

          if (ImGui::Button("Render (or Re-render)")) {
            m_progressBar = 0;
            shouldRender = true;
          }
        };
        ImGui::End();

        ImGui::Render();
        int displayWidth, displayHeight;
        glfwGetFramebufferSize(m_window, &displayWidth, &displayHeight);
        glViewport(0, 0, displayWidth, displayHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
      }
    }

  private:
    GLFWwindow* m_window = nullptr;
    bool m_showDemoWindow = false;
    GLuint m_renderTexture = 0;
    int m_textureWidth = 800;
    int m_textureHeight = 600;
    std::vector<unsigned char> m_renderBuffer; // RGBA
    hittable_list* m_world;
    camera* m_camera;
    int m_progressBar = 0;
    int m_samples = 16;

    typedef struct {
      std::string name;
      float* position;
      double radius;
    } SphereData;

    std::vector<SphereData> m_spheres;

    std::shared_ptr<material> material_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
    std::shared_ptr<material> material_center = std::make_shared<lambertian>(color(0.1, 0.2, 0.5));
    std::shared_ptr<material> material_left   = std::make_shared<metal>(color(0.8, 0.8, 0.8), 0.3f);
    std::shared_ptr<material> material_right  = std::make_shared<metal>(color(0.8, 0.6, 0.2), 1.0f);
  };

int main(int argc, char const *argv[]) {
  try {
    application app;
    app.run();
  } catch (std::exception e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
