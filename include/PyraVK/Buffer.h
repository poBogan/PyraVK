#pragma once
#include <PyraVK/PyraBase.h>

namespace pyra
{

    struct BufferConfig
    {
        vk::BufferUsageFlags c_buffer_usage;
        vk::DeviceSize c_buffer_size;
        vk::MemoryPropertyFlags c_memory_property_flags;
    };

    class Buffer : public PyraBase
    {
        vma::raii::Buffer m_buffer = nullptr;
        const BufferConfig m_config;
    public :
        Buffer(const BufferConfig& config, const class Device& device);
        [[nodiscard]] vk::Buffer get_buffer() const;
        [[nodiscard]] const vma::raii::Allocation& get_allocation() const;
        [[nodiscard]] const BufferConfig& get_config() const;
    };

}