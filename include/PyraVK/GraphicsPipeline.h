#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>

namespace pyra
{

	class GraphicsPipeline : public Base
	{
	private : 
		vk::raii::Pipeline handle = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr; 
		vk::raii::ShaderModule shaderModule = nullptr;
	public : 
		GraphicsPipeline(
			Device& device,
			vk::Format targetFormat,
			vk::Viewport viewport,
			vk::Rect2D scissor,
			vk::PrimitiveTopology primitiveTopology,
			vk::CullModeFlagBits faceCulling,
			vk::FrontFace frontFace,
			vk::PolygonMode polygonMode,
			vk::SampleCountFlagBits sampleCount
		);

		const vk::raii::Pipeline& getPipelineHandle() const; 
		const vk::Format getTargetFormat() const;
	};

}