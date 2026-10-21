#include <PyraVK/GraphicsPipeline.h>
#include <PyraVk/Device.h>
#include <PyraVK/Shader.h>

namespace pyra
{
	GraphicsPipeline::GraphicsPipeline(
			const GraphicsPipelineConfig& graphics_pipeline_config,
			const Shader& vertex_shader,
			const Shader& fragment_shader,
			const Device& device,
			const vk::raii::PipelineCache& pipeline_cache
			) : m_graphics_pipeline_config(graphics_pipeline_config)
	{
		vk::PipelineShaderStageCreateInfo vertexShaderStageInfo{};
		vertexShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertexShaderStageInfo.module = vertex_shader.get_shader_module();
		vertexShaderStageInfo.pName = graphics_pipeline_config.vertexShaderEntry.c_str();

		vk::PipelineShaderStageCreateInfo fragmentShaderStageInfo{};
		fragmentShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragmentShaderStageInfo.module = fragment_shader.get_shader_module();
		fragmentShaderStageInfo.pName = graphics_pipeline_config.fragmentShaderEntry.c_str();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{vertexShaderStageInfo, fragmentShaderStageInfo};

		vk::PipelineVertexInputStateCreateInfo vertexInputState;

		vk::PipelineInputAssemblyStateCreateInfo assemblyStateInfo;
		assemblyStateInfo.topology = graphics_pipeline_config.primitiveTopology;
		assemblyStateInfo.primitiveRestartEnable = vk::False;

		vk::PipelineViewportStateCreateInfo viewportInfo;
		viewportInfo.viewportCount = 1;
		viewportInfo.scissorCount = 1;

		std::vector<vk::DynamicState> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor};

		vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
		dynamicStateInfo.dynamicStateCount = dynamicStates.size();
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
		rasterizationInfo.depthClampEnable = vk::False;
		rasterizationInfo.rasterizerDiscardEnable = vk::False;
		rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
		rasterizationInfo.cullMode = graphics_pipeline_config.faceCullingMode;
		rasterizationInfo.frontFace = graphics_pipeline_config.frontFace;
		rasterizationInfo.depthBiasEnable = vk::False;
		rasterizationInfo.lineWidth = 1;

		vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
		multisampleStateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisampleStateInfo.sampleShadingEnable = vk::False;
		multisampleStateInfo.alphaToCoverageEnable = vk::False;
		multisampleStateInfo.alphaToOneEnable = vk::False;

		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		colorBlendAttachment.blendEnable = vk::True;
		colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

		vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo;
		colorBlendStateInfo.logicOpEnable = vk::False;
		colorBlendStateInfo.logicOp = vk::LogicOp::eCopy;
		colorBlendStateInfo.attachmentCount = 1;
		colorBlendStateInfo.pAttachments = &colorBlendAttachment;

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		m_pipeline_layout = vk::raii::PipelineLayout(device.get_logical_device(), pipelineLayoutInfo);

		vk::PipelineRenderingCreateInfo pipelineRenderingInfo;
		pipelineRenderingInfo.viewMask = 0;
		pipelineRenderingInfo.colorAttachmentCount = 1;
		pipelineRenderingInfo.pColorAttachmentFormats = &graphics_pipeline_config.colorAttachmentFormat;

		vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
		graphicsPipelineCreateInfo.pNext = &pipelineRenderingInfo;
		graphicsPipelineCreateInfo.stageCount = shaderStages.size();
		graphicsPipelineCreateInfo.pStages = shaderStages.data();
		graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
		graphicsPipelineCreateInfo.pInputAssemblyState = &assemblyStateInfo;
		graphicsPipelineCreateInfo.pTessellationState = nullptr;
		graphicsPipelineCreateInfo.pViewportState = &viewportInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &rasterizationInfo;
		graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateInfo;
		graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateInfo;
		graphicsPipelineCreateInfo.pDynamicState = &dynamicStateInfo;
		graphicsPipelineCreateInfo.layout = *m_pipeline_layout;
		graphicsPipelineCreateInfo.renderPass = nullptr;

		m_graphics_pipeline = vk::raii::Pipeline(device.get_logical_device(),pipeline_cache, vk::GraphicsPipelineCreateInfo(graphicsPipelineCreateInfo));
	}

	const vk::raii::Pipeline& GraphicsPipeline::get_graphics_pipeline() const
	{
		return m_graphics_pipeline;
	}

	const vk::raii::PipelineLayout& GraphicsPipeline::get_pipeline_layout() const
	{
		return m_pipeline_layout;
	}

	const GraphicsPipelineConfig& GraphicsPipeline::get_config() const
	{
		return m_graphics_pipeline_config;
	}
}
