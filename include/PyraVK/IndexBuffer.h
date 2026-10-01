#pragma once 
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>

namespace pyra
{

	class IndexBuffer : public Base
	{
	private : 
		vma::raii::Buffer indexBuffer = nullptr; 
		uint32_t indiciesCount = 0; 
	public : 
		IndexBuffer(const Device& device, std::vector<uint32_t>& indicies);
		const vk::raii::Buffer& getIndexBufferHandle() const; 
		uint32_t getIndiciesCount() const;
	};

}