#include <PyraVK/Renderer.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <PyraVK/EngineContext.h>

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 800, "test program", nullptr, nullptr);

	uint32_t extCount = 0;
	const char** extNames = glfwGetRequiredInstanceExtensions(&extCount);
	const std::vector<const char*> extension_names(extNames, extNames + extCount);

	pyra::VulkanInstance instance(extension_names, {"VK_LAYER_KHRONOS_validation"});

	VkSurfaceKHR surface_ = VK_NULL_HANDLE;
	glfwCreateWindowSurface(static_cast<vk::Instance>(instance.get_instance()), window, nullptr, &surface_);
	pyra::VulkanSurface surface(surface_,instance);

	const pyra::DeviceConfig config = instance.get_suitable_devices(surface).front();
	pyra::Device device(config);

	pyra::EngineContext engineContext(std::move(instance), std::move(surface), std::move(device));

	int width,height;
	glfwGetFramebufferSize(window, &width, &height);

	pyra::SwapchainConfig swapchainConfig{
		.c_swapchain_image_extent = vk::Extent2D(width, height),
		.c_swapchain_image_count = 3,
		.c_swapchain_image_color_format = vk::Format::eB8G8R8A8Unorm,
		.c_swapchain_image_color_space = vk::ColorSpaceKHR::eSrgbNonlinear,
		.c_swapchain_present_mode = vk::PresentModeKHR::eMailbox
	};
	pyra::Swapchain swapchain(swapchainConfig,engineContext);
	pyra::Renderer renderer(engineContext,std::move(swapchain),3);

	glfwDestroyWindow(window);
	glfwTerminate();

}
