#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>
#include <PyraVK/Shader.h>

namespace pyra
{

	class GraphicsPipeline : public Base
	{
	private : 
		vk::raii::Pipeline handle = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr; 

		std::string vertexShaderEntry; 
		std::string fragmentshaderEntry;

		vk::Format targetFormat;
		vk::PrimitiveTopology primitiveTopology;
		vk::CullModeFlagBits faceCullingMode; 
		vk::FrontFace frontFace; 
		vk::SampleCountFlagBits sampleCount;
	public : 
		GraphicsPipeline(
			Device& device,
			Shader& vertexShaderModule,	std::string vertexShaderEntry,
			Shader& fragmentShaderModule, std::string fragmentShaderEntry,
			vk::Format targetFormat,
			vk::PrimitiveTopology primitiveTopology,
			vk::CullModeFlagBits faceCullingMode,
			vk::FrontFace frontFace,
			vk::SampleCountFlagBits sampleCount
		);

		std::string getVertexShaderEntry() const; 
		std::string getFragmentShaderEntry() const;
		vk::Format getTargetFormat() const;
		vk::PrimitiveTopology getPrimitiveTopology() const; 
		vk::CullModeFlagBits getFaceCullingMode() const; 
		vk::FrontFace getFrontFace() const; 
		vk::SampleCountFlagBits getSampleCount() const; 

		const vk::raii::Pipeline& getGraphicsPipelineHandle() const;
	};

}