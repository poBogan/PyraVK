#include <PyraVK/VertexBuffer.h>


namespace pyra
{
	VertexBuffer::VertexBuffer(vma::raii::Buffer&& buffer, const uint32_t vertexCount) : Object({}, Object::Type::VERTEX_BUFFER), vertexBuffer(std::move(buffer)), vertexCount(vertexCount) {};

	const vma::raii::Buffer& VertexBuffer::getVertexBuffer() const
	{
		return vertexBuffer;
	}

	const uint32_t& VertexBuffer::getVertexCount() const
	{
		return vertexCount;
	}
}