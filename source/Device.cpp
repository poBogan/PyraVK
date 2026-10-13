#include <PyraVK/Device.h> 
#include <PyraVK/VulkanInstance.h>
#include <PyraVK/Surface.h>
#include <PyraVK/Renderer.h>

namespace pyra
{

	const std::vector<Device::Config> Device::getSuitableDevices(const VulkanInstance& instance,const Surface& surface)
	{
		std::vector<Device::Config> suitableDeviceConfigs{};
		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.getInstance().enumeratePhysicalDevices();

		for (vk::raii::PhysicalDevice currentPhysicalDevice : physicalDevices)
		{
			vk::PhysicalDeviceProperties device_properties = currentPhysicalDevice.getProperties();

			bool api_version_supported = device_properties.apiVersion >= instance.getConfig().vulkan_api_version;
			if (!api_version_supported)
			{
				spdlog::error("{:} does not support required api version", device_properties.deviceName.data());
				continue;
			}

			std::vector<vk::ExtensionProperties> supported_device_extentions = currentPhysicalDevice.enumerateDeviceExtensionProperties();

			bool device_extensions_supported = std::ranges::all_of(requiredDeviceExtensions, [&supported_device_extentions,&device_properties](const char* required) -> bool {
				bool extention_supported = std::ranges::any_of(supported_device_extentions, [&required,&device_properties](vk::ExtensionProperties& supported) -> bool {
					return strcmp(required, supported.extensionName.data()) == 0;
					});

				if(!extention_supported) spdlog::warn("{:} does not support extension \"{:}\"", device_properties.deviceName.data(),required);

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
			int32_t graphicsQueueFamilyIndex = -1;

			for (int32_t i = 0;i < queueFamilies.size();i++)
			{
				vk::QueueFamilyProperties& currentFamily = queueFamilies[i];
				if ((currentFamily.queueFlags & vk::QueueFlagBits::eGraphics) && currentPhysicalDevice.getSurfaceSupportKHR(i, surface.getSurface()))
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

			Device::Config config{ .physicalDevice = currentPhysicalDevice, .queueFamilyIndex = (uint32_t)graphicsQueueFamilyIndex };
			suitableDeviceConfigs.push_back(config);

		}

		return suitableDeviceConfigs;
	}

	Device::Device(const Device::Config config) : Object(config, Object::Type::RENDER_DEVICE)
	{
		physicalDevice = config.physicalDevice;
		queueFamilyIndex = config.queueFamilyIndex;

		float queuePriority = 0.50f;
		VkDeviceQueueCreateInfo graphicsQueueInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.queueFamilyIndex = queueFamilyIndex,
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
			.pQueueCreateInfos = &graphicsQueueInfo,
			.enabledExtensionCount = (uint32_t)requiredDeviceExtensions.size(),
			.ppEnabledExtensionNames = requiredDeviceExtensions.data(),
		};

		logicalDevice = vk::raii::Device(physicalDevice, vk::DeviceCreateInfo(deviceInfo));
		graphicsQueue = vk::raii::Queue(logicalDevice, queueFamilyIndex, 0);

	}

	const vk::raii::PhysicalDevice& Device::getPhysicalDevice() const
	{
		return physicalDevice; 
	}
	const vk::raii::Device& Device::getLogicalDevice() const
	{
		return logicalDevice;
	}
	const vk::raii::Queue& Device::getGraphicsQueue() const
	{
		return graphicsQueue;
	}
	const uint32_t& Device::getQueueFamilyIndex() const
	{
		return queueFamilyIndex;
	}
}