#include <PyraVK/Renderer.h>
#include <spdlog/spdlog.h>
#include <iostream>
#include <algorithm>
#include <ranges>
#include <limits>
#include <vector>


namespace pyra
{

	Renderer* Renderer::singletonRenderer = nullptr;

	Renderer* Renderer::createRenderer()
	{
		if (singletonRenderer == nullptr)
		{
			singletonRenderer = new Renderer();
		}
		return singletonRenderer;
	}

	Renderer* Renderer::getRenderer()
	{
		return singletonRenderer;
	}

	Renderer::Renderer()
	{

	}

	vk::Instance Renderer::createVulkanInstance(std::vector<const char*> required_instance_extentions)
	{
		requiredInstanceExtentions.insert(requiredInstanceExtentions.begin(), required_instance_extentions.begin(), required_instance_extentions.end());

		// Check if required instance extentions are supported
		std::vector<vk::ExtensionProperties> supportedInstanceExtentions(vk::enumerateInstanceExtensionProperties());

		bool instanceExtentionsSupported = std::ranges::all_of(requiredInstanceExtentions, [&supportedInstanceExtentions](const char* required_extention) -> bool {
			bool extentionSupported = std::ranges::any_of(supportedInstanceExtentions, [&required_extention](const vk::ExtensionProperties supported_extention) -> bool {
				return strcmp(required_extention, supported_extention.extensionName.data()) == 0;
				});
			return extentionSupported;
			});

		// Check if required instance layers are supported
		std::vector<vk::LayerProperties> supportedInstanceLayers(vk::enumerateInstanceLayerProperties());

		bool instanceLayersSupported = std::ranges::all_of(requiredInstanceLayers, [&supportedInstanceLayers](const char* requiredLayer) -> bool {
			bool layerSupported = std::ranges::any_of(supportedInstanceLayers, [&requiredLayer](const vk::LayerProperties& supportedLayer) -> bool {
				return strcmp(requiredLayer, supportedLayer.layerName.data()) == 0;
				});
			return layerSupported;
			});

		// Create Vulkan Instance if possible
		if (instanceExtentionsSupported && instanceLayersSupported)
		{
			try
			{
				vk::ApplicationInfo applicationInfo;
				applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
				applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
				applicationInfo.pApplicationName = "PyraVK";
				applicationInfo.pEngineName = "Pyra Engine";
				applicationInfo.apiVersion = vulkanVersionRequired;

				vk::InstanceCreateInfo instanceCreateInfo;
				instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExtentions.size());
				instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(requiredInstanceLayers.size());
				instanceCreateInfo.ppEnabledExtensionNames = requiredInstanceExtentions.data();
				instanceCreateInfo.ppEnabledLayerNames = requiredInstanceLayers.data();
				instanceCreateInfo.pApplicationInfo = &applicationInfo;

				vulkanInstance = vk::raii::Instance(context, instanceCreateInfo);
			}
			catch (std::exception& e)
			{
				spdlog::critical("Vulkan Instance could not be created");
				throw std::runtime_error("Vulkan Instance could not be created!");
			}
		}

		else
		{
			spdlog::critical("Vulkan instance could not be created");
			throw std::runtime_error("Vulkan Instance could not be created!");
		}

	}

	vk::Instance Renderer::getVulkanInstance()
	{
		return (*vulkanInstance);
	}

	void Renderer::configureDisplayWindow(vk::SurfaceKHR window_surface, vk::Extent2D window_size_in_pixels)
	{
		surface = window_surface; 
		swapchainImageExtent = window_size_in_pixels;
	}

	std::vector<vk::raii::PhysicalDevice> Renderer::getPhysicalDevices()
	{
		return vulkanInstance.enumeratePhysicalDevices();
	}

	bool Renderer::physicalDeviceSuitable(vk::raii::PhysicalDevice device)
	{
		if (surface == VK_NULL_HANDLE)
		{
			spdlog::error("No window surface bound, before checking for device suitability you should bind a device surface!");
			throw std::runtime_error("No window surface bound");
		}

		auto device_name = device.getProperties().deviceName.data();

		//Check if vulkan version supported 
		bool vulkanApiVersionSupported = device.getProperties().apiVersion >= vulkanVersionRequired;
		if (!vulkanApiVersionSupported)
		{
			spdlog::warn("{:} does not support required api version!", device_name);
		}

		// Check if device supports required extentions 
		std::vector<vk::ExtensionProperties> supportedDeviceExtentions = device.enumerateDeviceExtensionProperties();

		bool extentionsSupported = std::ranges::all_of(requiredDeviceExtentions, [&supportedDeviceExtentions, &device_name,this](const char* required) -> bool {
			bool extentionSupported = std::ranges::any_of(supportedDeviceExtentions, [&required](vk::ExtensionProperties& supported) -> bool {
				return strcmp(required, supported.extensionName.data()) == 0;
				});

			if (!extentionSupported)
			{
				spdlog::warn("{:} does not support \"{:}\"", device_name, required);
			}

			return extentionSupported;
			});

		// Check if device has required features 
		auto deviceFeatures = device.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
		bool featuresSupported =
			deviceFeatures.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
			deviceFeatures.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
			deviceFeatures.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

		if (!deviceFeatures.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters)
		{
			spdlog::warn("{:} does not support \"Shader Draw Parameters\"",device_name);
		}

		if (!deviceFeatures.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering)
		{
			spdlog::warn("{:} does not support \"Dynamic Rendering\"",device_name);
		}

		if (!deviceFeatures.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState)
		{
			spdlog::warn("{:} does not support \"Extended Dynamic State\"",device_name);
		}

		//Check if device supports graphics and presentation operations : 
		std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
		bool supportsGraphics = false;

		for (graphicsQueueFamilyIndex = 0;graphicsQueueFamilyIndex < queueFamilies.size() ; graphicsQueueFamilyIndex++)
		{
			if ((queueFamilies[graphicsQueueFamilyIndex].queueFlags & vk::QueueFlagBits::eGraphics) && device.getSurfaceSupportKHR(graphicsQueueFamilyIndex,surface))
			{
				supportsGraphics = true; 
				break;
			}
		}

		if (!supportsGraphics)
		{
			spdlog::warn("{:} does not support graphics operetions!",device_name);
		}

		return vulkanApiVersionSupported && extentionsSupported && featuresSupported && supportsGraphics;
	}

	void Renderer::selectPhysicalDevice(vk::raii::PhysicalDevice device)
	{
		if (physicalDeviceSuitable(device))
		{
			physicalDevice = device;
			spdlog::info("\"{:}\" is selected", physicalDevice.getProperties().deviceName.data());
			createLogicalDevice();
			createSwapchain();
		}

		else
		{
			spdlog::error("{:} is not suitable!", device.getProperties().deviceName.data());
			throw std::runtime_error("Device not suitable");
		}
	}

	void Renderer::createLogicalDevice()
	{
		float queueProperties = 0.5f;
		vk::DeviceQueueCreateInfo queueInfo;
		queueInfo.pQueuePriorities = &queueProperties;
		queueInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
		queueInfo.queueCount = 1;

		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> deviceFeatures;
		deviceFeatures.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters = true;
		deviceFeatures.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true; 
		deviceFeatures.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;

		vk::DeviceCreateInfo deviceInfo;
		deviceInfo.pNext = &deviceFeatures;
		deviceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtentions.size());
		deviceInfo.ppEnabledExtensionNames = requiredDeviceExtentions.data();
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pQueueCreateInfos = &queueInfo;

		logicalDevice = vk::raii::Device(physicalDevice, deviceInfo);
		graphicsQueue = vk::raii::Queue(logicalDevice, graphicsQueueFamilyIndex, 0);
	}

	void Renderer::createSwapchain()
	{

		auto surface_capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface); 

		if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			swapchainImageExtent = surface_capabilities.currentExtent;
		}

		else
		{
			swapchainImageExtent.width = std::clamp<uint32_t>(swapchainImageExtent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
			swapchainImageExtent.height = std::clamp<uint32_t>(swapchainImageExtent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
		}
		
	}

}