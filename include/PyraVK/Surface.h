#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/VulkanInstance.h>

namespace pyra
{

	class Surface : public Base
	{
	public : 
		struct Config
		{
			VkSurfaceKHR surface;
		};
	private : 
		vk::raii::SurfaceKHR handle = nullptr; 
		const Surface::Config config;
	public : 
		Surface(const VulkanInstance& instance,const Surface::Config config);
		const Surface::Config& getConfig() const;
		const vk::raii::SurfaceKHR& getSurfaceHandle() const; 
	};

}