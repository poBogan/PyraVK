#pragma once 
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>

namespace pyra
{

	class FramesInFlight
	{
	private :
		vk::raii::CommandBuffers commandBuffers = nullptr; 
		std::vector<vk::raii::Fence> commandBufferFences; 
		std::vector<vk::raii::Semaphore> commandBufferSemaphores;
		uint32_t currentFrame = 0;
		const uint32_t framesInFlightCount;
	public : 
		FramesInFlight(const Device& device,uint32_t framesInFlightCount);
		const vk::raii::CommandBuffers& getCommandBuffers() const; 
		const std::vector<vk::raii::Fence>& getCommandBufferFences() const; 
		const std::vector<vk::raii::Semaphore>& getCommandBufferSemaphores() const;

		const uint32_t getCurrentFrameIndex() const; 
		const uint32_t getFramesInFlightCount() const;
		const vk::raii::CommandBuffer& getCurrentFrameCommandBuffer() const; 
		const vk::raii::Fence& getCurrentFrameCommandBufferFence() const; 
		const vk::raii::Semaphore& getCurrentFrameCommandBufferSemaphore() const;
		void nextFrame();
	};

}