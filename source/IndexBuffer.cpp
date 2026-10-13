#include <PyraVK/IndexBuffer.h>


namespace pyra
{
	IndexBuffer::IndexBuffer(vma::raii::Buffer&& indexBuffer, uint32_t indiciesCount) : Object({}, Object::Type::INDEX_BUFFER), indexBuffer(std::move(indexBuffer)), indiciesCount(indiciesCount) {}
	
	const vma::raii::Buffer& IndexBuffer::getIndexBuffer() const
	{
		return indexBuffer;
	}

	const uint32_t& IndexBuffer::getIndiciesCount() const
	{
		return indiciesCount;
	}

	
}