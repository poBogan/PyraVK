#include <PyraVK/Renderer.h>
#include <spdlog/spdlog.h>
#include <iostream>
#include <algorithm>
#include <ranges>
#include <vector>

namespace pyra
{

	Renderer* Renderer::singletonRenderer = nullptr;

	void Renderer::createRenderer(std::vector<const char*> required_instance_extentions)
	{
		if (singletonRenderer == nullptr)
		{
			singletonRenderer = new Renderer(required_instance_extentions.data(),(uint32_t)required_instance_extentions.size());
		}
	}

	Renderer* Renderer::getRenderer()
	{
		return singletonRenderer;
	}

	Renderer::Renderer(const char* const* required_instance_extentions, uint32_t instance_extention_count)
	{
		// Check if required instance extentions are supported 
		std::vector<const char*> requiredInstanceExtentions(required_instance_extentions, required_instance_extentions + instance_extention_count);
		std::vector<vk::ExtensionProperties> supportedInstanceExtentions(vk::enumerateInstanceExtensionProperties());

		bool instanceExtentionsSupported = std::ranges::all_of(requiredInstanceExtentions, [&supportedInstanceExtentions](const char* required_extention) -> bool {
			bool extentionSupported = std::ranges::any_of(supportedInstanceExtentions, [&required_extention](const vk::ExtensionProperties supported_extention) -> bool {
				return strcmp(required_extention, supported_extention.extensionName.data()) == 0;
				});

			extentionSupported ? spdlog::info("{:<30} - supported", required_extention) : spdlog::error("{:<30} - not supported", required_extention);
			return extentionSupported;
			});

		// Check if required instance layers are supported 
		std::vector<const char*> requiredInstanceLayers({ "VK_LAYER_KHRONOS_validation"});
		std::vector<vk::LayerProperties> supportedInstanceLayers(vk::enumerateInstanceLayerProperties());

		bool instanceLayersSupported = std::ranges::all_of(requiredInstanceLayers, [&supportedInstanceLayers](const char* requiredLayer) -> bool {
			bool layerSupported = std::ranges::any_of(supportedInstanceLayers, [&requiredLayer](const vk::LayerProperties& supportedLayer) -> bool {
				return strcmp(requiredLayer, supportedLayer.layerName.data()) == 0;
				});
			layerSupported ? spdlog::info("{:<30} - supported", requiredLayer) : spdlog::error("{:<30} - not supported", requiredLayer);

			return layerSupported;
			});

		// Create Vulkan Instance if possible
		if (instanceExtentionsSupported && instanceLayersSupported)
		{
			vk::ApplicationInfo applicationInfo; 
			applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); 
			applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0); 
			applicationInfo.pApplicationName = "PyraVK";
			applicationInfo.pEngineName = "Pyra Engine"; 
			applicationInfo.apiVersion = vk::ApiVersion14;

			vk::InstanceCreateInfo instanceCreateInfo; 
			instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExtentions.size()); 
			instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredInstanceLayers.size());
			instanceCreateInfo.ppEnabledExtensionNames = requiredInstanceExtentions.data();
			instanceCreateInfo.ppEnabledLayerNames = requiredInstanceLayers.data();
			instanceCreateInfo.pApplicationInfo = &applicationInfo; 

			vulkanInstance = vk::raii::Instance(context, instanceCreateInfo);
		}

		else
		{
			throw std::runtime_error("Vulkan Instance could not be created!");
		}


	}

}