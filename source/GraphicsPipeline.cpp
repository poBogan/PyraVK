#include <PyraVK/GraphicsPipeline.h>
#include <fstream>

namespace pyra
{

	GraphicsPipeline::GraphicsPipeline(
		Device& device,
		Shader& vertexShaderModule, std::string vertexShaderEntry,
		Shader& fragmentShaderModule, std::string fragmentShaderEntry,
		vk::Format targetFormat,
		vk::PrimitiveTopology primitiveTopology,
		vk::CullModeFlagBits faceCullingMode,
		vk::FrontFace frontFace,
		vk::SampleCountFlagBits sampleCount
	)
	{
		this->targetFormat = targetFormat;
		this->primitiveTopology = primitiveTopology; 
		this->faceCullingMode = faceCullingMode; 
		this->frontFace = frontFace; 
		this->sampleCount = sampleCount;
		this->vertexShaderEntry = vertexShaderEntry; 
		this->fragmentshaderEntry = fragmentShaderEntry;

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
			{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = (VkShaderModule)*vertexShaderModule.getShaderModuleHandle(), .pName = vertexShaderEntry.c_str()},
			{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = (VkShaderModule)*fragmentShaderModule.getShaderModuleHandle(), .pName = fragmentShaderEntry.c_str()},
		}; 

		VkPipelineVertexInputStateCreateInfo vertexInputState{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = NULL,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = NULL
		};

		VkPipelineInputAssemblyStateCreateInfo assemblyStateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.topology = (VkPrimitiveTopology)primitiveTopology,
			.primitiveRestartEnable = VK_FALSE
		};

		VkPipelineViewportStateCreateInfo viewportInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.viewportCount = 1,
			.scissorCount = 1,
		}; 

		std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = {},
			.dynamicStateCount = (uint32_t)dynamicStates.size(),
			.pDynamicStates = dynamicStates.data()
		};

		VkPipelineRasterizationStateCreateInfo rasterizationInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = (VkCullModeFlags)faceCullingMode,
			.frontFace = (VkFrontFace)frontFace,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1
		}; 

		VkPipelineMultisampleStateCreateInfo multisampleStateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.rasterizationSamples = (VkSampleCountFlagBits)sampleCount,
			.sampleShadingEnable = VK_FALSE,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE
		};

		VkPipelineColorBlendAttachmentState colorBlendAttachment{
			.blendEnable = VK_TRUE,
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment
		};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.setLayoutCount = 0,
			.pSetLayouts = NULL,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = NULL
		};

		pipelineLayout = vk::raii::PipelineLayout(device.getLogicalDeviceHandle(), vk::PipelineLayoutCreateInfo(pipelineLayoutInfo));

		const VkFormat _targetFormat = (VkFormat)targetFormat;

		VkPipelineRenderingCreateInfo pipelineRenderingInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.pNext = NULL,
			.viewMask = NULL,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &_targetFormat,
		};

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &pipelineRenderingInfo,
			.flags = NULL,
			.stageCount = (uint32_t)shaderStages.size(),
			.pStages = shaderStages.data(),
			.pVertexInputState = &vertexInputState,
			.pInputAssemblyState = &assemblyStateInfo,
			.pTessellationState = NULL,
			.pViewportState = &viewportInfo,
			.pRasterizationState = &rasterizationInfo,
			.pMultisampleState = &multisampleStateInfo,
			.pColorBlendState = &colorBlendStateInfo,
			.pDynamicState = &dynamicStateInfo,
			.layout = (VkPipelineLayout)*pipelineLayout,
			.renderPass = nullptr,
		};

		handle = vk::raii::Pipeline(device.getLogicalDeviceHandle(),device.getGraphicsPipelineCacheHandle(), vk::GraphicsPipelineCreateInfo(graphicsPipelineCreateInfo));
	}

	std::string GraphicsPipeline::getVertexShaderEntry() const
	{
		return vertexShaderEntry;
	}

	std::string GraphicsPipeline::getFragmentShaderEntry() const
	{
		return fragmentshaderEntry;
	}

	vk::Format GraphicsPipeline::getTargetFormat() const
	{
		return targetFormat;
	}

	vk::PrimitiveTopology GraphicsPipeline::getPrimitiveTopology() const
	{
		return primitiveTopology;
	}

	vk::CullModeFlagBits GraphicsPipeline::getFaceCullingMode() const
	{
		return faceCullingMode;
	}

	vk::FrontFace GraphicsPipeline::getFrontFace() const
	{
		return frontFace;
	}

	vk::SampleCountFlagBits GraphicsPipeline::getSampleCount() const
	{
		return sampleCount;
	}

	const vk::raii::Pipeline& GraphicsPipeline::getGraphicsPipelineHandle() const
	{
		return handle;
	}

}