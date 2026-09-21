#include <PyraVK/Device.h> 

namespace pyra
{
	
	std::vector<Device::Config> Device::getSuitableDevices(const VulkanInstance& instance,const Surface& surface,const std::vector<const char*> device_extentions)
	{
		std::vector<Device::Config> suitableDeviceConfigs{};
		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.getVulkanInstanceHandle().enumeratePhysicalDevices();

		for (vk::raii::PhysicalDevice& physical_device : physicalDevices)
		{
			vk::PhysicalDeviceProperties device_properties = physical_device.getProperties();

			bool api_version_supported = device_properties.apiVersion >= instance.getConfig().vulkan_api_version;

			std::vector<vk::ExtensionProperties> supported_device_extentions = physical_device.enumerateDeviceExtensionProperties();

			bool device_extensions_supported = std::ranges::all_of(device_extentions, [&supported_device_extentions](const char* required) -> bool {
				bool extention_supported = std::ranges::any_of(supported_device_extentions, [&required](vk::ExtensionProperties& supported) -> bool {
					return strcmp(required, supported.extensionName.data()) == 0;
					});
				return extention_supported;
				});

			auto features = physical_device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

			bool device_features_supported = features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
				features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
				features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState &&
				features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2;

			bool suitable_queue_found = false;
			auto queue_families = physical_device.getQueueFamilyProperties();
			uint32_t queue_family_index = 0;

			for (queue_family_index = 0;queue_family_index < queue_families.size();queue_family_index++)
			{
				if (((queue_families[queue_family_index].queueFlags & vk::QueueFlagBits::eGraphics) != (vk::QueueFlags)0) && physical_device.getSurfaceSupportKHR(queue_family_index, *surface.getSurfaceHandle()))
				{
					suitable_queue_found = true;
					break;
				}
			}

			auto suitable_surface_format_exists = physical_device.getSurfaceFormatsKHR(*surface.getSurfaceHandle()).size() > 0;

			if (api_version_supported && 
				device_extensions_supported && 
				device_features_supported && 
				suitable_queue_found && 
				suitable_surface_format_exists
				)
			{
				suitableDeviceConfigs.push_back({ physical_device,queue_family_index,device_extentions });
			}

		}

		return suitableDeviceConfigs;
	}

	Device::Device(const Device::Config config) : config(config)
	{
		float queuePriority = 0.5f;
		VkDeviceQueueCreateInfo queueInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.queueFamilyIndex = config.graphicsQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		VkPhysicalDeviceFeatures2 feats;
		auto features = vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
		features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters = true;
		features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true;
		features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;
		features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 = true;

		VkDeviceCreateInfo deviceInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features.get<vk::PhysicalDeviceFeatures2>(),
			.flags = NULL,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueInfo,
			.enabledExtensionCount = (uint32_t)config.device_extentions.size(),
			.ppEnabledExtensionNames = config.device_extentions.data(),
		};

		logicalDeviceHandle = vk::raii::Device(config.physicalDevice, vk::DeviceCreateInfo(deviceInfo));
		graphicsQueueHandle = vk::raii::Queue(logicalDeviceHandle, config.graphicsQueueFamilyIndex, 0);

		VkCommandPoolCreateInfo commandPoolInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = config.graphicsQueueFamilyIndex
		};

		commandPoolHandle = vk::raii::CommandPool(logicalDeviceHandle, vk::CommandPoolCreateInfo(commandPoolInfo));

		VkPipelineCacheCreateInfo graphicsPipelineCacheInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			.pNext = NULL,
			.flags = {},
			.initialDataSize = 0,
			.pInitialData = NULL
		};

		graphicsPipelineCache = vk::raii::PipelineCache(logicalDeviceHandle, graphicsPipelineCacheInfo);
	}

	const Device::Config& Device::getConfig() const
	{
		return config;
	}

	const vk::raii::Device& Device::getLogicalDeviceHandle() const
	{
		return logicalDeviceHandle;
	}

	const vk::raii::Queue& Device::getGraphicsQueueHandle() const
	{
		return graphicsQueueHandle;
	}

	const vk::raii::CommandPool& Device::getCommandPoolHandle() const
	{
		return commandPoolHandle;
	}

	const vk::raii::PipelineCache& Device::getGraphicsPipelineCacheHandle() const
	{
		return graphicsPipelineCache;
	}

}