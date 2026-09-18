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
		vk::raii::CommandPool commandPoolHandle = nullptr;

		vk::raii::PipelineCache graphicsPipelineCache = nullptr;

		std::vector<const char*>device_extentions;
		uint32_t graphicsQueueFamilyIndex = 0;
		vk::PhysicalDeviceProperties deviceProperties;

		Device(vk::raii::PhysicalDevice physical_device, uint32_t graphics_queue_family_index, std::vector<const char*>device_extentions);
	public : 
		static std::vector<Device> getSuitableDevices(VulkanInstance& instance, Surface& surface, std::vector<const char*> device_extentions);

		void constructDeviceHandles();

		vk::PhysicalDeviceProperties getDeviceProperties() const;
		uint32_t getGraphicsQueueFamilyIndex() const;
		std::vector<const char*> getDeviceExtentions() const;

		const vk::raii::PhysicalDevice& getPhysicalDeviceHandle() const; 
		const vk::raii::Device& getLogicalDeviceHandle() const;
		const vk::raii::Queue& getGraphicsQueueHandle() const;
		const vk::raii::CommandPool& getCommandPoolHandle() const;
		const vk::raii::PipelineCache& getGraphicsPipelineCacheHandle() const;
	};

}