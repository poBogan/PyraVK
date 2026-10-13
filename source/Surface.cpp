#include <PyraVK/Surface.h>
#include <PyraVK/VulkanInstance.h>

namespace pyra
{
	Surface::Surface(const Surface::Config config, const VulkanInstance& instance) : Object(config,Object::Type::WINDOW_SURFACE)
	{
		surface = vk::raii::SurfaceKHR(instance.getInstance(), config.surface);
	}

	const vk::raii::SurfaceKHR& Surface::getSurface() const
	{
		return surface;
	}
}
