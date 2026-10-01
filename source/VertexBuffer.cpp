#include <PyraVK/VertexBuffer.h>


namespace pyra
{
	VertexBuffer::VertexBuffer(const Device& device,const std::vector<Vertex>& verticies) : vertexCount(verticies.size())
	{
		vk::BufferCreateInfo bufferCreateInfo;
		bufferCreateInfo.size = static_cast<vk::DeviceSize>(verticies.size() * sizeof(Vertex));
		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		bufferCreateInfo.queueFamilyIndexCount = 1;
		bufferCreateInfo.pQueueFamilyIndices = &device.getConfig().graphicsQueueFamilyIndex;
		
		const auto& allocator = device.getDeviceMemoryAllocator();

		vma::AllocationCreateInfo allocationCreateInfo; 
		allocationCreateInfo.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;
		allocationCreateInfo.usage = vma::MemoryUsage::eAuto; 
		allocationCreateInfo.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

		vma::AllocationInfo allocationInfo;
		vma::raii::Buffer stageBuffer = allocator.createBuffer(bufferCreateInfo, allocationCreateInfo, allocationInfo);

		stageBuffer.getAllocation().copyFromMemory((void*)verticies.data(),0,verticies.size() * sizeof(Vertex));

		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst; 
		allocationCreateInfo.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

		vertexBuffer = allocator.createBuffer(bufferCreateInfo, allocationCreateInfo, allocationInfo);

		vk::CommandBufferAllocateInfo commandBufferAllocateInfo; 
		commandBufferAllocateInfo.commandBufferCount = 1; 
		commandBufferAllocateInfo.commandPool = *device.getCommandPoolHandle();
		commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary; 

		auto cbVector = vk::raii::CommandBuffers(device.getLogicalDeviceHandle(), commandBufferAllocateInfo);
		vk::raii::CommandBuffer copyCommandBuffer = std::move(cbVector.front());
		copyCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		copyCommandBuffer.copyBuffer(*stageBuffer, *vertexBuffer, vk::BufferCopy(0, 0, static_cast<vk::DeviceSize>(verticies.size() * sizeof(Vertex))));
		copyCommandBuffer.end(); 

		vk::CommandBuffer cmdBuf = *copyCommandBuffer;
		vk::SubmitInfo submitInfo; 
		submitInfo.commandBufferCount = 1; 
		submitInfo.pCommandBuffers = &cmdBuf;

		const auto& graphicsQueue = device.getGraphicsQueueHandle();
		graphicsQueue.submit(submitInfo);
		device.getLogicalDeviceHandle().waitIdle();
	}

	const vk::raii::Buffer& VertexBuffer::getVertexBufferHandle() const
	{
		return vertexBuffer;
	}

	const uint32_t VertexBuffer::getVertexCount() const
	{
		return vertexCount;
	}
}