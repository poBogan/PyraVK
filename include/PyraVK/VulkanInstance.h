#pragma once
#include <PyraVK/PyraBase.h>


namespace pyra
{
    class VulkanContext : public PyraBase
    {
        vk::raii::Context m_context;
        uint32_t m_max_supported_api_version;
        std::unordered_set<std::string> m_instance_extensions;
        std::unordered_set<std::string> m_instance_layers;
    public :
        VulkanContext();
        bool api_version_supported(uint32_t api_version) const;
        bool instance_extension_supported(const char* extension_name) const;
        bool instance_layer_supported(const char* layer_name) const;
        const vk::raii::Context& get_context() const;
    };

    class VulkanInstance : public PyraBase
    {
        VulkanContext m_context;
        vk::raii::Instance m_instance = nullptr;
    public:
        static constexpr uint32_t vulkan_api_version = vk::ApiVersion14;

        VulkanInstance(const std::vector<const char*>& extensions,const std::vector<const char*>& layers);
        const VulkanContext& getVulkanContext() const;
        [[nodiscard]] const vk::raii::Instance& get_instance() const;
        std::vector<struct DeviceConfig> get_suitable_devices(const class VulkanSurface& surface) const;
    };
}
