#include <PyraVK/RenderDevice.h> 

namespace pyra
{
	std::unique_ptr<RenderDevice> RenderDevice::createRenderDevice(vk::raii::PhysicalDevice physical_device, uint32_t queue_family_index, vk::SurfaceFormatKHR target_surface_format, std::vector<const char*>device_extentions)
	{
		return std::unique_ptr<RenderDevice>(new RenderDevice(physical_device,queue_family_index,target_surface_format,device_extentions));
	}

	std::vector<std::unique_ptr<RenderDevice>> RenderDevice::getSuitableDevices(std::unique_ptr<VulkanInstance>& instance, std::unique_ptr<Surface>& surface, std::vector<const char*> device_extentions)
	{
		std::vector<std::unique_ptr<RenderDevice>> suitableRenderDevices{};
		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance->getInstanceObject().enumeratePhysicalDevices();

		for (vk::raii::PhysicalDevice& physical_device : physicalDevices)
		{
			vk::PhysicalDeviceProperties device_properties = physical_device.getProperties();

			bool api_version_supported = device_properties.apiVersion >= instance->getVulkanVersion();

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
				features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

			bool suitable_queue_found = false;
			auto queue_families = physical_device.getQueueFamilyProperties();

			uint32_t queue_family_index = 0;
			for (queue_family_index = 0;queue_family_index < queue_families.size();queue_family_index++)
			{
				if (((queue_families[queue_family_index].queueFlags & vk::QueueFlagBits::eGraphics) != (vk::QueueFlags)0) && physical_device.getSurfaceSupportKHR(queue_family_index, *surface->getSurfaceObj()))
				{
					suitable_queue_found = true;
					break;
				}
			}

			if (api_version_supported && device_extensions_supported && device_features_supported && suitable_queue_found)
			{
				bool found_compatible = false;
				vk::SurfaceFormatKHR selected_surface_format;
				auto supported_surface_formats = physical_device.getSurfaceFormatsKHR(*surface->getSurfaceObj());
				for (auto prefered : surface->getFormatPreferences())
				{
					if (std::ranges::any_of(supported_surface_formats, [&prefered](vk::SurfaceFormatKHR& supported_format) {
						return supported_format == prefered;
						}))
					{
						found_compatible = true;
						selected_surface_format = prefered;
						break;
					}
				}

				if (!found_compatible)
				{
					selected_surface_format = supported_surface_formats[0];
				}

				suitableRenderDevices.push_back(RenderDevice::createRenderDevice(physical_device, queue_family_index, selected_surface_format, device_extentions));
			}

		}

		return suitableRenderDevices;
	}

	RenderDevice::RenderDevice(vk::raii::PhysicalDevice physical_device, uint32_t graphics_queue_family_index, vk::SurfaceFormatKHR target_surface_format, std::vector<const char*>device_extentions)
	{
		this->physicalDevice = physical_device;
		this->device_extentions = device_extentions; 
		this->graphics_queue_family_index = graphics_queue_family_index;
		this->targetSurfaceFormat = target_surface_format;
		deviceProperties = physicalDevice.getProperties();

		float queuePriority = 0.5f;
		vk::DeviceQueueCreateInfo queueInfo; 
		queueInfo.pQueuePriorities = &queuePriority; 
		queueInfo.queueCount = 1; 
		queueInfo.queueFamilyIndex = this->graphics_queue_family_index;

		auto features = vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
		features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters = true; 
		features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true; 
		features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;

		vk::DeviceCreateInfo deviceInfo; 
		deviceInfo.enabledExtensionCount = (uint32_t)this->device_extentions.size(); 
		deviceInfo.ppEnabledExtensionNames = this->device_extentions.data(); 
		deviceInfo.queueCreateInfoCount = 1; 
		deviceInfo.pQueueCreateInfos = &queueInfo; 
		deviceInfo.pNext = &features.get<vk::PhysicalDeviceFeatures2>();

		logicalDevice = vk::raii::Device(this->physicalDevice, deviceInfo); 
		graphicsQueue = vk::raii::Queue(logicalDevice, this->graphics_queue_family_index, 0);
	}

	vk::PhysicalDeviceProperties RenderDevice::getDeviceProperties() const
	{
		return deviceProperties;
	}

	vk::SurfaceFormatKHR RenderDevice::getTargetSurfaceFormat() const
	{
		return targetSurfaceFormat;
	}

	uint32_t RenderDevice::get_graphics_queue_family_index() const
	{
		return graphics_queue_family_index;
	}

	const std::vector<const char*>& RenderDevice::getDeviceExtentions() const
	{
		return device_extentions;
	}

	const vk::raii::PhysicalDevice& RenderDevice::getPhysicalDeviceHandle() const
	{
		return physicalDevice;
	}

	const vk::raii::Device& RenderDevice::getLogicalDeviceHandle() const
	{
		return logicalDevice;
	}

	const vk::raii::Queue& RenderDevice::getGraphicsQueueHandle() const
	{
		return graphicsQueue;
	}


}