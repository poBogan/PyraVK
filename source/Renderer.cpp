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
		pyra_assert(vulkanInstance.has_value(), "Cannot create swapchain. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot create swapchain. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot create swapchain. No render device selected!");

		destroySwapchain();
		swapchain.emplace(Swapchain(renderDevice.value(), windowSurface.value(), window_size_in_pixels, targetImageCount, imageFormatPreferences, presentModePreferences));
		renderDevice->constructCommandBuffers(swapchain->getSwapchainImageCount() - 1);
	}

	void pyra::Renderer::constructGraphicsPipeline(
		vk::Viewport viewport,
		vk::Rect2D scissor,
		vk::PrimitiveTopology primitiveTopology,
		vk::CullModeFlagBits faceCulling,
		vk::FrontFace frontFace,
		vk::PolygonMode polygonMode,
		vk::SampleCountFlagBits sampleCount
	)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot create graphics pipeline. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot create graphics pipeline. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot create graphics pipeline. No render device selected!");
		pyra_assert(swapchain.has_value(), "Cannot create graphics pipeline. No swapchain found!");

		destroyGraphicsPipeline();
		graphicsPipeline.emplace(GraphicsPipeline(renderDevice.value(), swapchain.value().getSwapchainImageFormat().format, viewport, scissor, primitiveTopology, faceCulling, frontFace, polygonMode, sampleCount));
	}

	void Renderer::drawFrame()
	{
		static uint32_t frame_idx = 0;
		uint32_t frames_in_flight = renderDevice->getFramesInFlightCount();
		frame_idx %= frames_in_flight;

		const auto& logical_device = renderDevice->getLogicalDeviceHandle();
		const auto& graphicsQueue = renderDevice->getGraphicsQueueHandle();
		const auto& commandBuffers = renderDevice->getCommandBuffers();
		const auto& commandBufferAvailableFences = renderDevice->getCommandBufferAvailableFences();
		const auto& imagePresentableSemaphores = swapchain->getImagePresentableSemaphores();
		const auto& imagePaintableSemaphores = renderDevice->getTargetImagePaintableSemaphores();
		const auto& swapchain_handle = swapchain->getSwapchainHandle();
		const auto& swapchainImages = swapchain->getSwapchainImages();
		const auto& swapchainImageViews = swapchain->getSwapchainImageViews();
		const auto& swapchainImageExtent = swapchain->getSwapchainImageExtent();

		logical_device.waitForFences(*commandBufferAvailableFences[frame_idx], VK_TRUE, UINT64_MAX);
		logical_device.resetFences(*commandBufferAvailableFences[frame_idx]);

		auto [res,image_idx] = swapchain_handle.acquireNextImage(UINT64_MAX, *imagePaintableSemaphores[frame_idx]);

		commandBuffers[frame_idx].begin({});

		VkImageMemoryBarrier2 transition_optimal_color_attachment_barrier{
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
			.image = swapchainImages[image_idx],
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}				
		};

		VkDependencyInfo transition_optimal_color_attachment_dependency{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = NULL,
			.dependencyFlags = {},
			.memoryBarrierCount = 0,
			.pMemoryBarriers = NULL,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = NULL,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &transition_optimal_color_attachment_barrier
		};

		commandBuffers[frame_idx].pipelineBarrier2(vk::DependencyInfo(transition_optimal_color_attachment_dependency)); 

		vk::ClearColorValue clearVal(0.f, 0.f, 0.f, 1.f);
		VkRenderingAttachmentInfo renderingAttachment{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
			.pNext = NULL,
			.imageView = (VkImageView)*swapchainImageViews[image_idx],
			.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {.color = (VkClearColorValue)clearVal},
		};

		VkRenderingInfo renderingInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.pNext = NULL,
			.flags = {},
			.renderArea = {.offset = {0,0}, .extent = (VkExtent2D)*swapchainImageExtent},
			.layerCount = 1,
			.viewMask = 0,
			.colorAttachmentCount = 1,
			.pColorAttachments = &renderingAttachment
		};

		commandBuffers[frame_idx].beginRendering(renderingInfo);

		commandBuffers[frame_idx].bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline->getPipelineHandle());
		commandBuffers[frame_idx].draw(3, 1, 0, 0);

		commandBuffers[frame_idx].endRendering();

		VkImageMemoryBarrier2 transition_presentation_barrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = NULL,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
			.dstAccessMask = {},
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = swapchainImages[image_idx],
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VkDependencyInfo transition_presentation_dependency{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext = NULL,
			.dependencyFlags = {},
			.memoryBarrierCount = 0,
			.pMemoryBarriers = NULL,
			.bufferMemoryBarrierCount = 0,
			.pBufferMemoryBarriers = NULL,
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &transition_presentation_barrier
		};

		commandBuffers[frame_idx].pipelineBarrier2(vk::DependencyInfo(transition_presentation_dependency));
		commandBuffers[frame_idx].end();

		VkSemaphore imageReadySemaphore = (VkSemaphore)*imagePaintableSemaphores[frame_idx];
		VkPipelineStageFlags waitDestStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkCommandBuffer commandBuffer = (VkCommandBuffer)*commandBuffers[frame_idx];
		VkSemaphore imagePaintedSemaphore = (VkSemaphore)*imagePresentableSemaphores[image_idx];

		VkSubmitInfo submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &imageReadySemaphore,
			.pWaitDstStageMask = &waitDestStageMask,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &imagePaintedSemaphore,
		};

		graphicsQueue.submit(vk::SubmitInfo(submitInfo), *commandBufferAvailableFences[frame_idx]);
		VkSwapchainKHR sc = (VkSwapchainKHR)*swapchain->getSwapchainHandle();

		VkPresentInfoKHR presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &imagePaintedSemaphore,
			.swapchainCount = 1,
			.pSwapchains = &sc,
			.pImageIndices = &image_idx
		};

		graphicsQueue.presentKHR(presentInfo);
		frames_in_flight++;
	}

	void Renderer::destroyVulkanInstance()
	{
		destroyWindowSurface(); 
		vulkanInstance.has_value();
	}

	void Renderer::destroyWindowSurface()
	{
		destroyRenderDevice(); 
		windowSurface.reset();
	}

	void Renderer::destroyRenderDevice()
	{
		destroySwapchain(); 
		renderDevice.reset();
	}

	void Renderer::destroySwapchain()
	{
		destroyGraphicsPipeline(); 
		swapchain.reset();
	}

	void Renderer::destroyGraphicsPipeline()
	{
		graphicsPipeline.reset();
	}

	const vk::raii::Context& Renderer::getContext() const
	{
		return context;
	}
	const VulkanInstance& Renderer::getVulkanInstance() const
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot return vulkan instance! No instance found");
		return vulkanInstance.value();
	}
	const Surface& Renderer::getWindowSurface() const
	{
		pyra_assert(windowSurface.has_value(), "Cannot return surface. No surface found");
		return windowSurface.value();
	}

	const Device& Renderer::getRenderDevice() const
	{
		pyra_assert(renderDevice.has_value(), "Cannot return render device. No device selected!");
		return renderDevice.value();
	}

	const Swapchain& Renderer::getSwapchain() const
	{
		pyra_assert(swapchain.has_value(), "Cannot return swapchain. No swapchain found!");
		return swapchain.value();
	}

	const GraphicsPipeline& Renderer::getGraphicsPipeline() const
	{
		pyra_assert(graphicsPipeline.has_value(), "Cannot return graphics pipeline. No swapchain found");
		return graphicsPipeline.value();
	}

	bool pyra::Renderer::vulkanInstanceExists() const
	{
		return vulkanInstance.has_value();
	}
	bool pyra::Renderer::windowSurfaceExists() const
	{
		return windowSurface.has_value();
	}
	bool pyra::Renderer::renderDeviceExists() const
	{
		return renderDevice.has_value();
	}
	bool pyra::Renderer::swapchainExists() const
	{
		return swapchain.has_value();
	}
	bool pyra::Renderer::graphicsPipelineExists() const
	{
		return graphicsPipeline.has_value();
	}
}