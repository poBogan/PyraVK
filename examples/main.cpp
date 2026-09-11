#include <PyraVK/Renderer.h>
#include <PyraVK/RenderDevice.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 800, "test program", 0, 0);

	uint32_t extCount = 0;
	const char** extNames = glfwGetRequiredInstanceExtensions(&extCount);
	std::vector<const char*> extension_names(extNames, extNames + extCount);
	std::vector<const char*> layer_names{ "VK_LAYER_KHRONOS_validation"};

	auto instance = pyra::VulkanInstance::createVulkanInstance(extension_names, layer_names);

	VkSurfaceKHR surface_; 
	glfwCreateWindowSurface(*instance->getInstanceObject(), window, nullptr, &surface_);
	auto surface = pyra::Surface::createSurface(instance, surface_);


	pyra::Renderer::createRenderer(std::move(instance), std::move(surface)); 

	auto devices = pyra::Renderer::getRenderer()->getSuitableDevices();

	pyra::Renderer::getRenderer()->useRenderDevice(std::move(devices[0]));

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window); 
	glfwTerminate();
}