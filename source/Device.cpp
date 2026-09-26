#include <PyraVK/Device.h> 

namespace pyra
{
	
	std::vector<Device::Config> Device::getSuitableDevices(const VulkanInstance& instance,const Surface& surface)
	{
		std::vector<Device::Config> suitableDeviceConfigs{};
		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.getVulkanInstanceHandle().enumeratePhysicalDevices();

		for (vk::raii::PhysicalDevice& currentPhysicalDevice : physicalDevices)
		{
			vk::PhysicalDeviceProperties device_properties = currentPhysicalDevice.getProperties();

			bool api_version_supported = device_properties.apiVersion >= instance.getConfig().vulkan_api_version;
			if (!api_version_supported)
			{
				spdlog::error("{:} does not support required api version", device_properties.deviceName.data());
				continue;
			}

			const std::vector<const char*> requiredDeviceExtensions{ vk::KHRSwapchainExtensionName };
			std::vector<vk::ExtensionProperties> supported_device_extentions = currentPhysicalDevice.enumerateDeviceExtensionProperties();

			bool device_extensions_supported = std::ranges::all_of(requiredDeviceExtensions, [&supported_device_extentions,&device_properties](const char* required) -> bool {
				bool extention_supported = std::ranges::any_of(supported_device_extentions, [&required,&device_properties](vk::ExtensionProperties& supported) -> bool {
					return strcmp(required, supported.extensionName.data()) == 0;
					});
				pyra_warn(!extention_supported, "{:} does not support extension \"{:}\"", device_properties.deviceName.data(),required);
				return extention_supported;
				});

			if (!device_extensions_supported)
			{
				spdlog::error("{:} does not support required device extentions!", device_properties.deviceName.data());
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
			int32_t graphicsQueueFamilyIndex = -1, presentQueueFamilyIndex = -1;

			for (int32_t i = 0;i < queueFamilies.size();i++)
			{
				vk::QueueFamilyProperties& currentFamily = queueFamilies[i];
				if (currentFamily.queueFlags & vk::QueueFlagBits::eGraphics)
					graphicsQueueFamilyIndex = i;
				if (currentPhysicalDevice.getSurfaceSupportKHR(i, *surface.getSurfaceHandle()))
					presentQueueFamilyIndex = i;
				if (i == graphicsQueueFamilyIndex == presentQueueFamilyIndex)
					break;
			}

			if (graphicsQueueFamilyIndex == -1 || presentQueueFamilyIndex == -1)
			{
				spdlog::error("{:} does not have suitable queue(s)", device_properties.deviceName.data()); 
				continue;
			}

			suitableDeviceConfigs.push_back({ currentPhysicalDevice,(uint32_t)graphicsQueueFamilyIndex,(uint32_t)presentQueueFamilyIndex });

		}

		return suitableDeviceConfigs;
	}

	Device::Device(const Device::Config config) : config(config)
	{
		bool seperatePresentationQueue = config.graphicsQueueFamilyIndex != config.presentQueueFamilyIndex;

		std::vector<VkDeviceQueueCreateInfo> deviceQueueInfos;

		float queuePriority = 0.5f;
		VkDeviceQueueCreateInfo graphicsQueueInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.queueFamilyIndex = config.graphicsQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};
		deviceQueueInfos.push_back(graphicsQueueInfo);

		if (seperatePresentationQueue)
		{
			VkDeviceQueueCreateInfo presentationQueueInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = NULL,
				.flags = {},
				.queueFamilyIndex = config.presentQueueFamilyIndex,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			};
			deviceQueueInfos.push_back(presentationQueueInfo);
		}

		VkPhysicalDeviceFeatures2 feats;
		auto features = vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
		features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters = true;
		features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true;
		features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;
		features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 = true;

		const std::vector<const char*> requiredDeviceExtensions{ vk::KHRSwapchainExtensionName };

		VkDeviceCreateInfo deviceInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &features.get<vk::PhysicalDeviceFeatures2>(),
			.flags = NULL,
			.queueCreateInfoCount = (uint32_t)deviceQueueInfos.size(),
			.pQueueCreateInfos = deviceQueueInfos.data(),
			.enabledExtensionCount = (uint32_t)requiredDeviceExtensions.size(),
			.ppEnabledExtensionNames = requiredDeviceExtensions.data(),
		};

		logicalDeviceHandle = vk::raii::Device(config.physicalDevice, vk::DeviceCreateInfo(deviceInfo));
		graphicsQueueHandle = vk::raii::Queue(logicalDeviceHandle, config.graphicsQueueFamilyIndex, 0);

		if(seperatePresentationQueue) 
			presentationQueueHandle = vk::raii::Queue(logicalDeviceHandle, config.presentQueueFamilyIndex, 0);

		VkCommandPoolCreateInfo commandPoolInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = config.graphicsQueueFamilyIndex
		};

		commandPool = vk::raii::CommandPool(logicalDeviceHandle, commandPoolInfo);

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

	const vk::raii::Queue& Device::getPresentationQueueHandle() const
	{
		return presentationQueueHandle == nullptr ? graphicsQueueHandle : presentationQueueHandle;
	}

	const vk::raii::PipelineCache& Device::getGraphicsPipelineCacheHandle() const
	{
		return graphicsPipelineCache;
	}
}