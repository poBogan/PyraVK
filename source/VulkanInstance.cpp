#include <PyraVK/VulkanInstance.h>


namespace pyra
{
	VulkanInstance::VulkanInstance(const vk::raii::Context& context,std::vector<const char*> extentions, std::vector<const char*> layers, uint32_t vulkan_version)
	{
		pyra_assert(vulkan_api_version >= vk::ApiVersion13, "Vulkan version must be equal or above 1.3");

		try 
		{
			this->extentions = std::move(extentions);
			this->layers = std::move(layers);
			this->vulkan_api_version = vulkan_version;

			vk::ApplicationInfo app;
			app.apiVersion = this->vulkan_api_version;
			app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			app.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			app.pApplicationName = "PyraVK";
			app.pEngineName = "PyraVK Renderer";

			vk::InstanceCreateInfo instanceInfo;
			instanceInfo.enabledExtensionCount = (uint32_t)this->extentions.size();
			instanceInfo.ppEnabledExtensionNames = this->extentions.data();
			instanceInfo.enabledLayerCount = (uint32_t)this->layers.size();
			instanceInfo.ppEnabledLayerNames = this->layers.data();
			instanceInfo.pApplicationInfo = &app;

			handle = vk::raii::Instance(context, instanceInfo);
		}
		catch (std::exception& e)
		{
			pyra_assert(0, e.what());
		}
	}

	const std::vector<const char*>& VulkanInstance::getExtensions() const
	{
		return this->extentions;
	}

	const std::vector<const char*>& VulkanInstance::getLayers() const
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