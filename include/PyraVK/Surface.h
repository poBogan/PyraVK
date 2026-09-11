#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/VulkanInstance.h>

namespace pyra
{

	class Surface
	{
		vk::raii::SurfaceKHR surface_obj = nullptr; 
		std::vector<vk::SurfaceFormatKHR> format_preferences{};
	private : 
		Surface(std::unique_ptr<VulkanInstance>& instance,vk::SurfaceKHR surface, std::vector<vk::SurfaceFormatKHR> format_preferences);
	public : 
		Surface() = delete; 
		Surface(Surface&) = delete; 
		Surface(Surface&&) = delete; 

		static std::unique_ptr<Surface> createSurface(std::unique_ptr<VulkanInstance>& instance,vk::SurfaceKHR surface, std::vector<vk::SurfaceFormatKHR> format_preferences = {{vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear}, {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear}});
		const vk::raii::SurfaceKHR& getSurfaceObj() const; 
		const std::vector<vk::SurfaceFormatKHR>& getFormatPreferences() const;
	};

}