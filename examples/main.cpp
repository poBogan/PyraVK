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
	swapchainInfo = renderer.constructSwapchain(swapchainInfo);

	pyra::Shader::Config shaderInfo{
		.file = "pyra_shader.spirv",
		.vertexShaderEntries = {"vsEntry"},
		.fragmentShaderEntries = {"fsEntry"},
	};
	renderer.constructShaders(shaderInfo);

	pyra::GraphicsPipeline::Config pipelineInfo{
		.vertexShaderEntry = "vsEntry",
		.fragmentShaderEntry = "fsEntry",
		.colorAttachmentFormat = swapchainInfo.imageFormat,
		.primitiveTopology = vk::PrimitiveTopology::eTriangleList,
		.faceCullingMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,
	};
	renderer.constructGraphicsPipeline(pipelineInfo);

	pyra::Frames::Config framesConfig{ .concurrentFramesCount = 3 }; 
	renderer.constructFrames(framesConfig);

	std::vector<pyra::Vertex> verticies{
		{{0.f,1.f}, {1.f,1.f,0.f,1.f}},
		{{1.f,0.f}, {1.f,0.f,0.f,1.f}},
		{{1.f,1.f}, {1.f,0.f,0.f,1.f}},
	}; 

	auto vertexBuffer = std::make_shared<pyra::VertexBuffer>(renderer.getRenderDevice().value(), verticies);


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		renderer.Draw({ vertexBuffer,pipelineInfo },{{0,0}, {width,height}}, {{0,0}, {width,height}});
	}

	renderer.getRenderDevice()->getLogicalDeviceHandle().waitIdle();

	glfwDestroyWindow(window); 
	glfwTerminate();
}