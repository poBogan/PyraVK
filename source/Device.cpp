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
				features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

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

	void Device::constructDeviceHandles()
	{
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

		logicalDeviceHandle = vk::raii::Device(physicalDeviceHandle, deviceInfo);
		graphicsQueueHandle = vk::raii::Queue(logicalDeviceHandle, this->graphics_queue_family_index, 0);
	}

	Device::Device(vk::raii::PhysicalDevice physical_device, uint32_t graphics_queue_family_index, std::vector<const char*>device_extentions)
	{
		physicalDeviceHandle = physical_device;
		this->device_extentions = device_extentions; 
		this->graphics_queue_family_index = graphics_queue_family_index;
		deviceProperties = physicalDeviceHandle.getProperties();
	}

	vk::PhysicalDeviceProperties Device::getDeviceProperties() const
	{
		return deviceProperties;
	}

	uint32_t Device::get_graphics_queue_family_index() const
	{
		return graphics_queue_family_index;
	}

	const std::vector<const char*>& Device::getDeviceExtentions() const
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


}