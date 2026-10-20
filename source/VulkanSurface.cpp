#include <PyraVK/VulkanSurface.h>
#include <PyraVK/VulkanInstance.h>

namespace pyra
{
	VulkanSurface::VulkanSurface(VkSurfaceKHR surface, const VulkanInstance& instance) : Object()
	{
		m_surface = vk::raii::SurfaceKHR(instance.get_instance(), surface);
	}

	const vk::raii::SurfaceKHR& VulkanSurface::get_surface() const
	{
		return m_surface;
	}
}
