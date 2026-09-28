#include <PyraVK/VertexBuffer.h>


namespace pyra
{
	VertexBuffer::VertexBuffer(const Device& device,const std::vector<Vertex> verticies) : vertexCount(verticies.size())
	{
		VkBufferCreateInfo bufferCreateInfo{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.size = static_cast<uint32_t>(verticies.size() * sizeof(Vertex)),
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &device.getConfig().graphicsQueueFamilyIndex
		};

		vertexBuffer = vk::raii::Buffer(device.getLogicalDeviceHandle(),vk::BufferCreateInfo(bufferCreateInfo));

		vk::PhysicalDeviceMemoryProperties deviceMemoryProperties = device.getConfig().physicalDevice.getMemoryProperties();
		vk::MemoryRequirements bufferMemoryRequirements = vertexBuffer.getMemoryRequirements();

		uint32_t memoryTypeIndex = deviceMemoryProperties.memoryTypeCount;

		for (int i = 0;i < deviceMemoryProperties.memoryTypeCount;i++)
		{
			if ((1 << i) & bufferMemoryRequirements.memoryTypeBits)
			{
				static uint32_t requirementsBitfield = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
				if (((uint32_t)deviceMemoryProperties.memoryTypes[i].propertyFlags & requirementsBitfield) == requirementsBitfield)
				{
					memoryTypeIndex = i; 
					break;
				}
			}
		}

		pyra_assert(memoryTypeIndex != deviceMemoryProperties.memoryTypeCount, "No available memory type found!");

		VkMemoryAllocateInfo memoryAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = NULL,
			.allocationSize = bufferMemoryRequirements.size,
			.memoryTypeIndex = memoryTypeIndex
		};

		vertexBufferMemory = vk::raii::DeviceMemory(device.getLogicalDeviceHandle(), memoryAllocateInfo);
		vertexBuffer.bindMemory(vertexBufferMemory, 0);

		void* mapped = vertexBufferMemory.mapMemory(0, bufferMemoryRequirements.size);
		memcpy(mapped, verticies.data(), verticies.size() * sizeof(Vertex));
		vertexBufferMemory.unmapMemory(); 
	}

	const vk::raii::Buffer& VertexBuffer::getVertexBufferHandle() const
	{
		return vertexBuffer;
	}
	const vk::raii::DeviceMemory& VertexBuffer::getDeviceMemoryHandle() const
	{
		return vertexBufferMemory;
	}
	const uint32_t VertexBuffer::getVertexCount() const
	{
		return vertexCount;
	}
}