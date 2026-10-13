#pragma once
#include <PyraVK/Object.h>


namespace pyra
{
	struct Viewport
	{
		glm::vec2 offset; 
		glm::vec2 extent;
	};

	struct Scissor
	{
		glm::vec2 offset; 
		glm::vec2 extent;
	};

	class GraphicsPipeline : public Object
	{
	public : 	
		struct Config : Object::Config
		{
			std::string vertexShaderEntry;
			std::string fragmentShaderEntry;
			vk::Format colorAttachmentFormat;
			vk::PrimitiveTopology primitiveTopology;
			vk::CullModeFlagBits faceCullingMode; 
			vk::FrontFace frontFace; 
		};
	private : 
		vk::raii::Pipeline graphicsPipeline = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr; 
		const GraphicsPipeline::Config config; 
	public : 
		GraphicsPipeline(const GraphicsPipeline::Config config, const class Device& device, const class Shader& vertexShaderModule, const class Shader& fragmentShaderModule,const vk::raii::PipelineCache& pipelineCache = nullptr);
		const GraphicsPipeline::Config& getConfig() const;
		const vk::raii::Pipeline& getGraphicsPipeline() const;
	};

}