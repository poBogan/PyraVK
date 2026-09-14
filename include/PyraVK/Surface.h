#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/VulkanInstance.h>

namespace pyra
{

	class Surface : public Base
	{
		vk::raii::SurfaceKHR handle = nullptr; 
	public : 
		Surface(VulkanInstance& instance,VkSurfaceKHR surface);
		const vk::raii::SurfaceKHR& getSurfaceHandle() const; 
	};

}