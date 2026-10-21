#pragma once
#include <PyraVK/PyraBase.h>

namespace pyra
{
	struct DeviceConfig
	{
		vk::raii::PhysicalDevice c_physical_device = nullptr;
		uint32_t c_queue_family_index = 0;
	};

	class Device : public PyraBase
	{
		vk::raii::PhysicalDevice m_physical_device = nullptr;
		vk::raii::Device m_logical_device = nullptr;
		vk::raii::Queue m_device_queue = nullptr;
		vma::raii::Allocator m_allocator = nullptr;
		uint32_t m_queue_family_index = 0;
	public :
		static inline const std::vector<const char*> required_device_extensions{ vk::KHRSwapchainExtensionName, vk::EXTMemoryBudgetExtensionName };
		static std::vector<DeviceConfig> GetSuitableDevices(const class VulkanInstance& instance, const class VulkanSurface& surface);

		 Device(const class VulkanInstance& instance, const DeviceConfig& config);
		[[nodiscard]] const vk::raii::PhysicalDevice& get_physical_device() const;
		[[nodiscard]] const vk::raii::Device& get_logical_device() const;
		[[nodiscard]] const vk::raii::Queue& get_device_queue() const;
		[[nodiscard]] const uint32_t& get_queue_family_index() const;
		[[nodiscard]] const vma::raii::Allocator& get_allocator() const;
	};

}