#include <PyraVK/Surface.h>
#include <PyraVK/VulkanInstance.h>

namespace pyra
{
	Surface::Surface(const VkSurfaceKHR& surface, const VulkanInstance& instance) : Object()
	{
		this->surface = vk::raii::SurfaceKHR(instance.getInstance(), surface);
	}

	const vk::raii::SurfaceKHR& Surface::getSurface() const
	{
		return surface;
	}
}
