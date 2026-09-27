#pragma once 
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>

namespace pyra
{

	class Frames
	{
	public : 
		struct Config
		{
			uint32_t concurrentFramesCount;
		};
	private :
		vk::raii::CommandBuffers commandBuffers = nullptr; 
		std::vector<vk::raii::Fence> commandBufferFences; 
		std::vector<vk::raii::Semaphore> commandBufferSemaphores;
		const Frames::Config config;
		uint32_t currentFrame = 0;
	public : 
		Frames(const Device& device, Frames::Config config);
		const vk::raii::CommandBuffers& getCommandBuffers() const; 
		const std::vector<vk::raii::Fence>& getCommandBufferFences() const; 
		const std::vector<vk::raii::Semaphore>& getCommandBufferSemaphores() const;
		const Frames::Config& getConfig() const;

		const uint32_t getCurrentFrameIndex() const; 
		const vk::raii::CommandBuffer& getCurrentFrameCommandBuffer() const; 
		const vk::raii::Fence& getCurrentFrameCommandBufferFence() const; 
		const vk::raii::Semaphore& getCurrentFrameCommandBufferSemaphore() const;
		void nextFrame();
	};

}