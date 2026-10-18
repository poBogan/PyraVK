#pragma once
#include <PyraVK/Object.h>

namespace pyra
{

	class Surface : public Object
	{
	private : 
		vk::raii::SurfaceKHR surface = nullptr; 
	public : 
		Surface(const VkSurfaceKHR& surface, const class VulkanInstance& instance);
		[[nodiscard]] const vk::raii::SurfaceKHR& getSurface() const;
	};

}