#include <PyraVK/Renderer.h>


namespace pyra
{

	std::optional<Renderer> Renderer::singletonRenderer;

	void pyra::Renderer::createRenderer()
	{
		if (!singletonRenderer.has_value())
			singletonRenderer.emplace(Renderer());
	}

	Renderer& pyra::Renderer::getRenderer()
	{
		pyra_assert(singletonRenderer.has_value(), "Cannot get renderer. It is not initilaized!");
		return singletonRenderer.value();
	}

	void Renderer::constructVulkanInstance(std::vector<const char*> extentions,std::vector<const char*> layers)
	{
		destroyVulkanInstance();
		vulkanInstance.emplace(VulkanInstance(context, std::move(extentions), std::move(layers)));
	}

	void Renderer::constructWindowSurface(VkSurfaceKHR surface)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot init surface. No vulkan instance found");

		destroyWindowSurface();
		windowSurface.emplace(Surface(vulkanInstance.value(),surface));
	}

	std::vector<Device> Renderer::getSuitableRenderDevices()
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot fetch devices. No vulkan instance found!"); 
		pyra_assert(windowSurface.has_value(), "Cannot fetch devices. No surface found");
		return Device::getSuitableDevices(vulkanInstance.value(), windowSurface.value(), { vk::KHRSwapchainExtensionName, vk::KHRSynchronization2ExtensionName});
	}

	void Renderer::selectRenderDevice(Device&& render_device)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot select device. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot select device. No surface found!");

		destroyRenderDevice();
		render_device.constructDeviceHandles();
		renderDevice.emplace(std::move(render_device));
	}

	void Renderer::constructSwapchain(
		vk::Extent2D window_size_in_pixels,
		uint32_t targetImageCount,
		std::vector<vk::SurfaceFormatKHR> imageFormatPreferences,
		std::vector<vk::PresentModeKHR> presentModePreferences
		)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot construct swapchain. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot construct swapchain. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot construct swapchain. No render device selected!");

		destroySwapchain();
		swapchain.emplace(Swapchain(renderDevice.value(), windowSurface.value(), window_size_in_pixels, targetImageCount, imageFormatPreferences, presentModePreferences));

		// NOTE : Recreate graphics pipelines if their target format do not match the format of the new swapchain images : 
	}

	void Renderer::addShaderEntries(std::string file, std::vector<std::string>vertexShaderEntries, std::vector<std::string>fragmentShaderEntries)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot construct shaders. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot construct shaders. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot construct shaders. No render device selected!");

		std::shared_ptr<Shader> newShader(new Shader(renderDevice.value(), file, vertexShaderEntries, fragmentShaderEntries));

		for (std::string& vsEntry : vertexShaderEntries)
		{
			pyra_warn(shaderEntries.find(vsEntry) != shaderEntries.end(), "A vertex shader entry named \"{:}\" already exists! New one will overwrite it",vsEntry.c_str());
			shaderEntries.insert(std::pair<std::string, std::shared_ptr<Shader>>(vsEntry, newShader));
		}

		for (std::string& fsEntry : fragmentShaderEntries)
		{
			pyra_warn(shaderEntries.find(fsEntry) != shaderEntries.end(), "A fragment shader entry named \"{:}\" already exists! New one will overwrite it", fsEntry.c_str());
			shaderEntries.insert(std::pair<std::string, std::shared_ptr<Shader>>(fsEntry, newShader));
		}
	}

	void Renderer::addGraphicsPipeline(
		std::string pipeline_name,
		std::string vertexShaderEntry,
		std::string fragmentShaderEntry,
		vk::PrimitiveTopology primitiveTopology,
		vk::CullModeFlagBits faceCullingMode,
		vk::FrontFace frontFace,
		vk::SampleCountFlagBits sampleCount
	) {
		pyra_assert(vulkanInstance.has_value(), "Cannot construct graphics pipeline. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot construct graphics pipeline. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot construct graphics pipeline. No render device selected!");
		pyra_assert(swapchain.has_value(), "Cannot construct graphics pipeline. No swapchain found!");

		pyra_assert(shaderEntries.find(vertexShaderEntry) != shaderEntries.end(), "Cannot create graphics pipeline. No such vertex shader entry as \"{:}\" found!",vertexShaderEntry.c_str());
		pyra_assert(shaderEntries.find(fragmentShaderEntry) != shaderEntries.end(), "Cannot create graphics pipeline. No such fragment shader entry as \"{:}\" found!",fragmentShaderEntry.c_str());

		pyra_warn(graphicsPipelines.find(pipeline_name) != graphicsPipelines.end(),"A graphics pipeline named \"{:}\" already exists. New one will overwrite it",pipeline_name.c_str());
		
		std::pair<std::string, GraphicsPipeline> newPair(pipeline_name, GraphicsPipeline(renderDevice.value(),
			*shaderEntries[vertexShaderEntry], vertexShaderEntry,
			*shaderEntries[fragmentShaderEntry], fragmentShaderEntry,
			swapchain->getSwapchainImageFormat().format, primitiveTopology, faceCullingMode, frontFace, sampleCount));

		graphicsPipelines.insert(std::move(newPair));
	}

	void Renderer::destroyVulkanInstance()
	{
		destroyWindowSurface(); 
		vulkanInstance.reset();
	}

	void Renderer::destroyWindowSurface()
	{
		destroyRenderDevice(); 
		windowSurface.reset();
	}

	void Renderer::destroyRenderDevice()
	{
		graphicsPipelines.clear(); 
		shaderEntries.clear(); 
		swapchain.reset();

		renderDevice.reset();
	}

	void Renderer::destroySwapchain()
	{
		swapchain.reset();
	}

	void Renderer::destroyGraphicsPipelines()
	{
		graphicsPipelines.clear();
	}

	void Renderer::deleteGraphicsPipeline(std::string name)
	{
		graphicsPipelines.erase(name);
	}

	const vk::raii::Context& Renderer::getContext() const
	{
		return context;
	}

	const std::optional<VulkanInstance>& Renderer::getVulkanInstance() const
	{
		return vulkanInstance;
	}

	const std::optional<Surface>& Renderer::getWindowSurface() const
	{
		return windowSurface;
	}

	const std::optional<Device>& Renderer::getRenderDevice() const
	{
		return renderDevice;
	}

	const std::optional<Swapchain>& Renderer::getSwapchain() const
	{
		return swapchain;
	}

	const std::unordered_map<std::string, std::shared_ptr<Shader>>& Renderer::getShaderEntries() const
	{
		return shaderEntries;
	}

	const std::unordered_map<std::string, GraphicsPipeline>& Renderer::getGraphicsPipelines() const
	{
		return graphicsPipelines;
	}
}