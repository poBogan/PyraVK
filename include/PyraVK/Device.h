#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Surface.h>

namespace pyra
{

	class Device : public Base
	{
	private : 
		vk::raii::PhysicalDevice physicalDeviceHandle = nullptr;
		vk::raii::Device logicalDeviceHandle = nullptr; 

		vk::raii::Queue graphicsQueueHandle = nullptr;
		vk::raii::CommandPool commandPool = nullptr;
		vk::raii::CommandBuffers commandBuffers = nullptr;
		std::vector<vk::raii::Fence> commandBufferAvailableFences{}; 
		std::vector<vk::raii::Semaphore> targetImagePaintableSemaphores{};
		uint32_t frames_in_flight_ = 0;

		std::vector<const char*>device_extentions;
		uint32_t graphics_queue_family_index = 0;
		vk::PhysicalDeviceProperties deviceProperties;

		Device(vk::raii::PhysicalDevice physical_device, uint32_t graphics_queue_family_index, std::vector<const char*>device_extentions);
	public : 
		static std::vector<Device> getSuitableDevices(VulkanInstance& instance, Surface& surface, std::vector<const char*> device_extentions);

		void constructDeviceHandles();
		void constructCommandBuffers(uint32_t frames_in_flight);

		vk::PhysicalDeviceProperties getDeviceProperties() const;
		uint32_t get_graphics_queue_family_index() const;
		const std::vector<const char*>& getDeviceExtentions() const;

		const vk::raii::PhysicalDevice& getPhysicalDeviceHandle() const; 
		const vk::raii::Device& getLogicalDeviceHandle() const;
		const vk::raii::Queue& getGraphicsQueueHandle() const;
		const vk::raii::CommandBuffers& getCommandBuffers() const;
		const std::vector<vk::raii::Fence>& getCommandBufferAvailableFences() const; 
		const std::vector<vk::raii::Semaphore>& getTargetImagePaintableSemaphores() const;
		const uint32_t getFramesInFlightCount() const;
	};

}