#include <PyraVK/Surface.h>

namespace pyra
{


	Surface::Surface(std::unique_ptr<VulkanInstance>& instance,vk::SurfaceKHR surface, std::vector<vk::SurfaceFormatKHR> format_preferences)
	{
		surface_obj = vk::raii::SurfaceKHR(instance->getInstanceObject(),std::move(surface));
		this->format_preferences = std::move(format_preferences);
	}

	std::unique_ptr<Surface> Surface::createSurface(std::unique_ptr<VulkanInstance>& instance, vk::SurfaceKHR surface, std::vector<vk::SurfaceFormatKHR> format_preferences)
	{
		return std::unique_ptr<Surface>(new Surface(instance,std::move(surface),std::move(format_preferences)));
	}

	const vk::raii::SurfaceKHR& Surface::getSurfaceObj() const
	{
		return surface_obj;
	}

	const std::vector<vk::SurfaceFormatKHR>& Surface::getFormatPreferences() const
	{
		return format_preferences;
	}


}
