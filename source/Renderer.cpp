#include <PyraVK/Renderer.h>


namespace pyra
{

	std::optional<Renderer> Renderer::singletonRenderer;

	void pyra::Renderer::createRenderer()
	{
		if (!singletonRenderer.has_value())
		{
			spdlog::set_pattern("[PyraVK/%^%l%$] %v");
			singletonRenderer.emplace(Renderer());
		}
	}

	Renderer& pyra::Renderer::getRenderer()
	{
		pyra_assert(singletonRenderer.has_value(), "Cannot get renderer. It is not initilaized!");
		return singletonRenderer.value();
	}

	void Renderer::constructVulkanInstance(VulkanInstance::Config config)
	{
		destroyVulkanInstance();
		vulkanInstance.emplace(VulkanInstance(config));
	}

	void Renderer::constructWindowSurface(Surface::Config config)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot init surface. No vulkan instance found");

		destroyWindowSurface();
		windowSurface.emplace(Surface(vulkanInstance.value(),config));
	}

	std::vector<Device::Config> Renderer::getSuitableRenderDevices()
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot fetch devices. No vulkan instance found!"); 
		pyra_assert(windowSurface.has_value(), "Cannot fetch devices. No surface found");
		return Device::getSuitableDevices(vulkanInstance.value(), windowSurface.value() );
	}

	void Renderer::selectRenderDevice(Device::Config config)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot select device. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot select device. No surface found!");

		destroyRenderDevice();
		renderDevice.emplace(Device(config));
	}

	Swapchain::Config Renderer::constructSwapchain(Swapchain::Config requestedConfig)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot construct swapchain. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot construct swapchain. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot construct swapchain. No render device selected!");

		destroySwapchain();
		swapchain.emplace(Swapchain(renderDevice.value(), windowSurface.value(), requestedConfig));

		for (const auto& gp : graphicsPipelines)
		{
			if (gp.first.colorAttachmentFormat != swapchain->getConfig().imageFormat)
			{
				GraphicsPipeline::Config newPipelineConfig = gp.first; 
				newPipelineConfig.colorAttachmentFormat = swapchain->getConfig().imageFormat;
				constructGraphicsPipeline(newPipelineConfig);
			}
		}

		return swapchain->getConfig();
	}

	void Renderer::constructShaders(Shader::Config config)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot construct shaders. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot construct shaders. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot construct shaders. No render device selected!");

		std::shared_ptr<Shader> shaderModule(new Shader(renderDevice.value(), config));
		for (std::string& vertexShaderEntry : config.vertexShaderEntries)
		{
			pyra_warn(shaderEntries.find(vertexShaderEntry) != shaderEntries.end(), "A vertex shader entry named \"{:}\" already exists. Overwriting it",vertexShaderEntry.c_str());
			shaderEntries.insert(std::pair(vertexShaderEntry, shaderModule));
		}

		for (std::string& fragmentShaderEntry : config.fragmentShaderEntries)
		{
			pyra_warn(shaderEntries.find(fragmentShaderEntry) != shaderEntries.end(), "A fragment shader entry named \"{:}\" already exists. Overwriting it", fragmentShaderEntry.c_str());
			shaderEntries.insert(std::pair(fragmentShaderEntry, shaderModule));
		}
	}

	void Renderer::constructGraphicsPipeline(GraphicsPipeline::Config config)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot construct new graphics pipeline. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot construct new graphics pipeline. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot construct new graphics pipeline. No render device selected!");

		pyra_assert(shaderEntries.find(config.vertexShaderEntry) != shaderEntries.end(), "No vertex shader entry named as \"{:}\" exists!", config.vertexShaderEntry.c_str());
		pyra_assert(shaderEntries.find(config.fragmentShaderEntry) != shaderEntries.end(), "No fragment shader entry named as \"{:}\" exists!", config.fragmentShaderEntry.c_str());

		if (graphicsPipelines.find(config) != graphicsPipelines.end())
		{
			spdlog::warn("A graphics pipeline with identical configuration already exists. Skipping graphics pipeline creation"); 
			return;
		}

		else
		{
			graphicsPipelines.insert(std::pair(config, GraphicsPipeline(renderDevice.value(), *shaderEntries[config.vertexShaderEntry], *shaderEntries[config.fragmentShaderEntry], config)));
		}

		if (swapchain.has_value())
		{
			if (swapchain->getConfig().imageFormat != config.colorAttachmentFormat)
			{
				config.colorAttachmentFormat = swapchain->getConfig().imageFormat;
				constructGraphicsPipeline(config);
			}
		}
	}

	void Renderer::constructFramesInFlight(uint32_t framesInFlightCount)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot construct frames in flight. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot construct frames in flight. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot construct frames in flight. No render device selected!");

		commandBuffers.clear(); 
		commandBufferFences.clear();
		commandBufferSemaphores.clear();

		currentFrame = 0;
		framesInFlight = framesInFlightCount;

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = NULL,
			.commandPool = (VkCommandPool)*renderDevice->getCommandPoolHandle(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = framesInFlight
		};

		commandBuffers = vk::raii::CommandBuffers(renderDevice->getLogicalDeviceHandle(), commandBufferAllocateInfo); 

		VkFenceCreateInfo fenceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		VkSemaphoreCreateInfo semaphoreCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL
		};

		for (int i = 0;i < framesInFlight;i++)
		{
			commandBufferFences.push_back(vk::raii::Fence(renderDevice->getLogicalDeviceHandle(), fenceCreateInfo)); 
			commandBufferSemaphores.push_back(vk::raii::Semaphore(renderDevice->getLogicalDeviceHandle(), semaphoreCreateInfo));
		}

		commandBuffers.shrink_to_fit(); 
		commandBufferFences.shrink_to_fit(); 
		commandBufferSemaphores.shrink_to_fit();
	}

	void Renderer::Draw()
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot draw frame. No vulkan instance found!"); 
		pyra_assert(windowSurface.has_value(), "Cannot draw frame. No window surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot draw frame. No render device selected!");
		pyra_assert(swapchain.has_value(), "Cannot draw frame. No swapchain found!");


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
		shaderEntries.clear(); 
		graphicsPipelines.clear();
		swapchain.reset();

		commandBuffers.clear(); 
		commandBufferFences.clear(); 
		commandBufferSemaphores.clear(); 
		currentFrame = 0; 
		framesInFlight = 0;

		renderDevice.reset();
	}

	void Renderer::destroySwapchain()
	{
		swapchain.reset();
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

	const std::unordered_map<std::string, std::shared_ptr<Shader>>& pyra::Renderer::getShaderEntries() const
	{
		return shaderEntries;
	}
	const std::unordered_map<GraphicsPipeline::Config, GraphicsPipeline, GraphicsPipeline::Hasher>& pyra::Renderer::getGraphicsPipelines() const
	{
		return graphicsPipelines;
	}
}