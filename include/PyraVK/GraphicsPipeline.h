#pragma once
#include <PyraVK/PyraBase.h>


namespace pyra
{

	struct GraphicsPipelineConfig
	{
		std::string vertexShaderEntry{};
		std::string fragmentShaderEntry{};
		vk::Format colorAttachmentFormat{};
		vk::PrimitiveTopology primitiveTopology{};
		vk::CullModeFlagBits faceCullingMode{};
		vk::FrontFace frontFace{};
	};

	class GraphicsPipeline : public PyraBase
	{
		vk::raii::Pipeline m_graphics_pipeline = nullptr;
		vk::raii::PipelineLayout m_pipeline_layout = nullptr;
		const GraphicsPipelineConfig m_graphics_pipeline_config;
	public :
		GraphicsPipeline(
			const GraphicsPipelineConfig& graphics_pipeline_config,
			const class Shader& vertex_shader,
			const Shader& fragment_shader,
			const class Device& device,
			const vk::raii::PipelineCache& pipeline_cache = nullptr
			);

		[[nodiscard]] const vk::raii::Pipeline& get_graphics_pipeline() const;
		[[nodiscard]] const vk::raii::PipelineLayout& get_pipeline_layout() const;
		[[nodiscard]] const GraphicsPipelineConfig& get_config() const;
	};

}