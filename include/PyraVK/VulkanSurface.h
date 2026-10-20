#pragma once
#include <PyraVK/Object.h>

namespace pyra
{

	class VulkanSurface : public Object
	{
		vk::raii::SurfaceKHR m_surface = nullptr;
	public : 
		VulkanSurface(VkSurfaceKHR surface, const class VulkanInstance& instance);
		[[nodiscard]] const vk::raii::SurfaceKHR& get_surface() const;
	};

}