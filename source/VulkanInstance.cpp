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

	bool VulkanContext::api_version_supported() const
	{
		return m_max_supported_api_version >= VulkanInstance::vulkan_api_version;
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
		pyra_assert(m_context.api_version_supported());

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

}