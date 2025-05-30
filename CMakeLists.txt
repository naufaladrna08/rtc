cmake_minimum_required(VERSION 3.10.0)
project(rtc VERSION 0.1.0 LANGUAGES CXX)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/external/imgui
  ${CMAKE_SOURCE_DIR}/external/imgui/backends
  ${CMAKE_SOURCE_DIR}/external/imgui/misc/cpp
)

# Add cxx flag mavx
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")

add_executable(rtc 
  main.cpp
  external/imgui/imgui.cpp
  external/imgui/imgui_demo.cpp
  external/imgui/imgui_draw.cpp
  external/imgui/imgui_widgets.cpp
  external/imgui/imgui_tables.cpp
  external/imgui/misc/cpp/imgui_stdlib.cpp
  external/imgui/backends/imgui_impl_glfw.cpp
  external/imgui/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(rtc -lm -lglfw)