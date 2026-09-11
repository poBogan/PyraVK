#include <PyraVK/Definitions.h>
#include <PyraVK/VulkanInstance.h>


namespace pyra
{

	VulkanInstance::VulkanInstance(std::vector<const char*> extentions, std::vector<const char*> layers, uint32_t vulkan_version)
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

		this->instance_obj = vk::raii::Instance(context, instanceInfo);
	}

	bool VulkanInstance::extentionsSupported(std::vector<const char*> extentions)
	{
		vk::raii::Context tempContext;
		bool result = true;
		auto supported_extentions = tempContext.enumerateInstanceExtensionProperties();
		for (auto& required : extentions)
		{
			bool required_is_supported = std::ranges::any_of(supported_extentions, [&required](vk::ExtensionProperties& supported) -> bool {
				return strcmp(required, supported.extensionName.data()) == 0;
				});
			if (!required_is_supported)
			{
				result = false;
				pyra_log_warning("Instance extention \"{:}\" not supported", required);
			}
		}

		return result;
	}

	bool VulkanInstance::layersSupported(std::vector<const char*> layers)
	{
		vk::raii::Context tempContext;
		bool result = true;
		auto supported_layers = tempContext.enumerateInstanceLayerProperties();
		for (auto& required : layers)
		{
			bool required_is_supported = std::ranges::any_of(supported_layers, [&required](vk::LayerProperties& supported) -> bool {
				return strcmp(required, supported.layerName.data()) == 0;
				});
			if (!required_is_supported)
			{
				result = false;
				pyra_log_warning("Instance layer \"{:}\" not supported", required);
			}
		}

		return result;
	}

	std::unique_ptr<VulkanInstance> VulkanInstance::createVulkanInstance(std::vector<const char*> extentions, std::vector<const char*> layers, uint32_t vulkan_version)
	{
		return std::unique_ptr<VulkanInstance>(new VulkanInstance(std::move(extentions), std::move(layers), vulkan_version));
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

	const vk::raii::Instance& VulkanInstance::getInstanceObject() const
	{
		return this->instance_obj;
	}

}