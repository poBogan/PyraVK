#include <PyraVK/VulkanInstance.h>


namespace pyra
{
	VulkanInstance::VulkanInstance(const VulkanInstance::Config& config) : Object()
	{
		vk::ApplicationInfo appInfo;
		appInfo.pApplicationName = "PyraVK App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
		appInfo.pEngineName = "PyraVK Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
		appInfo.apiVersion = config.vulkan_api_version;

		vk::InstanceCreateInfo instanceInfo;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledLayerCount = static_cast<uint32_t>(config.layers.size());
		instanceInfo.ppEnabledLayerNames = config.layers.data();
		instanceInfo.enabledExtensionCount = static_cast<uint32_t>(config.extensions.size());
		instanceInfo.ppEnabledExtensionNames = config.extensions.data();

		instance = vk::raii::Instance(context, vk::InstanceCreateInfo(instanceInfo));
	}

	const vk::raii::Instance& VulkanInstance::getInstance() const
	{
		return instance;
	}

	const VulkanInstance::Config& VulkanInstance::getConfig() const
	{
		return config;
	}
}