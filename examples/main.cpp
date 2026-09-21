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

	pyra::VulkanInstance::Config vulkanInstanceInfo{
		.extentions = extension_names,
		.layers = {"VK_LAYER_KHRONOS_validation"}
	};
	renderer.constructVulkanInstance(vulkanInstanceInfo);

	pyra::Surface::Config windowSurfaceInfo;
	glfwCreateWindowSurface(*renderer.getVulkanInstance().value().getVulkanInstanceHandle(), window, nullptr, &windowSurfaceInfo.surface);
	renderer.constructWindowSurface(windowSurfaceInfo);

	std::vector<pyra::Device::Config> deviceConfigs = renderer.getSuitableRenderDevices();
	renderer.selectRenderDevice(deviceConfigs.front());

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	pyra::Swapchain::Config swapchainInfo{
		.imageExtent = {(uint32_t)width, (uint32_t)height},
		.imageCount = 3,
		.imageFormat = vk::Format::eB8G8R8A8Unorm,
		.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
		.presentMode = vk::PresentModeKHR::eMailbox
	};
	renderer.constructSwapchain(swapchainInfo);

	pyra::Shader::Config shaderInfo{
		.file = "pyra_shader.spirv",
		.vertexShaderEntries = {"VertexShaderMain"},
		.fragmentShaderEntries = {"FragmentShaderMain"},
	};
	renderer.constructShaderEntries(shaderInfo);

	pyra::GraphicsPipeline::Config pipelineInfo{
		.vertexShaderEntry = "VertexShaderMain",
		.fragmentShaderEntry = "FragmentShaderMain",
		.targetFormat = vk::Format::eB8G8R8A8Unorm,
		.primitiveTopology = vk::PrimitiveTopology::eTriangleList,
		.polygonMode = vk::PolygonMode::eFill,
		.faceCullingMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,
		.sampleCount = vk::SampleCountFlagBits::e1
	};
	renderer.constructGraphicsPipeline(pipelineInfo);
	renderer.useGraphicsPipeline(pipelineInfo);

	renderer.constructFramesInFlight(2);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		renderer.drawFrame();
	}

	renderer.getRenderDevice()->getLogicalDeviceHandle().waitIdle();

	glfwDestroyWindow(window); 
	glfwTerminate();
}