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
		pyra_assert(vulkanInstance.has_value(), "Cannot construct frames. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot construct frames. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot construct frames. No render device selected!");

		framesInFlight.emplace(FramesInFlight(renderDevice.value(), framesInFlightCount));
	}

	void transition_image_layout(
		const vk::raii::CommandBuffer& commandBuffer,
		VkPipelineStageFlags2 srcStageMask,
		VkPipelineStageFlags2 dstStageMask,
		VkAccessFlags2	srcAccessMask,
		VkAccessFlags2	dstAccessMask,
		VkImageLayout	oldImageLayout,
		VkImageLayout	newImageLayout,
		uint32_t		srcQueueFamilyIndex,
		uint32_t		dstQueueFamilyIndex,
		VkImage			image
	)
	{
		VkImageMemoryBarrier2 barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = NULL,
			.srcStageMask = srcStageMask,
			.srcAccessMask = srcAccessMask,
			.dstStageMask = dstStageMask,
			.dstAccessMask = dstAccessMask,
			.oldLayout = oldImageLayout,
			.newLayout = newImageLayout,
			.srcQueueFamilyIndex = srcQueueFamilyIndex,
			.dstQueueFamilyIndex = dstQueueFamilyIndex,
			.image = image,
			.subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
		}; 		
		VkDependencyInfo dependencyInfo{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = NULL,
			.dependencyFlags = NULL,
			.memoryBarrierCount = 0,
			.pMemoryBarriers = NULL,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = NULL,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier
		}; 
		commandBuffer.pipelineBarrier2(dependencyInfo);
	}

	void Renderer::Draw(RenderTarget renderTarget, Viewport viewport, Scissor scissor, glm::vec4 clearColor)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot draw frame. No vulkan instance found!"); 
		pyra_assert(windowSurface.has_value(), "Cannot draw frame. No window surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot draw frame. No render device selected!");
		pyra_assert(swapchain.has_value(), "Cannot draw frame. No swapchain found!");
		pyra_assert(framesInFlight.has_value(), "Cannot draw frame. No frames are not constructed!");

		renderDevice->getLogicalDeviceHandle().waitForFences(*framesInFlight->getCurrentFrameCommandBufferFence(), VK_TRUE, UINT64_MAX);
		renderDevice->getLogicalDeviceHandle().resetFences(*framesInFlight->getCurrentFrameCommandBufferFence());

		auto [result, imageIdx] = swapchain->getSwapchainHandle().acquireNextImage(UINT64_MAX, *framesInFlight->getCurrentFrameCommandBufferSemaphore());

		const auto& commandBuffer = framesInFlight->getCurrentFrameCommandBuffer();

		commandBuffer.begin({});

		transition_image_layout(
			commandBuffer,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			{},
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			(VkImage)swapchain->getImages()[imageIdx]
			);

		VkClearColorValue clearColorValue; 
		clearColorValue.float32[0] = clearColor.r;
		clearColorValue.float32[1] = clearColor.g;
		clearColorValue.float32[2] = clearColor.b;
		clearColorValue.float32[3] = clearColor.a;

		VkRenderingAttachmentInfo colorAttachmentInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = NULL,
			.imageView = (VkImageView)*swapchain->getImageViews()[imageIdx],
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = clearColorValue,
		};

		VkRenderingInfo renderingInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.pNext = NULL,
			.flags = {},
			.renderArea = {.offset = {0,0},.extent = swapchain->getConfig().imageExtent},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo
		};

		commandBuffer.beginRendering(renderingInfo); 
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipelines.at(renderTarget.graphicsPipeline).getGraphicsPipelineHandle());
		commandBuffer.bindVertexBuffers(0, *renderTarget.vertexBuffer->getVertexBufferHandle(), { 0 });
		commandBuffer.setViewport(0, vk::Viewport(viewport.offset.x, viewport.offset.y, viewport.extent.x, viewport.extent.y, 0.f, 1.f));
		commandBuffer.setScissor(0, vk::Rect2D(VkRect2D{ (int32_t)viewport.offset.x, (int32_t)viewport.offset.y, (uint32_t)viewport.extent.x, (uint32_t)viewport.extent.y }));
		commandBuffer.draw(renderTarget.vertexBuffer->getVertexCount(), 1, 0, 0);
		commandBuffer.endRendering();

		transition_image_layout(
			commandBuffer,
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			{},
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			(VkImage)swapchain->getImages()[imageIdx]
		);

		commandBuffer.end();

		VkSemaphore wS = (VkSemaphore)*framesInFlight->getCurrentFrameCommandBufferSemaphore();
		VkPipelineStageFlags waitDestination = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkCommandBuffer cB = (VkCommandBuffer)*framesInFlight->getCurrentFrameCommandBuffer();
		VkSemaphore sS = (VkSemaphore)*swapchain->getImagePresentableSemaphores()[imageIdx];

		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &wS,
			.pWaitDstStageMask = &waitDestination,
			.commandBufferCount = 1,
			.pCommandBuffers = &cB,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &sS
		};

		renderDevice->getGraphicsQueueHandle().submit(vk::SubmitInfo(submitInfo), *framesInFlight->getCurrentFrameCommandBufferFence());

		VkSwapchainKHR sC = (VkSwapchainKHR)*swapchain->getSwapchainHandle();

		VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &sS,
			.swapchainCount = 1,
			.pSwapchains = &sC,
			.pImageIndices = &imageIdx,
			.pResults = NULL
		};

		renderDevice->getGraphicsQueueHandle().presentKHR(vk::PresentInfoKHR(presentInfo));
		framesInFlight->nextFrame();
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
		framesInFlight.reset();
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