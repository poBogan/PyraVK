#include <PyraVK/Surface.h>

namespace pyra
{


	Surface::Surface(const VulkanInstance& instance,const Surface::Config config) : config(config)
	{
		handle = vk::raii::SurfaceKHR(instance.getVulkanInstanceHandle(), config.surface);
	}

	const vk::raii::SurfaceKHR& Surface::getSurfaceHandle() const
	{
		return handle;
	}
}
