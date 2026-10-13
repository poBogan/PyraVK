#pragma once 
#include <PyraVK/Object.h>

namespace pyra
{

	class IndexBuffer : public Object
	{
	private : 
		const vma::raii::Buffer indexBuffer; 
		const uint32_t indiciesCount = 0; 
	public : 
		IndexBuffer(vma::raii::Buffer&& indexBuffer, uint32_t indiciesCount);
		const vma::raii::Buffer& getIndexBuffer() const; 
		const uint32_t& getIndiciesCount() const;
	};

}