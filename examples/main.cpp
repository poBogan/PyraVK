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
	glfwCreateWindowSurface(*renderer.getVulkanInstance().getInstance(), window, nullptr, &windowSurfaceInfo.surface);
	renderer.constructWindowSurface(windowSurfaceInfo);

	std::vector<pyra::Device::Config> deviceConfigs = renderer.getSuitableDevices();
	renderer.selectDevice(deviceConfigs.front());

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
	renderer.constructConcurrentFrames(swapchainInfo.imageCount - 1);

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
	pyra::ResourceID pipelineID = renderer.constructGraphicsPipeline(pipelineInfo);
	
	std::vector<pyra::Vertex> verticies{
		{{0.f,1.f}, {1.f,1.f,0.f,1.f}},
		{{1.f,0.f}, {1.f,0.f,0.f,1.f}},
		{{1.f,1.f}, {1.f,0.f,0.f,1.f}},
	};

	pyra::ResourceID bufferID = renderer.constructVertexBuffer(verticies);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		renderer.Draw({pipelineID, bufferID}, {0.f, 1.f, 0.f, 1.f});
	}

	glfwDestroyWindow(window); 
	glfwTerminate();
}