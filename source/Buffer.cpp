#include <PyraVK/Buffer.h>
#include <PyraVK/Device.h>

namespace pyra
{
    Buffer::Buffer(const BufferConfig& config, const Device& device) : m_config(config)
    {
        const auto& allocator = device.get_allocator();

        vk::BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.queueFamilyIndexCount = 1;
        bufferCreateInfo.pQueueFamilyIndices = &device.get_queue_family_index();
        bufferCreateInfo.flags = {};
        bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
        bufferCreateInfo.usage = m_config.c_buffer_usage;
        bufferCreateInfo.size = m_config.c_buffer_size;

        vma::AllocationCreateInfo allocationCreateInfo;
        allocationCreateInfo.usage = vma::MemoryUsage::eAuto;
        allocationCreateInfo.requiredFlags = m_config.c_memory_property_flags;

        m_buffer = allocator.createBuffer(bufferCreateInfo,allocationCreateInfo);
    }

    vk::Buffer Buffer::get_buffer() const
    {
        return m_buffer;
    }

    const vma::raii::Allocation& Buffer::get_allocation() const
    {
        return m_buffer.getAllocation();
    }

    const BufferConfig& Buffer::get_config() const
    {
        return m_config;
    }
}
