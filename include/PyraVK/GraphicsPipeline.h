#pragma once
#include <PyraVK/Object.h>


namespace pyra
{

	class GraphicsPipeline : public Object
	{
	public :
		struct Config
		{
			std::string vertexShaderEntry{};
			std::string fragmentShaderEntry{};
			vk::Format colorAttachmentFormat{};
			vk::PrimitiveTopology primitiveTopology{};
			vk::CullModeFlagBits faceCullingMode{};
			vk::FrontFace frontFace{};
		};
	private :
		vk::raii::Pipeline graphicsPipeline = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr;
		const GraphicsPipeline::Config config;
	public : 
		GraphicsPipeline(const GraphicsPipeline::Config& config, const class Device& device, const class Shader& vertexShaderModule, const class Shader& fragmentShaderModule,const vk::raii::PipelineCache& pipelineCache = nullptr);
		[[nodiscard]] const GraphicsPipeline::Config& getConfig() const;
		[[nodiscard]] const vk::raii::Pipeline& getGraphicsPipeline() const;
	};

}