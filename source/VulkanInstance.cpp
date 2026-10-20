#include <PyraVK/VulkanInstance.h>
#include <PyraVK/Device.h>
#include <PyraVk/VulkanSurface.h>

namespace pyra
{
	VulkanContext::VulkanContext()
	{
		m_max_supported_api_version = m_context.enumerateInstanceVersion();

		for (const auto extensions = m_context.enumerateInstanceExtensionProperties(); const auto& extension : extensions)
		{
			m_instance_extensions.insert(std::string(extension.extensionName));
		}

		for (const auto layers = m_context.enumerateInstanceLayerProperties(); const auto& layer : layers)
		{
			m_instance_layers.insert(std::string(layer.layerName));
		}
	}

	bool VulkanContext::api_version_supported(const uint32_t api_version) const
	{
		return m_max_supported_api_version >= api_version;
	}

	bool VulkanContext::instance_extension_supported(const char* extension_name) const
	{
		return m_instance_extensions.contains(extension_name);
	}

	bool VulkanContext::instance_layer_supported(const char* layer_name) const
	{
		return m_instance_layers.contains(layer_name);
	}

	const vk::raii::Context& VulkanContext::get_context() const
	{
		return m_context;
	}

	const VulkanContext& VulkanInstance::getVulkanContext() const
	{
		return m_context;
	}

	const vk::raii::Instance& VulkanInstance::get_instance() const
	{
		return m_instance;
	}

	VulkanInstance::VulkanInstance(const std::vector<const char*>& extensions,const std::vector<const char*>& layers)
	{
		pyra_assert(m_context.api_version_supported(VulkanInstance::vulkan_api_version));

		for (const auto& extension : extensions)
			pyra_assert(m_context.instance_extension_supported(extension));

		for (const auto& layer : layers)
			pyra_assert(m_context.instance_layer_supported(layer));

		vk::ApplicationInfo appInfo;
		appInfo.pApplicationName = "PyraVK App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
		appInfo.pEngineName = "PyraVK Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
		appInfo.apiVersion = VulkanInstance::vulkan_api_version;

		vk::InstanceCreateInfo instanceInfo;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		instanceInfo.ppEnabledLayerNames = layers.data();
		instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instanceInfo.ppEnabledExtensionNames = extensions.data();

		m_instance = vk::raii::Instance(m_context.get_context(), vk::InstanceCreateInfo(instanceInfo));
	}

	std::vector<DeviceConfig> VulkanInstance::get_suitable_devices(const VulkanSurface& surface) const
	{
		std::vector<DeviceConfig> suitableDeviceConfigs{};

		for (std::vector<vk::raii::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices(); const vk::raii::PhysicalDevice& currentPhysicalDevice : physicalDevices)
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


}