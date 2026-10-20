#pragma once
#include <PyraVK/Object.h>
#include <PyraVK/Swapchain.h>

namespace pyra
{
	struct Frame
	{
		vk::raii::CommandBuffer command_buffer;
		vk::raii::Fence command_buffer_available_fence;
		vk::raii::Semaphore frame_image_paintable_semaphore;
	};

	class Renderer : public Object
	{
		vk::raii::CommandPool m_command_pool = nullptr;
		std::vector<Frame> m_frames_in_flight;
		Swapchain m_swapchain;
	public :
		Renderer(const EngineContext& engineContext,Swapchain&& swapchain, uint32_t frames_in_flight_count);
		void setNewSwapchain(Swapchain&& swapchain);
		void set_new_frames_in_flight(const EngineContext& engineContext,uint32_t frames_in_flight_count);
		[[nodiscard]] const Swapchain& getSwapchain() const;
		[[nodiscard]] const std::vector<Frame>& get_frames_in_flight() const;

		// void Draw(RenderTarget target,glm::vec4 clearColor);
	};
};