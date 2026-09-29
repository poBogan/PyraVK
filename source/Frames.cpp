#include <PyraVK/Frames.h>


namespace pyra
{
	FramesInFlight::FramesInFlight(const Device& device,uint32_t framesInFlightCount) : framesInFlightCount(framesInFlightCount)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = NULL,
			.commandPool = (VkCommandPool)*device.getCommandPoolHandle(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = framesInFlightCount
		};

		commandBuffers = vk::raii::CommandBuffers(device.getLogicalDeviceHandle(), commandBufferAllocateInfo);

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

		for (int i = 0;i < framesInFlightCount;i++)
		{
			commandBufferFences.push_back(vk::raii::Fence(device.getLogicalDeviceHandle(), fenceCreateInfo));
			commandBufferSemaphores.push_back(vk::raii::Semaphore(device.getLogicalDeviceHandle(), semaphoreCreateInfo));
		}

		commandBuffers.shrink_to_fit();
		commandBufferFences.shrink_to_fit();
		commandBufferSemaphores.shrink_to_fit();
	}

	const vk::raii::CommandBuffers& FramesInFlight::getCommandBuffers() const
	{
		return commandBuffers;
	}

	const std::vector<vk::raii::Fence>& FramesInFlight::getCommandBufferFences() const
	{
		return commandBufferFences;
	}

	const std::vector<vk::raii::Semaphore>& FramesInFlight::getCommandBufferSemaphores() const
	{
		return commandBufferSemaphores;
	}

	const uint32_t FramesInFlight::getCurrentFrameIndex() const
	{
		return currentFrame;
	}

	const uint32_t FramesInFlight::getFramesInFlightCount() const
	{
		return framesInFlightCount;
	}

	const vk::raii::CommandBuffer& FramesInFlight::getCurrentFrameCommandBuffer() const
	{
		return commandBuffers[getCurrentFrameIndex()];
	}

	const vk::raii::Fence& FramesInFlight::getCurrentFrameCommandBufferFence() const
	{
		return commandBufferFences[getCurrentFrameIndex()];
	}

	const vk::raii::Semaphore& FramesInFlight::getCurrentFrameCommandBufferSemaphore() const
	{
		return commandBufferSemaphores[getCurrentFrameIndex()];
	}

	void FramesInFlight::nextFrame()
	{
		currentFrame++;
		currentFrame %= framesInFlightCount;
	}


}