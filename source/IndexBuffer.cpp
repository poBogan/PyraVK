#include <PyraVK/IndexBuffer.h>


namespace pyra
{

	IndexBuffer::IndexBuffer(const Device& device, std::vector<uint32_t>& indicies)
	{
		indiciesCount = indicies.size();

		vk::BufferCreateInfo bufferCreateInfo; 
		bufferCreateInfo.queueFamilyIndexCount = 1; 
		bufferCreateInfo.pQueueFamilyIndices = &device.getConfig().graphicsQueueFamilyIndex;
		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
		bufferCreateInfo.size = indicies.size() * sizeof(uint32_t);
		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc; 
		
		const auto& allocator = device.getDeviceMemoryAllocator(); 

		vma::AllocationCreateInfo allocationCreateInfo;
		allocationCreateInfo.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;
		allocationCreateInfo.usage = vma::MemoryUsage::eAuto; 
		allocationCreateInfo.requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;

		vma::AllocationInfo allocationInfo;
		vma::raii::Buffer stageBuffer = allocator.createBuffer(bufferCreateInfo, allocationCreateInfo, allocationInfo);

		stageBuffer.getAllocation().copyFromMemory(indicies.data(), 0, indicies.size() * sizeof(uint32_t));

		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
		allocationCreateInfo.flags = {};
		allocationCreateInfo.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal; 

		indexBuffer = allocator.createBuffer(bufferCreateInfo, allocationCreateInfo, allocationInfo);

		vk::CommandBufferAllocateInfo commandBufferAllocateInfo; 
		commandBufferAllocateInfo.commandBufferCount = 1; 
		commandBufferAllocateInfo.commandPool = *device.getCommandPoolHandle(); 
		commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary; 

		auto cbVector = vk::raii::CommandBuffers(device.getLogicalDeviceHandle(), commandBufferAllocateInfo);
		vk::raii::CommandBuffer copyCommandBuffer = std::move(cbVector.front());
		copyCommandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
		copyCommandBuffer.copyBuffer(*stageBuffer, *indexBuffer, vk::BufferCopy(0, 0, static_cast<vk::DeviceSize>(indicies.size() * sizeof(uint32_t))));
		copyCommandBuffer.end();

		vk::CommandBuffer cmdBuf = *copyCommandBuffer;
		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuf;

		const auto& graphicsQueue = device.getGraphicsQueueHandle();
		graphicsQueue.submit(submitInfo);
		device.getLogicalDeviceHandle().waitIdle();
	}

	const vk::raii::Buffer& IndexBuffer::getIndexBufferHandle() const
	{
		return indexBuffer;
	}

	uint32_t IndexBuffer::getIndiciesCount() const
	{
		return indiciesCount;
	}

}