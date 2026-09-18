#include <PyraVK/VulkanInstance.h>


namespace pyra
{
	VulkanInstance::VulkanInstance(const vk::raii::Context& context,std::vector<const char*> extentions, std::vector<const char*> layers)
	{
		try 
		{
			this->extentions = std::move(extentions);
			this->layers = std::move(layers);

			VkApplicationInfo appInfo
			{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pNext = NULL,
				.pApplicationName = "PyraVK App",
				.applicationVersion = VK_MAKE_VERSION(1,0,0),
				.pEngineName = "PyraVK Engine",
				.engineVersion = VK_MAKE_VERSION(1,0,0),
				.apiVersion = vulkan_api_version
			};

			VkInstanceCreateInfo instanceInfo
			{
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
				.pNext = NULL,
				.flags = NULL,
				.pApplicationInfo = &appInfo,
				.enabledLayerCount = (uint32_t)this->layers.size(),
				.ppEnabledLayerNames = this->layers.data(),
				.enabledExtensionCount = (uint32_t)this->extentions.size(),
				.ppEnabledExtensionNames = this->extentions.data()
			};

			handle = vk::raii::Instance(context, vk::InstanceCreateInfo(instanceInfo));
		}
		catch (std::exception& e)
		{
			pyra_assert(0, e.what());
		}
	}

	std::vector<const char*> VulkanInstance::getExtentionNames() const
	{
		return this->extentions;
	}

	std::vector<const char*> VulkanInstance::getLayerNames() const
	{
		return this->layers;
	}

	uint32_t VulkanInstance::getVulkanVersion() const
	{
		return this->vulkan_api_version;
	}

	const vk::raii::Instance& VulkanInstance::getVulkanInstanceHandle() const
	{
		return handle;
	}

}