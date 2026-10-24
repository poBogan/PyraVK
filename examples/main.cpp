#define GLFW_INCLUDE_VULKAN
#include <fstream>
#include <iostream>
#include <GLFW/glfw3.h>

#include <PyraVK/VulkanInstance.h>
#include <PyraVK/VulkanSurface.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>
#include <PyraVK/Shader.h>
#include <PyraVK/GraphicsPipeline.h>
#include <PyraVK/Buffer.h>
#include "PyraVK/AssetPool.hpp"
#include "PyraVK/TaskManager.hpp"

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 800, "test program", nullptr, nullptr);

	uint32_t extCount = 0;
	const char** extNames = glfwGetRequiredInstanceExtensions(&extCount);
	const std::vector<const char*> extension_names(extNames, extNames + extCount);

	auto func = [](int a,int b) -> void  {std::cout << a << " " << b << std::endl;};
	pyra::TaskManager taskManager;
	taskManager.PushTask(func,5,2);
	taskManager.PushTask(func,3,2);
	taskManager.PushTask(func,5,1);
	taskManager.PushTask(func,10,2);


	while (glfwWindowShouldClose(window) == 0)
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

}
