#include <PyraVK/Device.h> 

namespace pyra
{
	
	std::vector<Device> Device::getSuitableDevices(VulkanInstance& instance, Surface& surface, std::vector<const char*> device_extentions)
	{
		std::vector<Device> suitableRenderDevices{};
		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.getVulkanInstanceHandle().enumeratePhysicalDevices();

		for (vk::raii::PhysicalDevice& physical_device : physicalDevices)
		{
			vk::PhysicalDeviceProperties device_properties = physical_device.getProperties();

			bool api_version_supported = device_properties.apiVersion >= instance.getVulkanVersion();

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
				suitableRenderDevices.push_back(Device(physical_device, queue_family_index, device_extentions));
			}

		}

		return suitableRenderDevices;
	}

	Device::Device(vk::raii::PhysicalDevice physical_device, uint32_t graphics_queue_family_index, std::vector<const char*>device_extentions)
	{
		physicalDeviceHandle = physical_device;
		this->device_extentions = device_extentions;
		this->graphicsQueueFamilyIndex = graphics_queue_family_index;
		deviceProperties = physicalDeviceHandle.getProperties();
	}

	void Device::constructDeviceHandles()
	{
		if (logicalDeviceHandle != nullptr)
		{
			spdlog::warn("{:} - Device handles already constructed. Skipping...", __FUNCTION__);
			return;
		}

		float queuePriority = 0.5f;
		VkDeviceQueueCreateInfo queueInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.queueFamilyIndex = this->graphicsQueueFamilyIndex,
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
			.enabledExtensionCount = (uint32_t)this->device_extentions.size(),
			.ppEnabledExtensionNames = this->device_extentions.data(),
		};

		logicalDeviceHandle = vk::raii::Device(physicalDeviceHandle, vk::DeviceCreateInfo(deviceInfo));
		graphicsQueueHandle = vk::raii::Queue(logicalDeviceHandle, this->graphicsQueueFamilyIndex, 0);

		VkCommandPoolCreateInfo commandPoolInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = NULL,
			.flags = {},
			.queueFamilyIndex = graphicsQueueFamilyIndex
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


	vk::PhysicalDeviceProperties Device::getDeviceProperties() const
	{
		return deviceProperties;
	}

	uint32_t Device::getGraphicsQueueFamilyIndex() const
	{
		return graphicsQueueFamilyIndex;
	}

	std::vector<const char*> Device::getDeviceExtentions() const
	{
		return device_extentions;
	}

	const vk::raii::PhysicalDevice& Device::getPhysicalDeviceHandle() const
	{
		return physicalDeviceHandle;
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