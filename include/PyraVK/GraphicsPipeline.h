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
			vk::Format targetFormat;
			vk::PrimitiveTopology primitiveTopology;
			vk::PolygonMode polygonMode;
			vk::CullModeFlagBits faceCullingMode; 
			vk::FrontFace frontFace; 
			vk::SampleCountFlagBits sampleCount;

			bool operator==(const GraphicsPipeline::Config& other) const
			{
				bool is_equal =
					this->vertexShaderEntry == other.vertexShaderEntry &&
					this->fragmentShaderEntry == other.fragmentShaderEntry &&
					this->targetFormat == other.targetFormat &&
					this->primitiveTopology == other.primitiveTopology &&
					this->polygonMode == other.polygonMode &&
					this->faceCullingMode == other.faceCullingMode &&
					this->frontFace == other.frontFace &&
					this->sampleCount == other.sampleCount;
				return is_equal;
			};
		};

		struct Hasher
		{
			std::size_t operator()(const GraphicsPipeline::Config& c) const
			{
				return chainHash(c.vertexShaderEntry, c.fragmentShaderEntry, c.targetFormat, c.primitiveTopology, c.polygonMode, c.faceCullingMode, c.frontFace, c.sampleCount);
			}
		};
	private : 
		vk::raii::Pipeline handle = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr; 
		const GraphicsPipeline::Config config;
	public : 
		GraphicsPipeline(Device& device, Shader& vertexShaderModule, Shader& fragmentShaderModule,const GraphicsPipeline::Config config);
		const GraphicsPipeline::Config& getConfig() const;
		const vk::raii::Pipeline& getGraphicsPipelineHandle() const;
	};

}