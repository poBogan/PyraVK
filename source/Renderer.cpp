#include <PyraVK/Renderer.h>
#include "PyraVK/EngineContext.h"

namespace pyra
{

	Renderer::Renderer(const EngineContext& engineContext,Swapchain&& swapchain, const uint32_t frames_in_flight_count) : m_swapchain(std::move(swapchain))
	{
		vk::CommandPoolCreateInfo cmdPoolCreateInfo;
		cmdPoolCreateInfo.queueFamilyIndex = engineContext.getDevice().get_queue_family_index();
		cmdPoolCreateInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		m_command_pool = vk::raii::CommandPool(engineContext.getDevice().get_logical_device(), cmdPoolCreateInfo);

		set_new_frames_in_flight(engineContext,frames_in_flight_count);
	}

	void Renderer::setNewSwapchain(Swapchain&& swapchain)
	{
		m_swapchain = std::move(swapchain);
	}

	void Renderer::set_new_frames_in_flight(const EngineContext& engineContext, const uint32_t frames_in_flight_count)
	{
		vk::CommandBufferAllocateInfo cmdBufferAllocateInfo;
		cmdBufferAllocateInfo.commandPool = m_command_pool;
		cmdBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
		cmdBufferAllocateInfo.commandBufferCount = frames_in_flight_count;

		vk::raii::CommandBuffers cmdBuffers(engineContext.getDevice().get_logical_device(),cmdBufferAllocateInfo);

		vk::FenceCreateInfo fenceCreateInfo;
		fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

		for (int i = 0; i < frames_in_flight_count; i++)
		{
			vk::SemaphoreCreateInfo semaphoreCreateInfo;
			Frame frame {
				.command_buffer = std::move(cmdBuffers[i]),
				.command_buffer_available_fence = vk::raii::Fence(engineContext.getDevice().get_logical_device(),fenceCreateInfo),
				.frame_image_paintable_semaphore = vk::raii::Semaphore(engineContext.getDevice().get_logical_device(),semaphoreCreateInfo),
			};
			m_frames_in_flight.push_back(std::move(frame));
		}
	}

	const Swapchain& Renderer::getSwapchain() const
	{
		return m_swapchain;
	}

	const std::vector<Frame>& Renderer::get_frames_in_flight() const
	{
		return m_frames_in_flight;
	}


	// static void transition_image_layout(
	// 	const vk::raii::CommandBuffer&	commandBuffer,
	// 	const vk::PipelineStageFlags2 	srcStageMask,
	// 	const vk::PipelineStageFlags2 	dstStageMask,
	// 	const vk::AccessFlags2			srcAccessMask,
	// 	const vk::AccessFlags2			dstAccessMask,
	// 	const vk::ImageLayout			oldImageLayout,
	// 	const vk::ImageLayout			newImageLayout,
	// 	const uint32_t					srcQueueFamilyIndex,
	// 	const uint32_t					dstQueueFamilyIndex,
	// 	const vk::Image					image
	// )
	// {
	// 	vk::ImageMemoryBarrier2 barrier;
	// 	barrier.srcStageMask = srcStageMask;
	// 	barrier.srcAccessMask = srcAccessMask;
	// 	barrier.dstStageMask = dstStageMask;
	// 	barrier.dstAccessMask = dstAccessMask;
	// 	barrier.oldLayout = oldImageLayout;
	// 	barrier.newLayout = newImageLayout;
	// 	barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
	// 	barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
	// 	barrier.image = image;
	// 	barrier.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
	//
	// 	vk::DependencyInfo dependencyInfo;
	// 	dependencyInfo.pNext = nullptr;
	// 	dependencyInfo.dependencyFlags = {};
	// 	dependencyInfo.memoryBarrierCount = 0;
	// 	dependencyInfo.pMemoryBarriers = nullptr;
	// 	dependencyInfo.bufferMemoryBarrierCount = 0;
	// 	dependencyInfo.pBufferMemoryBarriers = nullptr;
	// 	dependencyInfo.imageMemoryBarrierCount = 1;
	// 	dependencyInfo.pImageMemoryBarriers = &barrier;
	//
	// 	commandBuffer.pipelineBarrier2(dependencyInfo);
	// }
	//
	// void Renderer::Draw(RenderTarget target,vk::Viewport viewport, vk::Rect2D scissor,glm::vec4 clearColor)
	// {
	// 	pyra_assert(vulkanInstanceExists());
	// 	pyra_assert(windowSurfaceExists());
	// 	pyra_assert(deviceSelected());
	// 	pyra_assert(swapchainExists());
	// 	pyra_assert(concurrentFramesCount() > 0);
	// 	pyra_assert(graphicsPipelineExists(target.graphics_pipeline));
	// 	pyra_assert(vertexBufferExists(target.vertex_buffer));
	//
	// 	if (graphicsPipelinePool.get_resource(target.graphics_pipeline).getConfig().colorAttachmentFormat != swapchain->getConfig().imageFormat)
	// 	{
	// 		pyra_error("Graphics pipeline's target color format and Swapchain's image format mismatch!");
	// 		return;
	// 	}
	//
	// 	currentFrameIdx %= concurrentFrames.size();
	// 	const auto& logicalDevice = renderDevice->getLogicalDevice();
	// 	const auto& graphicsQueue = renderDevice->getGraphicsQueue();
	// 	const auto& [commandBuffer, commandBufferReadyFence, commandBufferImagePaintableSemaphore] = concurrentFrames[currentFrameIdx];
	// 	const auto& swapchain_ = swapchain->getSwapchain();
	//
	// 	const auto& graphicsPipeline = graphicsPipelinePool.get_resource(target.graphics_pipeline);
	// 	const auto& vertexBuffer = vertexBufferPool.get_resource(target.vertex_buffer);
	//
	// 	vk::Result result = logicalDevice.waitForFences(*commandBufferReadyFence, vk::True, std::numeric_limits<uint64_t>::max());
	// 	logicalDevice.resetFences(*commandBufferReadyFence);
	//
	// 	auto [res, imageIdx] = swapchain_.acquireNextImage(UINT64_MAX, *commandBufferImagePaintableSemaphore);
	//
	// 	commandBuffer.begin({});
	//
	// 	transition_image_layout(
	// 		commandBuffer,
	// 		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
	// 		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
	// 		{},
	// 		vk::AccessFlagBits2::eColorAttachmentWrite,
	// 		vk::ImageLayout::eUndefined,
	// 		vk::ImageLayout::eColorAttachmentOptimal,
	// 		vk::QueueFamilyIgnored,
	// 		vk::QueueFamilyIgnored,
	// 		swapchain->getImages()[imageIdx]
	// 		);
	//
	// 	vk::ClearColorValue clearColorValue;
	// 	clearColorValue.float32[0] = clearColor.r;
	// 	clearColorValue.float32[1] = clearColor.g;
	// 	clearColorValue.float32[2] = clearColor.b;
	// 	clearColorValue.float32[3] = clearColor.a;
	//
	// 	vk::RenderingAttachmentInfo colorAttachmentInfo;
	// 	colorAttachmentInfo.pNext = nullptr;
	// 	colorAttachmentInfo.imageView = *swapchain->getImageViews()[imageIdx];
	// 	colorAttachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	// 	colorAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
	// 	colorAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
	// 	colorAttachmentInfo.clearValue = clearColorValue;
	//
	// 	vk::RenderingInfo renderingInfo;
	// 	renderingInfo.pNext = nullptr;
	// 	renderingInfo.flags = {};
	// 	renderingInfo.renderArea = vk::Rect2D(vk::Offset2D(0,0), swapchain->getConfig().imageExtent);
	// 	renderingInfo.layerCount = 1;
	// 	renderingInfo.viewMask = 0;
	// 	renderingInfo.colorAttachmentCount = 1;
	// 	renderingInfo.pColorAttachments = &colorAttachmentInfo;
	//
	// 	commandBuffer.beginRendering(renderingInfo);
	// 	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline.getGraphicsPipeline());
	// 	commandBuffer.setViewport(0, viewport);
	// 	commandBuffer.setScissor(0, scissor);
	// 	commandBuffer.bindVertexBuffers(0,*vertexBuffer.getVertexBuffer(),vertexBuffer.getVertexBuffer().getAllocation().getInfo().offset);
	// 	commandBuffer.draw(vertexBuffer.getVertexCount(), 1, 0, 0);
	// 	commandBuffer.endRendering();
	//
	// 	transition_image_layout(
	// 		commandBuffer,
	// 		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
	// 		vk::PipelineStageFlagBits2::eBottomOfPipe,
	// 		vk::AccessFlagBits2::eColorAttachmentWrite,
	// 		{},
	// 		vk::ImageLayout::eColorAttachmentOptimal,
	// 		vk::ImageLayout::ePresentSrcKHR,
	// 		vk::QueueFamilyIgnored,
	// 		vk::QueueFamilyIgnored,
	// 		swapchain->getImages()[imageIdx]
	// 	);
	//
	// 	commandBuffer.end();
	//
	// 	vk::PipelineStageFlags waitDestination = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	// 	vk::SubmitInfo submitInfo;
	// 	submitInfo.pNext = nullptr;
	// 	submitInfo.waitSemaphoreCount = 1;
	// 	submitInfo.pWaitSemaphores = &*commandBufferImagePaintableSemaphore;
	// 	submitInfo.pWaitDstStageMask = &waitDestination;
	// 	submitInfo.commandBufferCount = 1;
	// 	submitInfo.pCommandBuffers = &*commandBuffer;
	// 	submitInfo.signalSemaphoreCount = 1;
	// 	submitInfo.pSignalSemaphores = &*swapchain->getImagePresentableSemaphores()[imageIdx];
	//
	// 	graphicsQueue.submit(submitInfo,commandBufferReadyFence);
	//
	// 	vk::PresentInfoKHR presentInfo;
	// 	presentInfo.pNext = nullptr;
	// 	presentInfo.waitSemaphoreCount = 1;
	// 	presentInfo.pWaitSemaphores = &*swapchain->getImagePresentableSemaphores()[imageIdx];
	// 	presentInfo.swapchainCount = 1;
	// 	presentInfo.pSwapchains = &*swapchain_;
	// 	presentInfo.pImageIndices = &imageIdx;
	// 	presentInfo.pResults = nullptr;
	//
	// 	result = graphicsQueue.presentKHR(presentInfo);
	// 	currentFrameIdx++;
	// }
}
