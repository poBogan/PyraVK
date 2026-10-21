#include <PyraVK/Device.h> 
#include <PyraVK/VulkanInstance.h>

#include "PyraVK/VulkanSurface.h"

namespace pyra
{
	std::vector<DeviceConfig> Device::GetSuitableDevices(const VulkanInstance& instance, const VulkanSurface& surface)
	{
		std::vector<DeviceConfig> suitableDeviceConfigs{};

		for (std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.get_instance().enumeratePhysicalDevices(); const vk::raii::PhysicalDevice& currentPhysicalDevice : physicalDevices)
		{
			vk::PhysicalDeviceProperties device_properties = currentPhysicalDevice.getProperties();

			// Check if device supports the required vulkan api version
			if (device_properties.apiVersion < VulkanInstance::vulkan_api_version)
			{
				spdlog::error("{:} does not support required api version", device_properties.deviceName.data());
				continue;
			}

			std::vector<vk::ExtensionProperties> supported_device_extensions = currentPhysicalDevice.enumerateDeviceExtensionProperties();

			bool device_extensions_supported = std::ranges::all_of(Device::required_device_extensions, [&supported_device_extensions,&device_properties](const char* required) -> bool {
				const bool extension_supported = std::ranges::any_of(supported_device_extensions, [&required](const vk::ExtensionProperties& supported) -> bool {
					return strcmp(required, supported.extensionName.data()) == 0;
					});

				if(!extension_supported) spdlog::warn("{:} does not support extension \"{:}\"", device_properties.deviceName.data(),required);

				return extension_supported;
				});

			if (!device_extensions_supported)
			{
				spdlog::error("{:} does not support required device extensions!", device_properties.deviceName.data());
				continue;
			}

			auto features = currentPhysicalDevice.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

			bool device_features_supported = features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
				features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
				features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState &&
				features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2;

			if (!device_features_supported)
			{
				spdlog::error("{:} does not support required device features",device_properties.deviceName.data());
				continue;
			}

			auto queueFamilies = currentPhysicalDevice.getQueueFamilyProperties();
			int32_t graphicsQueueFamilyIndex = -1;

			for (int32_t i = 0;i < queueFamilies.size();i++)
			{
				vk::QueueFamilyProperties& currentFamily = queueFamilies[i];
				if ((currentFamily.queueFlags & vk::QueueFlagBits::eGraphics) && currentPhysicalDevice.getSurfaceSupportKHR(i, surface.get_surface()))
				{
					graphicsQueueFamilyIndex = i;
					break;
				}
			}

			if (graphicsQueueFamilyIndex == -1)
			{
				spdlog::error("{:} does not have suitable queue", device_properties.deviceName.data());
				continue;
			}

			const auto supported_formats = currentPhysicalDevice.getSurfaceFormatsKHR(surface.get_surface());
			if (supported_formats.empty())
			{
				spdlog::error("{:} does not have compatible surface format", device_properties.deviceName.data());
				continue;
			}

			DeviceConfig config{ .c_physical_device = currentPhysicalDevice, .c_queue_family_index = static_cast<uint32_t>(graphicsQueueFamilyIndex)};
			suitableDeviceConfigs.push_back(config);

		}

		return suitableDeviceConfigs;
	}


	Device::Device(const VulkanInstance& instance, const DeviceConfig& config) : PyraBase()
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
		m_device_queue = vk::raii::Queue(m_logical_device, m_queue_family_index, 0);

		vma::AllocatorCreateInfo allocatorCreateInfo;
		allocatorCreateInfo.physicalDevice = m_physical_device;
		allocatorCreateInfo.flags = vma::AllocatorCreateFlagBits::eExtMemoryBudget;

		m_allocator = vma::raii::Allocator(instance.get_instance(), m_logical_device, allocatorCreateInfo);
	}

	const vk::raii::PhysicalDevice& Device::get_physical_device() const
	{
		return m_physical_device;
	}

	const vk::raii::Device& Device::get_logical_device() const
	{
		return m_logical_device;
	}

	const vk::raii::Queue& Device::get_device_queue() const
	{
		return m_device_queue;
	}

	const uint32_t& Device::get_queue_family_index() const
	{
		return m_queue_family_index;
	}

	const vma::raii::Allocator& Device::get_allocator() const
	{
		return m_allocator;
	}

}
