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
			vk::raii::PhysicalDevice physicalDevice;
			uint32_t graphicsQueueFamilyIndex;
			std::vector<const char*>device_extentions;
		};
	private : 
		vk::raii::Device logicalDeviceHandle = nullptr; 
		vk::raii::Queue graphicsQueueHandle = nullptr;
		vk::raii::CommandPool commandPoolHandle = nullptr;
		vk::raii::PipelineCache graphicsPipelineCache = nullptr;
		const Device::Config config;
	public : 
		static std::vector<Device::Config> getSuitableDevices(const VulkanInstance& instance,const Surface& surface,const std::vector<const char*> device_extentions);
		Device(const Device::Config config);

		const Device::Config& getConfig() const;
		const vk::raii::Device& getLogicalDeviceHandle() const;
		const vk::raii::Queue& getGraphicsQueueHandle() const;
		const vk::raii::CommandPool& getCommandPoolHandle() const;
		const vk::raii::PipelineCache& getGraphicsPipelineCacheHandle() const;
	};

}