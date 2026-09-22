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
		return Device::getSuitableDevices(vulkanInstance.value(), windowSurface.value(), { vk::KHRSwapchainExtensionName} );
	}

	void Renderer::selectRenderDevice(Device::Config config)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot select device. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot select device. No surface found!");

		destroyRenderDevice();
		renderDevice.emplace(Device(config));
	}

	void Renderer::constructSwapchain(Swapchain::Config config)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot construct swapchain. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot construct swapchain. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot construct swapchain. No render device selected!");

		destroySwapchain();
		swapchain.emplace(Swapchain(renderDevice.value(), windowSurface.value(),config));
	}

	void Renderer::constructShaderEntries(Shader::Config config)
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
			spdlog::warn("A graphics pipeline with same configuration already exists. Skipping graphics pipeline creation"); 
		}

		graphicsPipelines.insert(std::pair(config, GraphicsPipeline(renderDevice.value(), *shaderEntries[config.vertexShaderEntry], *shaderEntries[config.fragmentShaderEntry], config)));
	}

	void Renderer::useGraphicsPipeline(GraphicsPipeline::Config config)
	{
		if (graphicsPipelines.find(config) == graphicsPipelines.end())
		{
			spdlog::warn("No graphics pipline with given configuration exists! Creating one on the fly.");
			constructGraphicsPipeline(config);
		}
		currentGraphicsPipelineConfig.emplace(std::move(config));
	}

	void Renderer::constructFramesInFlight(uint32_t frames_in_flight_count)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot construct frames in flight. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot construct frames in flight. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot construct frames in flight. No render device selected!");

		commandBuffers.clear(); 
		commandBufferReadyFences.clear();
		imagePaintableSemaphores.clear(); 
		currentFrame = 0;
		this->frames_in_flight_count = frames_in_flight_count;

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = NULL,
			.commandPool = (VkCommandPool)*renderDevice.value().getCommandPoolHandle(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = frames_in_flight_count
		};

		commandBuffers = vk::raii::CommandBuffers(renderDevice.value().getLogicalDeviceHandle(), vk::CommandBufferAllocateInfo(commandBufferAllocateInfo));

		VkFenceCreateInfo fenceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		VkSemaphoreCreateInfo semaphoreCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = NULL,
			.flags = {}
		};

		for (int i = 0;i < frames_in_flight_count; i++)
		{
			commandBufferReadyFences.push_back(vk::raii::Fence(renderDevice.value().getLogicalDeviceHandle(), fenceCreateInfo));
			imagePaintableSemaphores.push_back(vk::raii::Semaphore(renderDevice.value().getLogicalDeviceHandle(), semaphoreCreateInfo));
		}
	}

	void Renderer::drawFrame(glm::vec4 clearColor, vk::Viewport viewport, vk::Rect2D scissor)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot draw frame. No vulkan instance exists!"); 
		pyra_assert(windowSurface.has_value(), "Cannot draw frame. No window surface exists!");
		pyra_assert(renderDevice.has_value(), "Cannot draw frame. No rendering device selected!");
		pyra_assert(swapchain.has_value(), "Cannot draw frame. No swapchain exists!"); 
		pyra_assert(currentGraphicsPipelineConfig.has_value(), "Cannot draw frames. No graphics pipeline selected!");
		pyra_assert(frames_in_flight_count > 0, "Cannot draw frames. No frames in flight exists!");

		currentFrame %= frames_in_flight_count; 

		renderDevice->getLogicalDeviceHandle().waitForFences(*commandBufferReadyFences[currentFrame], true, UINT64_MAX);
		renderDevice->getLogicalDeviceHandle().resetFences(*commandBufferReadyFences[currentFrame]);

		auto [res, imageIdx] = swapchain->getSwapchainHandle().acquireNextImage(UINT64_MAX, *imagePaintableSemaphores[currentFrame]);

		commandBuffers[currentFrame].begin({});

		VkImageMemoryBarrier2 optimal_color_attachment_layout_transition_barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = NULL,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = {},
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = (VkImage)swapchain->getImages()[imageIdx],
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VkDependencyInfo optimal_color_attachment_dependency_info{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = NULL,
			.dependencyFlags = {},
			.memoryBarrierCount = 0,
			.pMemoryBarriers = NULL,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = NULL,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &optimal_color_attachment_layout_transition_barrier
		}; 

		commandBuffers[currentFrame].pipelineBarrier2(vk::DependencyInfo(optimal_color_attachment_dependency_info));

		vk::ClearColorValue clearColorVal(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		vk::ClearValue clearVal(clearColorVal);

		VkRenderingAttachmentInfo attachmentInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = NULL,
			.imageView = (VkImageView)*swapchain->getImageViews()[imageIdx],
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = (VkClearValue)clearVal,
		};

		VkRenderingInfo renderingInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.pNext = NULL,
			.flags = {},
			.renderArea = {.offset = {0,0}, .extent = swapchain->getConfig().imageExtent},
			.layerCount = 1,
			.viewMask = NULL,
			.colorAttachmentCount = 1,
			.pColorAttachments = &attachmentInfo
		};

		commandBuffers[currentFrame].beginRendering(renderingInfo);

		vk::Pipeline pipelineHandle = *graphicsPipelines.at(currentGraphicsPipelineConfig.value()).getGraphicsPipelineHandle();
		commandBuffers[currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineHandle);

		vk::Extent2D ext = swapchain->getConfig().imageExtent;
		if (viewport == vk::Viewport(0, 0, 0, 0, 0, 0))
		{
			viewport = vk::Viewport(0, 0, (float)ext.width, (float)ext.height, 0.f, 1.f);
		}

		if (scissor == vk::Rect2D({ 0,0 }, { 0,0 }))
		{
			scissor = vk::Rect2D({ 0,0 }, ext); 
		}

		commandBuffers[currentFrame].setViewport(0, viewport);
		commandBuffers[currentFrame].setScissor(0, scissor);

		commandBuffers[currentFrame].draw(3, 1, 0, 0);

		commandBuffers[currentFrame].endRendering();

		VkImageMemoryBarrier2 present_layout_transition_barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = NULL,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask =	VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			.dstAccessMask = {},
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = (VkImage)swapchain->getImages()[imageIdx],
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VkDependencyInfo present_dependency_info{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = NULL,
			.dependencyFlags = {},
			.memoryBarrierCount = 0,
			.pMemoryBarriers = NULL,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = NULL,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &present_layout_transition_barrier
		};

		commandBuffers[currentFrame].pipelineBarrier2(vk::DependencyInfo(present_dependency_info));

		commandBuffers[currentFrame].end(); 

		VkCommandBuffer cmdBuff = (VkCommandBuffer)*commandBuffers[currentFrame];
		VkSemaphore imagePaintableSemaphore = (VkSemaphore)*imagePaintableSemaphores[currentFrame];
		VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSemaphore imagePresentableSemaphore = (VkSemaphore)*swapchain->getImagePresentableSemaphores()[imageIdx];

		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &imagePaintableSemaphore,
			.pWaitDstStageMask = &waitDstStageMask,
			.commandBufferCount = 1,
			.pCommandBuffers = &cmdBuff,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &imagePresentableSemaphore
		};

		renderDevice->getGraphicsQueueHandle().submit(vk::SubmitInfo(submitInfo),*commandBufferReadyFences[currentFrame]);
		
		VkSwapchainKHR swapchain_ = (VkSwapchainKHR)*swapchain->getSwapchainHandle();

		VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &imagePresentableSemaphore,
			.swapchainCount = 1,
			.pSwapchains = &swapchain_,
			.pImageIndices = &imageIdx,
			.pResults = NULL
		};

		renderDevice->getGraphicsQueueHandle().presentKHR(vk::PresentInfoKHR(presentInfo));
		currentFrame++;
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
		swapchain.reset();
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
	const GraphicsPipeline::Config& pyra::Renderer::getCurrentGraphicsPipelineConfig() const
	{
		return currentGraphicsPipelineConfig.value();
	}
}