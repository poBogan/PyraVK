#pragma once 
#include <PyraVK/Object.h>

namespace pyra
{

	struct Vertex
	{
		glm::vec2 position;
		glm::vec4 color;

		static VkVertexInputBindingDescription getVertexBindingDescription()
		{
			VkVertexInputBindingDescription bindingInfo{
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
			};

			return bindingInfo;
		};

		static std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions
			{
				{.location = 0,.binding = 0,.format = VK_FORMAT_R32G32_SFLOAT,.offset = offsetof(Vertex,position) }, 
				{.location = 1,.binding = 0,.format = VK_FORMAT_R32G32B32A32_SFLOAT,.offset = offsetof(Vertex,color) }
			}; 
			return attributeDescriptions;
		}
	};

	class VertexBuffer : public Object
	{
	private:
		vma::raii::Buffer vertexBuffer;
		uint32_t vertexCount;
	public:
		VertexBuffer(vma::raii::Buffer&& buffer, const uint32_t vertexCount);
		const vma::raii::Buffer& getVertexBuffer() const; 
		const uint32_t& getVertexCount() const;
	};

}