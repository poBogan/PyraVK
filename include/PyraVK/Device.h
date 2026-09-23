#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Surface.h>

namespace pyra
{

	class Device : public Base
	{
	public : 
		struct Config
		{
			const vk::raii::PhysicalDevice physicalDevice;
			const uint32_t graphicsQueueFamilyIndex;
			const uint32_t presentQueueFamilyIndex;
			const std::vector<const char*>device_extentions;
			uint32_t framesInFlight = 2;
		};
	private : 
		vk::raii::Device logicalDeviceHandle = nullptr; 
		vk::raii::Queue graphicsQueueHandle = nullptr;
		vk::raii::Queue presentationQueueHandle = nullptr;

		vk::raii::PipelineCache graphicsPipelineCache = nullptr;

		vk::raii::CommandPool commandPoolHandle = nullptr;
		vk::raii::CommandBuffers commandBuffers = nullptr;
		std::vector<vk::raii::Fence> commandBufferReadyFences{};
		std::vector<vk::raii::Semaphore> imagePaintableSemaphores{};

		const Device::Config config;
	public : 
		static std::vector<Device::Config> getSuitableDevices(const VulkanInstance& instance,const Surface& surface,const std::vector<const char*> device_extentions);
		Device(const Device::Config config);

		const Device::Config& getConfig() const;
		const vk::raii::Device& getLogicalDeviceHandle() const;
		const vk::raii::Queue& getGraphicsQueueHandle() const;
		const vk::raii::Queue& getPresentationQueueHandle() const;
		const vk::raii::CommandPool& getCommandPoolHandle() const;
		const vk::raii::PipelineCache& getGraphicsPipelineCacheHandle() const;
		const vk::raii::CommandBuffers& getCommandBuffers() const;
		const std::vector<vk::raii::Fence>& getCommandBufferReadyFences() const; 
		const std::vector<vk::raii::Semaphore>& getImagePaintableSemaphores() const;
	};

}