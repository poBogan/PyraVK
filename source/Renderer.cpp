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

	void Renderer::constructVulkanInstance(const VulkanInstance::Config& config)
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

	void Renderer::constructWindowSurface(const VkSurfaceKHR& surface)
	{
		pyra_assert(vulkanInstanceExists());
		destroyWindowSurface();
		windowSurface.emplace(Surface(surface,*vulkanInstance));
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

	std::vector<Device::Config> Renderer::getSuitableDevices() const
	{
		pyra_assert(vulkanInstanceExists()); 
		pyra_assert(windowSurfaceExists()); 
		return Device::getSuitableDevices(*vulkanInstance, *windowSurface);
	}

	void Renderer::selectDevice(const Device::Config& config)
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
		allocatorCreateInfo.physicalDevice = *renderDevice->getPhysicalDevice();
		allocatorCreateInfo.vulkanApiVersion = vulkanInstance->getConfig().vulkan_api_version;
		memoryAllocator.emplace(vulkanInstance->getInstance(), renderDevice->getLogicalDevice(), allocatorCreateInfo);		

		// Create command pool and command buffers for copying buffers
		copy_CommandPool.emplace(renderDevice->getLogicalDevice(), commandPoolCreateInfo); 
		vk::CommandBufferAllocateInfo copyCommandBufferAllocateInfo;
		copyCommandBufferAllocateInfo.commandBufferCount = 1; 
		copyCommandBufferAllocateInfo.commandPool = **copy_CommandPool;
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

		constexpr vk::SemaphoreCreateInfo semaphoreCreateInfo{};

		for (uint32_t i = 0;i < frameCount;i++)
		{
			Frame frame{
				.commandBuffer = std::move(commandBuffers[i]),
				.commandBufferReadyFence = vk::raii::Fence(renderDevice->getLogicalDevice(),fenceCreateInfo),
				.commandBufferImagePaintableSemaphore = vk::raii::Semaphore(renderDevice->getLogicalDevice(), semaphoreCreateInfo)
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

	void Renderer::constructShaders(const Shader::Config& config)
	{
		pyra_assert(vulkanInstanceExists());
		pyra_assert(windowSurfaceExists());
		pyra_assert(deviceSelected());

		auto newShader = std::make_shared<Shader>(config, *renderDevice);

		for (const auto& vertexShaderEntry : config.vertexShaderEntries)
		{
			shaderEntries.insert(std::pair(vertexShaderEntry, newShader));
		}

		for (const auto& fragmentShaderEntry : config.fragmentShaderEntries)
		{
			shaderEntries.insert(std::pair(fragmentShaderEntry, newShader));
		}
	}

	bool Renderer::shaderEntryExists(const std::string& entryName) const
	{
		return (shaderEntries.contains(entryName));
	}

	void Renderer::destroyAllShaders()
	{
		shaderEntries.clear();
	}

	ResourceID Renderer::constructGraphicsPipeline(const GraphicsPipeline::Config& config)
	{
		pyra_assert(vulkanInstanceExists());
		pyra_assert(windowSurfaceExists());
		pyra_assert(deviceSelected());

		pyra_assert(shaderEntryExists(config.vertexShaderEntry)); 
		pyra_assert(shaderEntryExists(config.fragmentShaderEntry));

		return graphicsPipelinePool.add_resource(GraphicsPipeline(config, *renderDevice, *shaderEntries[config.vertexShaderEntry], *shaderEntries[config.fragmentShaderEntry]));
	}

	bool Renderer::graphicsPipelineExists(const ResourceID id) const
	{
		return graphicsPipelinePool.resource_vaild(id);
	}

	void Renderer::destroyGraphicsPipeline(const ResourceID id)
	{
		graphicsPipelinePool.destroy_resource(id);
	}

	void Renderer::destroyAllGraphicsPipelines()
	{
		graphicsPipelinePool.clear_resources();
	}

	ResourceID Renderer::constructVertexBuffer(const std::vector<Vertex>& vertices)
	{
		pyra_assert(vulkanInstanceExists());
		pyra_assert(windowSurfaceExists());
		pyra_assert(deviceSelected());

		vk::BufferCreateInfo bufferCreateInfo; 
		bufferCreateInfo.queueFamilyIndexCount = 1; 
		bufferCreateInfo.pQueueFamilyIndices = &renderDevice->getQueueFamilyIndex();
		bufferCreateInfo.size = sizeof(Vertex) * vertices.size();
		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc; 

		vma::AllocationCreateInfo allocationCreateInfo; 
		allocationCreateInfo.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;
		allocationCreateInfo.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
		allocationCreateInfo.usage = vma::MemoryUsage::eAuto;

		const vma::raii::Buffer stageBuffer(*memoryAllocator, bufferCreateInfo, allocationCreateInfo);

		void* mapped = stageBuffer.getAllocation().map();
		memcpy(mapped, vertices.data(), sizeof(Vertex) * vertices.size());
		stageBuffer.getAllocation().unmap(); 

		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
		allocationCreateInfo.flags = {};
		allocationCreateInfo.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal; 
		allocationCreateInfo.usage = vma::MemoryUsage::eAuto;

		vma::raii::Buffer vertexBuffer(*memoryAllocator, bufferCreateInfo, allocationCreateInfo);

		copy_CommandBuffer->begin({});
		copy_CommandBuffer->copyBuffer(stageBuffer, vertexBuffer,vk::BufferCopy(0,0, sizeof(Vertex) * vertices.size() ));
		copy_CommandBuffer->end();

		vk::SubmitInfo submitInfo; 
		submitInfo.commandBufferCount = 1; 
		submitInfo.pCommandBuffers = (&(**copy_CommandBuffer));
		renderDevice->getGraphicsQueue().submit(submitInfo); 
		renderDevice->getLogicalDevice().waitIdle();

		return vertexBufferPool.add_resource(VertexBuffer(std::move(vertexBuffer),verticies.size()));
	}

	bool Renderer::vertexBufferExists(const ResourceID id) const
	{
		return vertexBufferPool.resource_vaild(id);
	}

	void Renderer::destroyVertexBuffer(const ResourceID id)
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

	static void transition_image_layout(
		const vk::raii::CommandBuffer&	commandBuffer,
		const vk::PipelineStageFlags2 	srcStageMask,
		const vk::PipelineStageFlags2 	dstStageMask,
		const vk::AccessFlags2			srcAccessMask,
		const vk::AccessFlags2			dstAccessMask,
		const vk::ImageLayout			oldImageLayout,
		const vk::ImageLayout			newImageLayout,
		const uint32_t					srcQueueFamilyIndex,
		const uint32_t					dstQueueFamilyIndex,
		const vk::Image					image
	)
	{
		vk::ImageMemoryBarrier2 barrier;
		barrier.srcStageMask = srcStageMask;
		barrier.srcAccessMask = srcAccessMask;
		barrier.dstStageMask = dstStageMask;
		barrier.dstAccessMask = dstAccessMask;
		barrier.oldLayout = oldImageLayout;
		barrier.newLayout = newImageLayout;
		barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
		barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
		barrier.image = image;
		barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

		vk::DependencyInfo dependencyInfo;
		dependencyInfo.pNext = nullptr;
		dependencyInfo.dependencyFlags = {};
		dependencyInfo.memoryBarrierCount = 0;
		dependencyInfo.pMemoryBarriers = nullptr;
		dependencyInfo.bufferMemoryBarrierCount = 0;
		dependencyInfo.pBufferMemoryBarriers = nullptr;
		dependencyInfo.imageMemoryBarrierCount = 1;
		dependencyInfo.pImageMemoryBarriers = &barrier;

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
		const auto& [commandBuffer, commandBufferReadyFence, commandBufferImagePaintableSemaphore] = concurrentFrames[currentFrameIdx];
		const auto& swapchain_ = swapchain->getSwapchain();

		const auto& graphicsPipeline = graphicsPipelinePool.get_resource(target.graphics_pipeline);
		const auto& vertexBuffer = vertexBufferPool.get_resource(target.vertex_buffer);

		vk::Result result = logicalDevice.waitForFences(*commandBufferReadyFence, vk::True, std::numeric_limits<uint64_t>::max());
		logicalDevice.resetFences(*commandBufferReadyFence);

		auto [res, imageIdx] = swapchain_.acquireNextImage(UINT64_MAX, *commandBufferImagePaintableSemaphore);
		
		commandBuffer.begin({});

		transition_image_layout(
			commandBuffer,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			{},
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::QueueFamilyIgnored,
			vk::QueueFamilyIgnored,
			swapchain->getImages()[imageIdx]
			);

		vk::ClearColorValue clearColorValue;
		clearColorValue.float32[0] = clearColor.r;
		clearColorValue.float32[1] = clearColor.g;
		clearColorValue.float32[2] = clearColor.b;
		clearColorValue.float32[3] = clearColor.a;

		vk::RenderingAttachmentInfo colorAttachmentInfo;
		colorAttachmentInfo.pNext = nullptr;
		colorAttachmentInfo.imageView = *swapchain->getImageViews()[imageIdx];
		colorAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachmentInfo.clearValue = clearColorValue;

		vk::RenderingInfo renderingInfo;
		renderingInfo.pNext = nullptr;
		renderingInfo.flags = {};
		renderingInfo.renderArea = vk::Rect2D(vk::Offset2D(0,0), swapchain->getConfig().imageExtent);
		renderingInfo.layerCount = 1;
		renderingInfo.viewMask = 0;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachmentInfo;

		commandBuffer.beginRendering(renderingInfo); 
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline.getGraphicsPipeline());
		commandBuffer.setViewport(0, target.viewport); 
		commandBuffer.setScissor(0, target.scissor);
		commandBuffer.bindVertexBuffers(0,*vertexBuffer.getVertexBuffer(),vertexBuffer.getVertexBuffer().getAllocation().getInfo().offset);
		commandBuffer.draw(vertexBuffer.getVertexCount(), 1, 0, 0);
		commandBuffer.endRendering();

		transition_image_layout(
			commandBuffer,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eBottomOfPipe,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			{},
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::QueueFamilyIgnored,
			vk::QueueFamilyIgnored,
			swapchain->getImages()[imageIdx]
		);

		commandBuffer.end();

		vk::PipelineStageFlags waitDestination = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::SubmitInfo submitInfo;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &*commandBufferImagePaintableSemaphore;
		submitInfo.pWaitDstStageMask = &waitDestination;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &*commandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &*swapchain->getImagePresentableSemaphores()[imageIdx];

		graphicsQueue.submit(submitInfo,commandBufferReadyFence);

		vk::PresentInfoKHR presentInfo;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &*swapchain->getImagePresentableSemaphores()[imageIdx];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &*swapchain_;
		presentInfo.pImageIndices = &imageIdx;
		presentInfo.pResults = nullptr;

		result = graphicsQueue.presentKHR(presentInfo);
		currentFrameIdx++;
	} 

}