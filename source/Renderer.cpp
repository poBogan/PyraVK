#include <PyraVK/Renderer.h>


namespace pyra
{

	std::optional<Renderer> Renderer::singletonRenderer;

	Renderer::~Renderer()
	{
		destroyVulkanInstance();
	}

	void pyra::Renderer::createRenderer()
	{
		// Singleton Design, create only one renderer instance
		if (!rendererExists())
		{
			spdlog::set_pattern("[PyraVK/%^%l%$] %v"); 
			singletonRenderer.emplace(Renderer());
		}
	}

	bool Renderer::rendererExists()
	{
		return singletonRenderer.has_value();
	}

	Renderer& pyra::Renderer::getRenderer()
	{
		pyra_assert(rendererExists());
		return singletonRenderer.value();
	}

	void Renderer::constructVulkanInstance(VulkanInstance::Config config)
	{
		destroyVulkanInstance();
		vulkanInstance.emplace(VulkanInstance(config));
	}

	bool Renderer::vulkanInstanceExists() const
	{
		return vulkanInstance.has_value();
	}

	const VulkanInstance& Renderer::getVulkanInstance() const
	{
		pyra_assert(vulkanInstanceExists());
		return *vulkanInstance;
	}

	void Renderer::destroyVulkanInstance()
	{
		destroyWindowSurface();
		vulkanInstance.reset();
	}

	void Renderer::constructWindowSurface(Surface::Config config)
	{
		pyra_assert(vulkanInstanceExists());
		destroyWindowSurface();
		windowSurface.emplace(Surface(config,*vulkanInstance));
	}

	bool Renderer::windowSurfaceExists() const
	{
		return windowSurface.has_value();
	}

	const Surface& Renderer::getWindowSurface() const
	{
		pyra_assert(windowSurfaceExists());
		return *windowSurface;
	}

	void Renderer::destroyWindowSurface()
	{
		unselectDevice(); 
		windowSurface.reset();
	}

	std::vector<Device::Config> Renderer::getSuitableDevices()
	{
		pyra_assert(vulkanInstanceExists()); 
		pyra_assert(windowSurfaceExists()); 
		return Device::getSuitableDevices(*vulkanInstance, *windowSurface);
	}

	void Renderer::selectDevice(Device::Config config)
	{
		pyra_assert(vulkanInstanceExists()); 
		pyra_assert(windowSurfaceExists());

		// Destroy the previous device and select a new one
		unselectDevice();
		renderDevice.emplace(Device(config));

		// Create a command pool for creating concurrent frames
		vk::CommandPoolCreateInfo commandPoolCreateInfo; 
		commandPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		commandPoolCreateInfo.queueFamilyIndex = renderDevice->getQueueFamilyIndex();
		frameCommandPool.emplace(renderDevice->getLogicalDevice(), commandPoolCreateInfo);

		// Create graphics pipeline cache
		vk::PipelineCacheCreateInfo pipelineCacheCreateInfo; 
		pipelineCacheCreateInfo.initialDataSize = 0; 
		graphicsPipelineCache.emplace(renderDevice->getLogicalDevice(), pipelineCacheCreateInfo);

		// Create a device memory allocator
		vma::AllocatorCreateInfo allocatorCreateInfo; 
		allocatorCreateInfo.physicalDevice = renderDevice->getPhysicalDevice();
		allocatorCreateInfo.vulkanApiVersion = vulkanInstance->getConfig().vulkan_api_version;
		memoryAllocator.emplace(vulkanInstance->getInstance(), renderDevice->getLogicalDevice(), allocatorCreateInfo);		

		// Create command pool and command buffers for copying buffers
		copy_CommandPool.emplace(renderDevice->getLogicalDevice(), commandPoolCreateInfo); 
		vk::CommandBufferAllocateInfo copyCommandBufferAllocateInfo;
		copyCommandBufferAllocateInfo.commandBufferCount = 1; 
		copyCommandBufferAllocateInfo.commandPool = *copy_CommandPool;
		copyCommandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary; 
		copy_CommandBuffer.emplace(std::move(vk::raii::CommandBuffers(renderDevice->getLogicalDevice(), copyCommandBufferAllocateInfo).front()));
	}

	bool Renderer::deviceSelected() const
	{
		return renderDevice.has_value();
	}

	const Device& Renderer::getDevice() const
	{
		pyra_assert(deviceSelected()); 
		return *renderDevice;
	}

	void Renderer::unselectDevice()
	{
		if (renderDevice.has_value())
			renderDevice->getLogicalDevice().waitIdle();

		destroySwapchain();

		destroyConcurrentFrames(); 
		frameCommandPool.reset();

		destroyAllShaders(); 
		destroyAllGraphicsPipelines(); 
		graphicsPipelineCache.reset();

		copy_CommandBuffer.reset(); 
		copy_CommandPool.reset();

		destroyAllVertexBuffers();

		memoryAllocator.reset();
		renderDevice.reset();
	}

	Swapchain::Config Renderer::constructSwapchain(Swapchain::Config requestedConfig)
	{
		pyra_assert(vulkanInstanceExists());
		pyra_assert(windowSurfaceExists());
		pyra_assert(deviceSelected());

		destroySwapchain();
		swapchain.emplace(Swapchain(requestedConfig, *renderDevice, *windowSurface));

		return swapchain->getConfig();
	}

	bool Renderer::swapchainExists() const
	{
		return swapchain.has_value();
	}

	const Swapchain& Renderer::getSwapchain() const
	{
		pyra_assert(swapchainExists());
		return *swapchain;
	}

	void Renderer::destroySwapchain()
	{
		swapchain.reset();
	}

	void Renderer::constructConcurrentFrames(uint32_t frameCount)
	{
		pyra_assert(vulkanInstanceExists());
		pyra_assert(windowSurfaceExists());
		pyra_assert(deviceSelected());

		destroyConcurrentFrames();
		concurrentFrames.reserve(frameCount);

		currentFrameIdx = 0; 

		vk::CommandBufferAllocateInfo commandBufferAllocateInfo; 
		commandBufferAllocateInfo.commandBufferCount = frameCount;
		commandBufferAllocateInfo.commandPool = *frameCommandPool;
		commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary; 

		vk::raii::CommandBuffers commandBuffers((*renderDevice).getLogicalDevice(), commandBufferAllocateInfo);

		vk::FenceCreateInfo fenceCreateInfo; 
		fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled; 
	
		vk::SemaphoreCreateInfo semaphoreCreateInfo; 
		
		for (uint32_t i = 0;i < frameCount;i++)
		{
			Frame frame{
				.commandBuffer = std::move(commandBuffers[i]),
				.commandBufferReadyFence = vk::raii::Fence((*renderDevice).getLogicalDevice(),fenceCreateInfo),
				.commandBufferImagePaintableSemaphore = vk::raii::Semaphore((*renderDevice).getLogicalDevice(), semaphoreCreateInfo)
			}; 

			concurrentFrames.push_back(std::move(frame));
		}

	}

	uint32_t Renderer::concurrentFramesCount() const
	{
		return concurrentFrames.size();
	}

	void Renderer::destroyConcurrentFrames()
	{
		concurrentFrames.clear();
		currentFrameIdx = 0;
	}

	void Renderer::constructShaders(Shader::Config config)
	{
		pyra_assert(vulkanInstanceExists());
		pyra_assert(windowSurfaceExists());
		pyra_assert(deviceSelected());

		std::shared_ptr<Shader> newShader = std::make_shared<Shader>(config, *renderDevice); 

		for (const auto& vertexShaderEntry : config.vertexShaderEntries)
		{
			shaderEntries.insert(std::pair(vertexShaderEntry, newShader));
		}

		for (const auto& fragmentShaderEntry : config.fragmentShaderEntries)
		{
			shaderEntries.insert(std::pair(fragmentShaderEntry, newShader));
		}
	}

	bool Renderer::shaderEntryExists(std::string entryName) const
	{
		return (shaderEntries.find(entryName) != shaderEntries.end());
	}

	void Renderer::destroyAllShaders()
	{
		shaderEntries.clear();
	}

	ResourceID Renderer::constructGraphicsPipeline(GraphicsPipeline::Config config)
	{
		pyra_assert(vulkanInstanceExists());
		pyra_assert(windowSurfaceExists());
		pyra_assert(deviceSelected());

		pyra_assert(shaderEntryExists(config.vertexShaderEntry)); 
		pyra_assert(shaderEntryExists(config.fragmentShaderEntry));

		return graphicsPipelinePool.add_resource(GraphicsPipeline(config, *renderDevice, *shaderEntries[config.vertexShaderEntry], *shaderEntries[config.fragmentShaderEntry]));
	}

	bool Renderer::graphicsPipelineExists(ResourceID id) const
	{
		return graphicsPipelinePool.resource_vaild(id);
	}

	void Renderer::destroyGraphicsPipeline(ResourceID id)
	{
		graphicsPipelinePool.destroy_resource(id);
	}

	void Renderer::destroyAllGraphicsPipelines()
	{
		graphicsPipelinePool.clear_resources();
	}

	ResourceID Renderer::constructVertexBuffer(const std::vector<Vertex>& verticies)
	{
		pyra_assert(vulkanInstanceExists());
		pyra_assert(windowSurfaceExists());
		pyra_assert(deviceSelected());

		vk::BufferCreateInfo bufferCreateInfo; 
		bufferCreateInfo.queueFamilyIndexCount = 1; 
		bufferCreateInfo.pQueueFamilyIndices = &renderDevice->getQueueFamilyIndex();
		bufferCreateInfo.size = sizeof(Vertex) * verticies.size();
		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc; 

		vma::AllocationCreateInfo allocationCreateInfo; 
		allocationCreateInfo.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;
		allocationCreateInfo.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		allocationCreateInfo.usage = vma::MemoryUsage::eAuto;

		vma::raii::Buffer stageBuffer(*memoryAllocator, bufferCreateInfo, allocationCreateInfo);

		void* mapped = stageBuffer.getAllocation().map();
		memcpy(mapped, verticies.data(), sizeof(Vertex) * verticies.size()); 
		stageBuffer.getAllocation().unmap(); 

		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
		allocationCreateInfo.flags = {};
		allocationCreateInfo.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal; 
		allocationCreateInfo.usage = vma::MemoryUsage::eAuto;

		vma::raii::Buffer vertexBuffer(*memoryAllocator, bufferCreateInfo, allocationCreateInfo);

		copy_CommandBuffer->begin({});
		copy_CommandBuffer->copyBuffer(stageBuffer, vertexBuffer,vk::BufferCopy(0,0, sizeof(Vertex) * verticies.size() ));
		copy_CommandBuffer->end(); 

		vk::CommandBuffer cpyCmdBuff = (vk::CommandBuffer)*copy_CommandBuffer;

		vk::SubmitInfo submitInfo; 
		submitInfo.commandBufferCount = 1; 
		submitInfo.pCommandBuffers = &cpyCmdBuff;
		renderDevice->getGraphicsQueue().submit(submitInfo); 
		renderDevice->getLogicalDevice().waitIdle();

		return vertexBufferPool.add_resource(VertexBuffer(std::move(vertexBuffer),verticies.size()));
	}

	bool Renderer::vertexBufferExists(ResourceID id) const
	{
		return vertexBufferPool.resource_vaild(id);
	}

	void Renderer::destroyVertexBuffer(ResourceID id)
	{
		return vertexBufferPool.destroy_resource(id);
	}

	void Renderer::destroyAllVertexBuffers()
	{
		vertexBufferPool.clear_resources();
	}

	ResourceID Renderer::constructIndexBuffer(const std::vector<uint32_t>& indicies)
	{
		return ResourceID();
	}

	void Renderer::destroyIndexBuffer(ResourceID id)
	{

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
	

	
	void Renderer::Draw(RenderTarget target, glm::vec4 clearColor)
	{
		pyra_assert(vulkanInstanceExists()); 
		pyra_assert(windowSurfaceExists());
		pyra_assert(deviceSelected());
		pyra_assert(swapchainExists());
		pyra_assert(concurrentFramesCount() > 0);
		pyra_assert(graphicsPipelineExists(target.graphics_pipeline)); 
		pyra_assert(vertexBufferExists(target.vertex_buffer));

		if (graphicsPipelinePool.get_resource(target.graphics_pipeline).getConfig().colorAttachmentFormat != swapchain->getConfig().imageFormat)
		{
			pyra_error("Graphics pipeline's target color format and Swapchain's image format mismatch!");
			return;
		}

		currentFrameIdx %= concurrentFrames.size();
		const auto& logicalDevice = renderDevice->getLogicalDevice();
		const auto& graphicsQueue = renderDevice->getGraphicsQueue();
		const auto& currentFrame = concurrentFrames[currentFrameIdx];
		const auto& swapchain_ = swapchain->getSwapchain();
		const auto& commandBuffer = currentFrame.commandBuffer;

		const auto& graphicsPipeline = graphicsPipelinePool.get_resource(target.graphics_pipeline);
		const auto& vertexBuffer = vertexBufferPool.get_resource(target.vertex_buffer);

		logicalDevice.waitForFences(*currentFrame.commandBufferReadyFence, VK_TRUE, UINT64_MAX);
		logicalDevice.resetFences(*currentFrame.commandBufferReadyFence);

		auto [result, imageIdx] = swapchain_.acquireNextImage(UINT64_MAX, *currentFrame.commandBufferImagePaintableSemaphore);
		
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
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline.getGraphicsPipeline());
		commandBuffer.setViewport(0, target.viewport); 
		commandBuffer.setScissor(0, target.scissor);
		commandBuffer.bindVertexBuffers(0,*vertexBuffer.getVertexBuffer(),vertexBuffer.getVertexBuffer().getAllocation().getInfo().offset);
		commandBuffer.draw(vertexBuffer.getVertexCount(), 1, 0, 0);
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

		VkSemaphore wS = (VkSemaphore)*currentFrame.commandBufferImagePaintableSemaphore;
		VkPipelineStageFlags waitDestination = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkCommandBuffer cB = (VkCommandBuffer)*commandBuffer;
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

		graphicsQueue.submit(vk::SubmitInfo(submitInfo),currentFrame.commandBufferReadyFence);

		VkSwapchainKHR sC = (VkSwapchainKHR)*swapchain_;

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

		graphicsQueue.presentKHR(vk::PresentInfoKHR(presentInfo));
		currentFrameIdx++;
	} 

}