cmake_minimum_required(VERSION 3.16)
project(pyravk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

add_subdirectory(third-party/glfw)
add_executable(tutorial main.cpp VulkanApplication.hpp)
target_link_libraries(tutorial PUBLIC glfw PUBLIC Vulkan::Vulkan)

