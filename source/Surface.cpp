#include <PyraVK/Surface.h>

namespace pyra
{


	Surface::Surface(VulkanInstance& instance,vk::SurfaceKHR surface)
	{
		handle = vk::raii::SurfaceKHR(instance.getVulkanInstanceHandle(), surface);
	}

	const vk::raii::SurfaceKHR& Surface::getSurfaceHandle() const
	{
		return handle;
	}
}
