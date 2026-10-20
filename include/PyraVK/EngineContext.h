#pragma once
#include <PyraVK/Object.h>
#include <PyraVK/VulkanInstance.h>
#include <PyraVK/VulkanSurface.h>
#include <PyraVK/Device.h>

namespace pyra
{

    class EngineContext : public Object
    {
        VulkanInstance m_instance;
        VulkanSurface m_surface;
        Device m_device;
    public :
        EngineContext(VulkanInstance&& instance, VulkanSurface&& surface, Device&& device);

        const VulkanInstance& getVulkanInstance() const;
        const VulkanSurface& getVulkanSurface() const;
        const Device& getDevice() const;
    };

}
