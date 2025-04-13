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
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#include <GLFW/glfw3native.h>
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

std::atomic<bool> shouldExit = false;
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
      glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

      if (!glfwInit()) {
        throw new std::runtime_error("GLFW is required but you're device seem's to unsupport \
          the library. Go buy yourself a good device or install Linux bro.");
      }

      m_window = glfwCreateWindow(1024, 720, "My Favorite Renderer Gw", nullptr, nullptr);
      if (!m_window) {
        throw new std::runtime_error("Honestly, fuck yourself");
      }

      glfwMaximizeWindow(m_window);
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
      io.Fonts->AddFontFromFileTTF("assets/NationalPark-SemiBold.ttf", 18.0f);

      ImGuiStyle &style = ImGui::GetStyle();
      ImVec4 *colors = style.Colors;
      ImGui::StyleColorsDark();

      // Primary background
      colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);  // #131318
      colors[ImGuiCol_MenuBarBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f); // #131318

      colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);

      // Headers
      colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
      colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
      colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.35f, 1.00f);

      // Buttons
      colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
      colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.32f, 0.40f, 1.00f);
      colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.38f, 0.50f, 1.00f);

      // Frame BG
      colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
      colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);
      colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

      // Tabs
      colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
      colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.50f, 1.00f);
      colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.38f, 1.00f);
      colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.17f, 1.00f);
      colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);

      // Title
      colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
      colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
      colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

      // Borders
      colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
      colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

      // Text
      colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
      colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);

      // Highlights
      colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
      colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
      colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.80f, 1.00f, 1.00f);
      colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.70f, 1.00f, 0.50f);
      colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.80f, 1.00f, 0.75f);
      colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.90f, 1.00f, 1.00f);

      // Scrollbar
      colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
      colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
      colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
      colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.55f, 1.00f);

      // Style tweaks
      style.WindowRounding = 5.0f;
      style.FrameRounding = 5.0f;
      style.GrabRounding = 5.0f;
      style.TabRounding = 5.0f;
      style.PopupRounding = 5.0f;
      style.ScrollbarRounding = 5.0f;
      style.WindowPadding = ImVec2(10, 10);
      style.FramePadding = ImVec2(6, 4);
      style.ItemSpacing = ImVec2(8, 6);
      style.PopupBorderSize = 0.f;

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
      shouldExit = true;
      if (m_renderThread.joinable()) {
        m_renderThread.join();
      }

      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();

      if (m_window != nullptr) glfwDestroyWindow(m_window);
      glfwTerminate();
    }

    void run() {
      m_renderThread = std::thread([&] () {
        m_camera->maxDepth = 50;
        m_camera->lookFrom = point3(-2,2,1);
        m_camera->lookAt   = point3(0,0,-1);
        m_camera->vup      = vec3(0,1,0);
        m_renderBuffer.resize(m_textureWidth * m_textureHeight * 3);
        
        m_world->add(std::make_shared<sphere>(point3(0.0f, -100.5f, -1), 100, material_ground));
        // m_world->add(std::make_shared<sphere>(point3(0.0f,  0.0f,   -1), 0.5, material_center));
        // m_world->add(std::make_shared<sphere>(point3(1.2f,  0.0f,   -1), 0.5, material_dielectric));
        // m_world->add(std::make_shared<sphere>(point3(1.2f,  0.0f,   -1), 0.4, material_bubble));
        // m_world->add(std::make_shared<sphere>(point3(-1.2f,  0.0f,  -1), 0.5, material_dielectric));

        auto R = std::cos(pi / 4);

        m_world->add(std::make_shared<sphere>(point3(-R, 0.0f, -1.0f), R, material_center));
        m_world->add(std::make_shared<sphere>(point3( R, 0.0f, -1.0f), 0.5, material_center));

        static int lastSize = m_textureWidth * m_textureHeight * 3;
        while (!shouldExit) {
          m_textureHeight = static_cast<int>(m_textureWidth / m_renderAspectRatio);
          int size = m_textureWidth * m_textureHeight * 3;
          if (size != lastSize) {
            m_renderBuffer.resize(size);
          }

          m_camera->aspectRatio = m_renderAspectRatio;
          m_camera->imageWidth = m_textureWidth;
          m_camera->samplePerPixel = m_samples;
          m_camera->defocusAngle = m_defocusAngle;
          m_camera->focusDist = m_focusDist;
          m_camera->vfov = m_vfov;

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

        /*
         * Dockspace is always ON in this app, so I'm not using is opt_fullscreen
         */
        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags dockspaceWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        dockspaceWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        dockspaceWindowFlags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        static bool dockspaceWindowOpen = true;
        ImGui::Begin("Main Dockspace", &dockspaceWindowOpen, dockspaceWindowFlags);
        ImGui::PopStyleVar(3);

        // Submit the dockspace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiID dockspaceId = ImGui::GetID("MyDockspace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), dockspaceFlags);

        if (ImGui::BeginMenuBar()) {
          if (ImGui::BeginMenu("App")) {
            if (ImGui::MenuItem("Exit")) {
              glfwSetWindowShouldClose(m_window, true);
            }

            ImGui::EndMenu();
          }

          ImGui::EndMenuBar();
        }

        // End dockspace window
        ImGui::End();

        if (ImGui::Begin("Renderer")) {
          ImVec2 windowSize = ImGui::GetWindowSize();
          int width = windowSize.x - 32.0f; 
          int height = static_cast<int>(width / m_renderAspectRatio);

          ImGui::Text("Render Preview");
          ImGui::Image((ImTextureID)(intptr_t) m_renderTexture, ImVec2(width, height));
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
          ImGui::InputInt("Image Width", &m_textureWidth);
          ImGui::InputInt("Samples", &m_samples);
          ImGui::InputInt("Vertical Field of View", &m_vfov);
          ImGui::InputDouble("Focus Distance", &m_focusDist);
          ImGui::InputDouble("Defocus Angle", &m_defocusAngle);
          
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
    double m_defocusAngle = 10.0f;
    double m_focusDist = 3.4f;
    double m_renderAspectRatio = 4.0f / 3.0f;
    std::thread m_renderThread;
    int m_vfov = 20;

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
    std::shared_ptr<material> material_dielectric = std::make_shared<dielectric>(1.50f);
    std::shared_ptr<material> material_bubble = std::make_shared<dielectric>(1.00f / 1.50f);
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
