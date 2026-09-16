#include <PyraVK/GraphicsPipeline.h>
#include <fstream>

namespace pyra
{
	const char* pyra_shader_path = "pyra_shader.spirv";
	const char* vertex_shader_entry = "VertexShaderMain"; 
	const char* fragment_shader_entry = "FragmentShaderMain";

	std::vector<char> readFile(const char* file_path)
	{
		std::ifstream file(file_path, std::ios::ate | std::ios::binary); 
		if (!file.is_open()) return {};

		int fileSize = file.tellg(); 
		file.seekg(std::ios::beg, 0); 

		std::vector<char> buffer(fileSize);
		file.read(buffer.data(), fileSize);

		return buffer;
	}

	GraphicsPipeline::GraphicsPipeline(
		Device& device,
		vk::Format targetFormat,
		vk::Viewport viewport,
		vk::Rect2D scissor,
		vk::PrimitiveTopology primitiveTopology,
		vk::CullModeFlagBits faceCulling,
		vk::FrontFace frontFace,
		vk::PolygonMode polygonMode,
		vk::SampleCountFlagBits sampleCount
	) : targetFormat_(targetFormat), viewport_(viewport), scissor_(scissor), primitiveTopology_(primitiveTopology),
		faceCulling_(faceCulling), frontFace_(frontFace), polygonMode_(polygonMode), sampleCount_(sampleCount)
	{
		auto code = readFile(pyra_shader_path);

		VkShaderModuleCreateInfo shaderModuleInfo{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.codeSize = (uint32_t)code.size(),
			.pCode = (uint32_t*)code.data(),
		};

		shaderModule = vk::raii::ShaderModule(device.getLogicalDeviceHandle(), shaderModuleInfo);

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
			{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = *shaderModule, .pName = vertex_shader_entry},
			{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = *shaderModule, .pName = fragment_shader_entry},
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

		VkViewport _viewport = (VkViewport)viewport; 
		VkRect2D _scissor = (VkRect2D)scissor;

		VkPipelineViewportStateCreateInfo viewportInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.viewportCount = 1,
			.pViewports = &_viewport,
			.scissorCount = 1,
			.pScissors = &_scissor
		}; 

		VkPipelineRasterizationStateCreateInfo rasterizationInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = NULL,
			.flags = NULL,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = (VkPolygonMode)polygonMode,
			.cullMode = (VkCullModeFlags)faceCulling,
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
			.pDynamicState = NULL,
			.layout = (VkPipelineLayout)*pipelineLayout,
			.renderPass = nullptr,
		};

		handle = vk::raii::Pipeline(device.getLogicalDeviceHandle(), nullptr, vk::GraphicsPipelineCreateInfo(graphicsPipelineCreateInfo));
	}

	const vk::raii::Pipeline& GraphicsPipeline::getPipelineHandle() const
	{
		return handle;
	}

	const vk::Format GraphicsPipeline::getTargetFormat() const
	{
		return targetFormat_;
	}

	const vk::Viewport GraphicsPipeline::getViewport() const
	{
		return viewport_;
	}

	const vk::Rect2D GraphicsPipeline::getScissor() const
	{
		return scissor_;
	}

	const vk::PrimitiveTopology GraphicsPipeline::getPrimitiveTopology() const
	{
		return primitiveTopology_;
	}

	const vk::CullModeFlagBits GraphicsPipeline::getCullMode() const
	{
		return faceCulling_;
	}

	const vk::FrontFace GraphicsPipeline::getFrontFace() const
	{
		return frontFace_;
	}

	const vk::PolygonMode GraphicsPipeline::getPolygonMode() const
	{
		return polygonMode_;
	}

	const vk::SampleCountFlagBits GraphicsPipeline::getSampleCount() const
	{
		return sampleCount_;
	}

}