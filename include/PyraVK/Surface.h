#pragma once
#include <PyraVK/Object.h>

namespace pyra
{

	class Surface : public Object
	{
	public : 
		struct Config : Object::Config
		{
			VkSurfaceKHR surface;
		};
	private : 
		vk::raii::SurfaceKHR surface = nullptr; 
	public : 
		Surface(const Surface::Config config, const class VulkanInstance& instance);
		
		const vk::raii::SurfaceKHR& getSurface() const; 
	};

}