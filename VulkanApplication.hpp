#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


class VulkanApplication
{
    const int width = 800;
    const int height = 600;
    GLFWwindow* window = nullptr;

public:
    void run();
private :
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
};

inline void VulkanApplication::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

inline void VulkanApplication::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(width, height, "Vulkan Instance", nullptr, nullptr);
}

inline void VulkanApplication::initVulkan()
{
}

inline void VulkanApplication::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

inline void VulkanApplication::cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}
