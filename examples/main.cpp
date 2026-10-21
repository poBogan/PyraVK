#define GLFW_INCLUDE_VULKAN
#include <fstream>
#include <GLFW/glfw3.h>

#include <PyraVK/VulkanInstance.h>
#include <PyraVK/VulkanSurface.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>
#include <PyraVK/Shader.h>
#include <PyraVK/GraphicsPipeline.h>

static std::vector<uint32_t> read_shader(const std::string& file_name)
{
	std::ifstream file(file_name, std::ios::binary | std::ios::ate);
	if (file.is_open() == false)
	{
		return  {};
	}
	const auto file_size = file.tellg();
	const auto vector_size = (file_size / sizeof(uint32_t));
	file.seekg(0, std::ios::beg);
	std::vector<uint32_t> shader_buffer(vector_size);

	file.read(reinterpret_cast<char*>(shader_buffer.data()), file_size);
	return shader_buffer;
}

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

	int width,height;
	glfwGetFramebufferSize(window, &width, &height);

	pyra::SwapchainConfig swapchainConfig{
		.c_swapchain_image_extent = vk::Extent2D(width, height),
		.c_swapchain_image_count = 3,
		.c_swapchain_image_color_format = vk::Format::eB8G8R8A8Unorm,
		.c_swapchain_image_color_space = vk::ColorSpaceKHR::eSrgbNonlinear,
		.c_swapchain_present_mode = vk::PresentModeKHR::eMailbox
	};
	pyra::Swapchain swapchain(swapchainConfig,device,surface);

	pyra::ShaderConfig shaderConfig {
		.c_shader_code = read_shader("pyra_shader.spirv"),
		.c_vertex_shader_entries = {"vsEntry"},
		.c_fragment_shader_entries = {"fsEntry"}
	};
	pyra::Shader shader(shaderConfig,device,surface);

	pyra::GraphicsPipelineConfig graphicsPipelineConfig {
		.vertexShaderEntry = "vsEntry",
		.fragmentShaderEntry = "fsEntry",
		.colorAttachmentFormat = swapchainConfig.c_swapchain_image_color_format,
		.primitiveTopology = vk::PrimitiveTopology::eTriangleList,
		.faceCullingMode = vk::CullModeFlagBits::eNone,
		.frontFace = vk::FrontFace::eClockwise,
	};
	pyra::GraphicsPipeline graphics_pipeline(graphicsPipelineConfig,shader,shader,device);

	while (glfwWindowShouldClose(window) == 0)
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

}
