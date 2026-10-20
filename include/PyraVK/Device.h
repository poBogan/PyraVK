#pragma once
#include <PyraVK/Object.h>

namespace pyra
{

	struct DeviceConfig
	{
		vk::raii::PhysicalDevice c_physical_device = nullptr;
		uint32_t c_queue_family_index = 0;
	};

	class Device : public Object
	{
		vk::raii::PhysicalDevice m_physical_device = nullptr;
		vk::raii::Device m_logical_device = nullptr;
		vk::raii::Queue m_graphics_queue = nullptr;
		uint32_t m_queue_family_index = 0;
	public :
		static inline const std::vector<const char*> required_device_extensions{ vk::KHRSwapchainExtensionName, vk::EXTMemoryBudgetExtensionName };

		explicit Device(const DeviceConfig& config);
		[[nodiscard]] const vk::raii::PhysicalDevice& get_physical_device() const;
		[[nodiscard]] const vk::raii::Device& get_logical_device() const;
		[[nodiscard]] const vk::raii::Queue& get_graphics_queue() const;
		[[nodiscard]] const uint32_t& get_queue_family_index() const;
		[[nodiscard]] bool surface_is_compatible(const class VulkanSurface& surface) const;
	};

}