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
		};
	private : 
		vk::raii::Device logicalDeviceHandle = nullptr; 
		vk::raii::Queue graphicsQueueHandle = nullptr;
		vk::raii::Queue presentationQueueHandle = nullptr;
		vk::raii::CommandPool commandPool = nullptr;
		vk::raii::PipelineCache graphicsPipelineCache = nullptr;
		const Device::Config config;
	public : 
		static std::vector<Device::Config> getSuitableDevices(const VulkanInstance& instance,const Surface& surface);
		Device(const Device::Config config);

		const Device::Config& getConfig() const;
		const vk::raii::Device& getLogicalDeviceHandle() const;
		const vk::raii::Queue& getGraphicsQueueHandle() const;
		const vk::raii::Queue& getPresentationQueueHandle() const;
		const vk::raii::PipelineCache& getGraphicsPipelineCacheHandle() const;
	};

}