#pragma once
#include <PyraVK/PyraBase.h>

namespace pyra
{

	class VulkanSurface : public PyraBase
	{
		vk::raii::SurfaceKHR m_surface = nullptr;
	public : 
		VulkanSurface(VkSurfaceKHR surface, const class VulkanInstance& instance);
		[[nodiscard]] const vk::raii::SurfaceKHR& get_surface() const;
	};

}