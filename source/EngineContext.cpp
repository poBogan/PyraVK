#include "PyraVK/EngineContext.h"


namespace pyra
{
    EngineContext::EngineContext(VulkanInstance&& instance, VulkanSurface&& surface, Device&& device) :
    m_instance(std::move(instance)), m_surface(std::move(surface)), m_device(std::move(device))
    {
        pyra_assert(m_device.surface_is_compatible(m_surface));
    }

    const VulkanInstance& EngineContext::getVulkanInstance() const
    {
        return m_instance;
    }

    const VulkanSurface& EngineContext::getVulkanSurface() const
    {
        return m_surface;
    }

    const Device& EngineContext::getDevice() const
    {
        return m_device;
    }
}
