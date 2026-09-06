#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace pyra
{
	class Renderer
	{
	private : 
		static Renderer* singletonRenderer;
		Renderer();
	private : 
		uint32_t vulkanVersionRequired = vk::ApiVersion13;
		std::vector<const char*> requiredInstanceExtentions;
		std::vector<const char*> requiredInstanceLayers{ "VK_LAYER_KHRONOS_validation" };

		vk::raii::Instance vulkanInstance = nullptr;
		vk::raii::Context context;

		vk::SurfaceKHR surface = VK_NULL_HANDLE;
		vk::Extent2D swapchainImageExtent;
		vk::Format swapchainImageFormat;
	
		std::vector<const char*> requiredDeviceExtentions{ vk::KHRSwapchainExtensionName };
		vk::raii::PhysicalDevice physicalDevice = nullptr; 
		vk::raii::Device logicalDevice = nullptr;
		vk::raii::Queue graphicsQueue = nullptr;
		uint32_t graphicsQueueFamilyIndex = 0;
	public : 
		Renderer(Renderer&) = delete;
		static Renderer* createRenderer();
		static Renderer* getRenderer();

		vk::Instance createVulkanInstance(std::vector<const char*>required_instance_extentions);
		vk::Instance getVulkanInstance(); 

		void configureDisplayWindow(vk::SurfaceKHR window_surface,vk::Extent2D window_size_in_pixels);

		std::vector<vk::raii::PhysicalDevice> getPhysicalDevices();
		bool physicalDeviceSuitable(vk::raii::PhysicalDevice device);
		void selectPhysicalDevice(vk::raii::PhysicalDevice device); 
	private : 
		void createLogicalDevice();
		void createSwapchain();
	};
}