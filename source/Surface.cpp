#include <PyraVK/Surface.h>

namespace pyra
{


	Surface::Surface(VulkanInstance& instance,VkSurfaceKHR surface)
	{
		handle = vk::raii::SurfaceKHR(instance.getVulkanInstanceHandle(), surface);
	}

	const vk::raii::SurfaceKHR& Surface::getSurfaceHandle() const
	{
		return handle;
	}
}
