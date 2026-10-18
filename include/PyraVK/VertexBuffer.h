#pragma once 
#include <PyraVK/Object.h>

namespace pyra
{

	struct Vertex
	{
		glm::vec2 position;
		glm::vec4 color;

		static vk::VertexInputBindingDescription getVertexBindingDescription()
		{
			vk::VertexInputBindingDescription bindingInfo;
			bindingInfo.binding = 0;
			bindingInfo.stride = sizeof(Vertex);
			bindingInfo.inputRate = vk::VertexInputRate::eVertex;
			return bindingInfo;
		};

		static std::vector<vk::VertexInputAttributeDescription> getVertexAttributeDescriptions()
		{
			vk::VertexInputAttributeDescription positionInfo;
			positionInfo.binding = 0;
			positionInfo.location = 0;
			positionInfo.format = vk::Format::eR32G32Sfloat;
			positionInfo.offset = offsetof(Vertex, position);

			vk::VertexInputAttributeDescription colorInfo;
			colorInfo.binding = 0;
			colorInfo.location = 1;
			colorInfo.format = vk::Format::eR32G32B32A32Sfloat;
			colorInfo.offset = offsetof(Vertex, color);

			std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{positionInfo, colorInfo};

			return attributeDescriptions;
		}
	};

	class VertexBuffer : public Object
	{
	private:
		vma::raii::Buffer vertexBuffer;
		uint32_t vertexCount;
	public:
		VertexBuffer(vma::raii::Buffer&& buffer, uint32_t vertexCount);
		[[nodiscard]] const vma::raii::Buffer& getVertexBuffer() const;
		[[nodiscard]] const uint32_t& getVertexCount() const;
	};

}