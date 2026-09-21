#include <PyraVK/VulkanInstance.h>


namespace pyra
{
	VulkanInstance::VulkanInstance(const VulkanInstance::Config config) : config(config)
	{
		try 
		{
			VkApplicationInfo appInfo
			{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pNext = NULL,
				.pApplicationName = "PyraVK App",
				.applicationVersion = VK_MAKE_VERSION(1,0,0),
				.pEngineName = "PyraVK Engine",
				.engineVersion = VK_MAKE_VERSION(1,0,0),
				.apiVersion = config.vulkan_api_version
			};

			VkInstanceCreateInfo instanceInfo
			{
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pNext = NULL,
				.flags = NULL,
				.pApplicationInfo = &appInfo,
				.enabledLayerCount = (uint32_t)config.layers.size(),
				.ppEnabledLayerNames = config.layers.data(),
				.enabledExtensionCount = (uint32_t)config.extentions.size(),
				.ppEnabledExtensionNames = config.extentions.data()
			};

			handle = vk::raii::Instance(context, vk::InstanceCreateInfo(instanceInfo));
		}
		catch (std::exception& e)
		{
			pyra_assert(0, e.what());
		}
	}

	const VulkanInstance::Config& VulkanInstance::getConfig() const
	{
		return config;
	}

	const vk::raii::Instance& VulkanInstance::getVulkanInstanceHandle() const
	{
		return handle;
	}

}