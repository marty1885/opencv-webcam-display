cmake_minimum_required(VERSION 3.5)
project(camera_app)

# This library wasn't intende to be used as a submodule. Some hack to get it goint
set(IMGUI_SFML_BUILD_EXAMPLES OFF)
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO OFF)
message(${CMAKE_CURRENT_SOURCE_DIR})
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)
add_subdirectory(third_party/imgui-sfml)


find_package(OpenCV REQUIRED)

add_executable(camera_app camera_app/main.cpp)
target_include_directories(camera_app PRIVATE third_party/imgui third_party/imgui-sfml ${OpenCV_INCLUDE_DIRS})
target_link_libraries(camera_app PRIVATE ImGui-SFML::ImGui-SFML imgui ${OpenCV_LIBS})