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
		.extensions = extension_names,
		.layers = {"VK_LAYER_KHRONOS_validation"}
	};
	renderer.constructVulkanInstance(vulkanInstanceInfo);

	VkSurfaceKHR surface;
	glfwCreateWindowSurface(*renderer.getVulkanInstance().getInstance(), window, nullptr,&surface);
	renderer.constructWindowSurface(surface);

	const std::vector<pyra::Device::Config> deviceConfigs = renderer.getSuitableDevices();
	renderer.selectDevice(deviceConfigs.front());

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	pyra::Swapchain::Config swapchainInfo{
		.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)},
		.imageCount = 3,
		.imageFormat = vk::Format::eB8G8R8A8Unorm,
		.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
		.presentMode = vk::PresentModeKHR::eMailbox
	};
	swapchainInfo = renderer.constructSwapchain(swapchainInfo);
	renderer.constructConcurrentFrames(swapchainInfo.imageCount - 1);

	const pyra::Shader::Config shaderInfo{
		.file = "pyra_shader.spirv",
		.vertexShaderEntries = {"vsEntry"},
		.fragmentShaderEntries = {"fsEntry"}
	};
	renderer.constructShaders(shaderInfo);

	const pyra::GraphicsPipeline::Config pipelineInfo{
		.vertexShaderEntry = "vsEntry",
		.fragmentShaderEntry = "fsEntry",
		.colorAttachmentFormat = swapchainInfo.imageFormat,
		.primitiveTopology = vk::PrimitiveTopology::eTriangleList,
		.faceCullingMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,
	};
	pyra::ResourceID pipelineID = renderer.constructGraphicsPipeline(pipelineInfo);
	
	std::vector<pyra::Vertex> vertices{
		{.position = {0.f,1.f}, .color = {1.f,1.f,0.f,1.f}},
		{.position = {1.f,0.f}, .color = {1.f,0.f,0.f,1.f}},
		{.position = {1.f,1.f}, .color = {1.f,0.f,0.f,1.f}},
	};
	pyra::ResourceID bufferID = renderer.constructVertexBuffer(vertices);

	vk::Viewport viewport(0.f,0.f,static_cast<float>(width),static_cast<float>(height),0.f,1.f);
	vk::Rect2D scissor(vk::Offset2D(0,0), vk::Extent2D(static_cast<uint32_t>(width),static_cast<uint32_t>(height)));
	pyra::RenderTarget target {
		.graphics_pipeline = pipelineID,
		.vertex_buffer = bufferID,
		.viewport = viewport,
		.scissor = scissor
	};
	glm::vec4 clearColor {0.f,0.f,1.f,1.f};

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		renderer.Draw(target,clearColor);
	}

	glfwDestroyWindow(window); 
	glfwTerminate();
}