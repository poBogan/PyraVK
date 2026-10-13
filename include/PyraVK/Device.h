#pragma once
#include <PyraVK/Object.h>

namespace pyra
{

	inline const std::vector<const char*> requiredDeviceExtensions{ vk::KHRSwapchainExtensionName, vk::EXTMemoryBudgetExtensionName };

	class Device : public Object
	{
	public : 
		struct Config : Object::Config
		{
			vk::raii::PhysicalDevice physicalDevice;
			uint32_t queueFamilyIndex;
		};
	private : 
		vk::raii::PhysicalDevice physicalDevice = nullptr; 
		vk::raii::Device logicalDevice = nullptr; 
		vk::raii::Queue graphicsQueue = nullptr;
		uint32_t queueFamilyIndex;
	public : 
		static const std::vector<Device::Config> getSuitableDevices(const class VulkanInstance& instance, const class Surface& surface);
		Device(const Device::Config config);

		const vk::raii::PhysicalDevice& getPhysicalDevice() const; 
		const vk::raii::Device& getLogicalDevice() const;
		const vk::raii::Queue& getGraphicsQueue() const;
		const uint32_t& getQueueFamilyIndex() const;
	};

}