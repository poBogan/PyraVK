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

		vk::Format targetFormat_; 
		vk::Viewport viewport_; 
		vk::Rect2D scissor_; 
		vk::PrimitiveTopology primitiveTopology_; 
		vk::CullModeFlagBits faceCulling_; 
		vk::FrontFace frontFace_; 
		vk::PolygonMode polygonMode_; 
		vk::SampleCountFlagBits sampleCount_;
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
		const vk::Viewport getViewport() const; 
		const vk::Rect2D getScissor() const; 
		const vk::PrimitiveTopology getPrimitiveTopology() const; 
		const vk::CullModeFlagBits getCullMode() const;
		const vk::FrontFace getFrontFace() const; 
		const vk::PolygonMode getPolygonMode() const; 
		const vk::SampleCountFlagBits getSampleCount() const;
	};

}