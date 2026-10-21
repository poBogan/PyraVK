#include <PyraVK/Device.h> 
#include <PyraVK/VulkanInstance.h>

#include "PyraVK/VulkanSurface.h"

namespace pyra
{

	Device::Device(const DeviceConfig& config) : PyraBase()
	{
		m_physical_device = config.c_physical_device;
		m_queue_family_index = config.c_queue_family_index;

		float queuePriority = 0.50f;
		vk::DeviceQueueCreateInfo graphicsQueueInfo;
		graphicsQueueInfo.queueFamilyIndex = m_queue_family_index;
		graphicsQueueInfo.queueCount = 1;
		graphicsQueueInfo.pQueuePriorities = &queuePriority;

		auto features = vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
		features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters = true;
		features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true;
		features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;
		features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 = true;

		vk::DeviceCreateInfo deviceInfo;
		deviceInfo.pNext = &features.get<vk::PhysicalDeviceFeatures2>();
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pQueueCreateInfos = &graphicsQueueInfo;
		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size());
		deviceInfo.ppEnabledExtensionNames = required_device_extensions.data();

		m_logical_device = vk::raii::Device(m_physical_device, deviceInfo);
		m_graphics_queue = vk::raii::Queue(m_logical_device, m_queue_family_index, 0);
	}

	const vk::raii::PhysicalDevice& Device::get_physical_device() const
	{
		return m_physical_device;
	}

	const vk::raii::Device& Device::get_logical_device() const
	{
		return m_logical_device;
	}

	const vk::raii::Queue& Device::get_graphics_queue() const
	{
		return m_graphics_queue;
	}

	const uint32_t& Device::get_queue_family_index() const
	{
		return m_queue_family_index;
	}

	bool Device::surface_is_compatible(const VulkanSurface& surface) const
	{
		bool result = true;
		result = result && m_physical_device.getSurfaceSupportKHR(m_queue_family_index, surface.get_surface());
		result = result && !m_physical_device.getSurfaceFormatsKHR(surface.get_surface()).empty();
		return result;
	}
}
