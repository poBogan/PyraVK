#include <PyraVK/Frames.h>


namespace pyra
{
	Frames::Frames(const Device& device, Frames::Config config) : config(config)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = NULL,
			.commandPool = (VkCommandPool)*device.getCommandPoolHandle(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = config.concurrentFramesCount
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

		for (int i = 0;i < config.concurrentFramesCount;i++)
		{
			commandBufferFences.push_back(vk::raii::Fence(device.getLogicalDeviceHandle(), fenceCreateInfo));
			commandBufferSemaphores.push_back(vk::raii::Semaphore(device.getLogicalDeviceHandle(), semaphoreCreateInfo));
		}

		commandBuffers.shrink_to_fit();
		commandBufferFences.shrink_to_fit();
		commandBufferSemaphores.shrink_to_fit();
	}

	const vk::raii::CommandBuffers& Frames::getCommandBuffers() const
	{
		return commandBuffers;
	}

	const std::vector<vk::raii::Fence>& Frames::getCommandBufferFences() const
	{
		return commandBufferFences;
	}

	const std::vector<vk::raii::Semaphore>& Frames::getCommandBufferSemaphores() const
	{
		return commandBufferSemaphores;
	}
	const Frames::Config& Frames::getConfig() const
	{
		return config;
	}

	const uint32_t Frames::getCurrentFrameIndex() const
	{
		return currentFrame;
	}

	const vk::raii::CommandBuffer& Frames::getCurrentFrameCommandBuffer() const
	{
		return commandBuffers[getCurrentFrameIndex()];
	}

	const vk::raii::Fence& Frames::getCurrentFrameCommandBufferFence() const
	{
		return commandBufferFences[getCurrentFrameIndex()];
	}

	const vk::raii::Semaphore& Frames::getCurrentFrameCommandBufferSemaphore() const
	{
		return commandBufferSemaphores[getCurrentFrameIndex()];
	}

	void Frames::nextFrame()
	{
		currentFrame++;
		currentFrame %= config.concurrentFramesCount;
	}


}