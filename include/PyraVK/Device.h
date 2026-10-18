#pragma once
#include <PyraVK/Object.h>

namespace pyra
{

	inline const std::vector<const char*> requiredDeviceExtensions{ vk::KHRSwapchainExtensionName, vk::EXTMemoryBudgetExtensionName };

	class Device : public Object
	{
	public : 
		struct Config
		{
			vk::raii::PhysicalDevice physicalDevice = nullptr;
			uint32_t queueFamilyIndex = 0;
		};
	private : 
		vk::raii::PhysicalDevice physicalDevice = nullptr; 
		vk::raii::Device logicalDevice = nullptr; 
		vk::raii::Queue graphicsQueue = nullptr;
		uint32_t queueFamilyIndex = 0;
	public : 
		static std::vector<Device::Config> getSuitableDevices(const class VulkanInstance& instance, const class Surface& surface);
		explicit Device(const Device::Config& config);

		[[nodiscard]] const vk::raii::PhysicalDevice& getPhysicalDevice() const;
		[[nodiscard]] const vk::raii::Device& getLogicalDevice() const;
		[[nodiscard]] const vk::raii::Queue& getGraphicsQueue() const;
		[[nodiscard]] const uint32_t& getQueueFamilyIndex() const;
	};

}