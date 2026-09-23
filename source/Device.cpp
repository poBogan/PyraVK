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
			pyra_warn(!api_version_supported, "{:} does not support required Vulkan version!", device_properties.deviceName.data());

			std::vector<vk::ExtensionProperties> supported_device_extentions = physical_device.enumerateDeviceExtensionProperties();

			bool device_extensions_supported = std::ranges::all_of(device_extentions, [&supported_device_extentions,&device_properties](const char* required) -> bool {
				bool extention_supported = std::ranges::any_of(supported_device_extentions, [&required,&device_properties](vk::ExtensionProperties& supported) -> bool {
					return strcmp(required, supported.extensionName.data()) == 0;
					});
				pyra_warn(!extention_supported, "{:} does not support extension \"{:}\"", device_properties.deviceName.data(),required);
				return extention_supported;
				});

			auto features = physical_device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

			bool device_features_supported = features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
				features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
				features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState &&
				features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2;

			pyra_warn(!device_features_supported, "{:} does not support required device features!",device_properties.deviceName.data());

			auto queue_families = physical_device.getQueueFamilyProperties();

			bool graphics_queue_found = false;
			uint32_t graphics_queue_family_index = 0;

			for (graphics_queue_family_index = 0;graphics_queue_family_index < queue_families.size();graphics_queue_family_index++)
			{
				if (((queue_families[graphics_queue_family_index].queueFlags & vk::QueueFlagBits::eGraphics) != (vk::QueueFlags)0))
				{
					graphics_queue_found = true;
					break;
				}
			}

			bool presentation_queue_found = false;
			uint32_t presentation_queue_family_index = 0;

			for (;presentation_queue_family_index < queue_families.size();presentation_queue_family_index++)
			{
				if (physical_device.getSurfaceSupportKHR(presentation_queue_family_index, *surface.getSurfaceHandle()))
				{
					presentation_queue_found = true; 
					break;
				}
			}

			if (api_version_supported && 
				device_extensions_supported && 
				device_features_supported && 
				graphics_queue_found && 
				presentation_queue_found
				)
			{
				suitableDeviceConfigs.push_back(Device::Config{ .physicalDevice = physical_device,.graphicsQueueFamilyIndex = graphics_queue_family_index,.presentQueueFamilyIndex = presentation_queue_family_index,.device_extentions = device_extentions });
			}

		}

		return suitableDeviceConfigs;
	}

	Device::Device(const Device::Config config) : config(config)
	{
		float queuePriority = 0.5f;
		VkDeviceQueueCreateInfo graphicsQueueInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.queueFamilyIndex = config.graphicsQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		VkDeviceQueueCreateInfo presentationQueueInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = {},
			.queueFamilyIndex = config.presentQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		std::array<VkDeviceQueueCreateInfo, 2> deviceQueueCreateInfos{ graphicsQueueInfo,presentationQueueInfo };

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
			.queueCreateInfoCount = 2,
			.pQueueCreateInfos = deviceQueueCreateInfos.data(),
			.enabledExtensionCount = (uint32_t)config.device_extentions.size(),
			.ppEnabledExtensionNames = config.device_extentions.data(),
		};

		logicalDeviceHandle = vk::raii::Device(config.physicalDevice, vk::DeviceCreateInfo(deviceInfo));
		graphicsQueueHandle = vk::raii::Queue(logicalDeviceHandle, config.graphicsQueueFamilyIndex, 0);
		presentationQueueHandle = vk::raii::Queue(logicalDeviceHandle, config.presentQueueFamilyIndex, 0);

		VkCommandPoolCreateInfo commandPoolInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = config.graphicsQueueFamilyIndex
		};

		VkPipelineCacheCreateInfo graphicsPipelineCacheInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			.pNext = NULL,
			.flags = {},
			.initialDataSize = 0,
			.pInitialData = NULL
		};

		graphicsPipelineCache = vk::raii::PipelineCache(logicalDeviceHandle, graphicsPipelineCacheInfo);

		commandPoolHandle = vk::raii::CommandPool(logicalDeviceHandle, vk::CommandPoolCreateInfo(commandPoolInfo));

		VkCommandBufferAllocateInfo cmdBuffAllInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = NULL,
			.commandPool = (VkCommandPool)*commandPoolHandle,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = config.framesInFlight
		};

		commandBuffers = vk::raii::CommandBuffers(logicalDeviceHandle, vk::CommandBufferAllocateInfo(cmdBuffAllInfo));

		VkFenceCreateInfo fenceCreateInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = NULL,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		VkSemaphoreCreateInfo semaphoreCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = NULL,
			.flags = {}
		};

		for (int i = 0;i < config.framesInFlight; i++)
		{
			commandBufferReadyFences.push_back(vk::raii::Fence(logicalDeviceHandle, vk::FenceCreateInfo(fenceCreateInfo)));
			imagePaintableSemaphores.push_back(vk::raii::Semaphore(logicalDeviceHandle, vk::SemaphoreCreateInfo(semaphoreCreateInfo)));
		}
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
		return presentationQueueHandle;
	}

	const vk::raii::CommandPool& Device::getCommandPoolHandle() const
	{
		return commandPoolHandle;
	}

	const vk::raii::PipelineCache& Device::getGraphicsPipelineCacheHandle() const
	{
		return graphicsPipelineCache;
	}

	const vk::raii::CommandBuffers& Device::getCommandBuffers() const
	{
		return commandBuffers;
	}

	const std::vector<vk::raii::Fence>& Device::getCommandBufferReadyFences() const
	{
		return commandBufferReadyFences;
	}

	const std::vector<vk::raii::Semaphore>& Device::getImagePaintableSemaphores() const
	{
		return imagePaintableSemaphores;
	}

}