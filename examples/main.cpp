#include <PyraVK/Renderer.h>
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

	pyra::Renderer::createRenderer();
	auto& renderer = pyra::Renderer::getRenderer(); 

	renderer.constructVulkanInstance(extension_names);

	VkSurfaceKHR s; 
	glfwCreateWindowSurface(*renderer.getVulkanInstance().getVulkanInstanceHandle(), window, nullptr, &s);
	renderer.constructWindowSurface(s);

	auto devices = renderer.getSuitableRenderDevices();
	renderer.selectRenderDevice(std::move(devices[0]));

	int width, height; 
	glfwGetFramebufferSize(window, &width, &height);
	renderer.constructSwapchain({ (uint32_t)width, (uint32_t)height });

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	renderer.getRenderDevice().getLogicalDeviceHandle().waitIdle();

	glfwDestroyWindow(window); 
	glfwTerminate();
}