#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>
#include <PyraVK/Shader.h>



namespace pyra
{

	class GraphicsPipeline : public Base
	{
	public : 	
		struct Config
		{
			std::string vertexShaderEntry;
			std::string fragmentShaderEntry;
			vk::Format colorAttachmentFormat;
			vk::PrimitiveTopology primitiveTopology;
			vk::CullModeFlagBits faceCullingMode; 
			vk::FrontFace frontFace; 

			bool operator==(const GraphicsPipeline::Config& other) const
			{
				bool is_equal =
					this->vertexShaderEntry == other.vertexShaderEntry &&
					this->fragmentShaderEntry == other.fragmentShaderEntry &&
					this->colorAttachmentFormat == other.colorAttachmentFormat &&
					this->primitiveTopology == other.primitiveTopology &&
					this->faceCullingMode == other.faceCullingMode &&
					this->frontFace == other.frontFace;
				return is_equal;
			};
		};

		struct Hasher
		{
			std::size_t operator()(const GraphicsPipeline::Config& c) const
			{
				return chainHash(c.vertexShaderEntry, c.fragmentShaderEntry,c.colorAttachmentFormat, c.primitiveTopology, c.faceCullingMode, c.frontFace);
			}
		};
	private : 
		vk::raii::Pipeline handle = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr; 
		const GraphicsPipeline::Config config; 
	public : 
		GraphicsPipeline(const Device& device,const Shader& vertexShaderModule,const Shader& fragmentShaderModule,const GraphicsPipeline::Config& config);
		const GraphicsPipeline::Config& getConfig() const;
		const vk::raii::Pipeline& getGraphicsPipelineHandle() const;
	};

}