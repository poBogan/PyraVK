#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Surface.h>
#include <vk_mem_alloc_raii.hpp>

namespace pyra
{

	class Device : public Base
	{
	public : 
		struct Config
		{
			const vk::raii::PhysicalDevice physicalDevice;
			const uint32_t graphicsQueueFamilyIndex;
		};
	private : 
		vk::raii::Device logicalDeviceHandle = nullptr; 
		vk::raii::Queue graphicsQueueHandle = nullptr;
		vk::raii::CommandPool commandPoolHandle = nullptr;
		vk::raii::PipelineCache graphicsPipelineCache = nullptr;
		vma::raii::Allocator deviceMemoryAllocator = nullptr;
		const Device::Config config;
	public : 
		static std::vector<Device::Config> getSuitableDevices(const VulkanInstance& instance,const Surface& surface);
		Device(const Device::Config config);

		const Device::Config& getConfig() const;
		const vk::raii::Device& getLogicalDeviceHandle() const;
		const vk::raii::Queue& getGraphicsQueueHandle() const;
		const vk::raii::CommandPool& getCommandPoolHandle() const;
		const vk::raii::PipelineCache& getGraphicsPipelineCacheHandle() const;
		const vma::raii::Allocator& getDeviceMemoryAllocator() const;
	};

}