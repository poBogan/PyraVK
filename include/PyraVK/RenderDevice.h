#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Surface.h>


namespace pyra
{

	class RenderDevice
	{
	private : 
		vk::raii::PhysicalDevice physicalDevice = nullptr;
		vk::raii::Device logicalDevice = nullptr; 
		vk::raii::Queue graphicsQueue = nullptr; 

		std::vector<const char*>device_extentions;
		uint32_t graphics_queue_family_index = 0;
		vk::PhysicalDeviceProperties deviceProperties;
		vk::SurfaceFormatKHR targetSurfaceFormat;

		RenderDevice(vk::raii::PhysicalDevice physical_device, uint32_t graphics_queue_family_index,vk::SurfaceFormatKHR target_surface_format, std::vector<const char*>device_extentions);
	public : 
		RenderDevice() = delete;
		RenderDevice(RenderDevice&) = delete;
		RenderDevice(RenderDevice&&) = delete;
		
		static std::unique_ptr<RenderDevice> createRenderDevice(vk::raii::PhysicalDevice physical_device, uint32_t graphics_queue_family_index, vk::SurfaceFormatKHR target_surface_format, std::vector<const char*>device_extentions);
		static std::vector<std::unique_ptr<RenderDevice>> getSuitableDevices(std::unique_ptr<VulkanInstance>& instance,std::unique_ptr<Surface>& surface,std::vector<const char*> device_extentions);

		vk::PhysicalDeviceProperties getDeviceProperties() const;
		vk::SurfaceFormatKHR getTargetSurfaceFormat() const;
		uint32_t get_graphics_queue_family_index() const;
		const std::vector<const char*>& getDeviceExtentions() const;

		const vk::raii::PhysicalDevice& getPhysicalDeviceHandle() const; 
		const vk::raii::Device& getLogicalDeviceHandle() const;
		const vk::raii::Queue& getGraphicsQueueHandle() const;
	};

}