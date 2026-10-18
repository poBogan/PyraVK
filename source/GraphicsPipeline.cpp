#include <PyraVK/GraphicsPipeline.h>
#include <PyraVK/Shader.h>
#include <PyraVk/Device.h>
#include <PyraVK/VertexBuffer.h>

namespace pyra
{
	GraphicsPipeline::GraphicsPipeline(const GraphicsPipeline::Config& config, const class Device& device, const class Shader& vertexShaderModule, const class Shader& fragmentShaderModule, const vk::raii::PipelineCache& pipelineCache) : Object(), config(config)
	{
		vk::PipelineShaderStageCreateInfo vertexShaderStageInfo{};
		vertexShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertexShaderStageInfo.module = vertexShaderModule.getShaderModule();
		vertexShaderStageInfo.pName = config.vertexShaderEntry.c_str();

		vk::PipelineShaderStageCreateInfo fragmentShaderStageInfo{};
		fragmentShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragmentShaderStageInfo.module = fragmentShaderModule.getShaderModule();
		fragmentShaderStageInfo.pName = config.fragmentShaderEntry.c_str();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{vertexShaderStageInfo, fragmentShaderStageInfo};

		auto bindingDescription = Vertex::getVertexBindingDescription();
		auto attributeDescriptions = Vertex::getVertexAttributeDescriptions();

		vk::PipelineVertexInputStateCreateInfo vertexInputState;
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &bindingDescription;
		vertexInputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
		vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

		vk::PipelineInputAssemblyStateCreateInfo assemblyStateInfo;
		assemblyStateInfo.topology = config.primitiveTopology;
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
		rasterizationInfo.cullMode = config.faceCullingMode;
		rasterizationInfo.frontFace = config.frontFace;
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

		pipelineLayout = vk::raii::PipelineLayout(device.getLogicalDevice(), pipelineLayoutInfo);

		vk::PipelineRenderingCreateInfo pipelineRenderingInfo;
		pipelineRenderingInfo.viewMask = 0;
		pipelineRenderingInfo.colorAttachmentCount = 1;
		pipelineRenderingInfo.pColorAttachmentFormats = &config.colorAttachmentFormat;

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
		graphicsPipelineCreateInfo.layout = *pipelineLayout;
		graphicsPipelineCreateInfo.renderPass = nullptr;

		graphicsPipeline = vk::raii::Pipeline(device.getLogicalDevice(),pipelineCache, vk::GraphicsPipelineCreateInfo(graphicsPipelineCreateInfo));
	}

	const GraphicsPipeline::Config& GraphicsPipeline::getConfig() const
	{
		return config;
	}

	const vk::raii::Pipeline& GraphicsPipeline::getGraphicsPipeline() const
	{
		return graphicsPipeline;
	}

}