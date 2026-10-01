#pragma once 
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>

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

	class VertexBuffer : public Base
	{
	private:
		vk::raii::Buffer vertexBuffer = nullptr;
		const uint32_t vertexCount;
	public:
		VertexBuffer(const Device& device,const std::vector<Vertex> verticies);
		const vk::raii::Buffer& getVertexBufferHandle() const; 
		const uint32_t getVertexCount() const;
	};

}