#include <PyraVK/Shader.h>
#include <fstream>

namespace pyra
{
	std::vector<char> readShader(std::string fileName)
	{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			spdlog::error("Shader file \"{:}\" could not be opened!",fileName.c_str());
			return {};
		}

		int fileSize = file.tellg();
		file.seekg(std::ios::beg, 0);

		std::vector<char> buffer(fileSize);
		file.read(buffer.data(), fileSize);

		return buffer;
	}

	pyra::Shader::Shader(const Device& device, const Shader::Config config) : config(config)
	{
		std::vector<char> moduleCode = readShader(config.file);
		moduleCode.shrink_to_fit();

		VkShaderModuleCreateInfo shaderModuleInfo{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = NULL,
			.flags = {},
			.codeSize = (uint32_t)moduleCode.size(),
			.pCode = reinterpret_cast<uint32_t*>(moduleCode.data())
		};
		shaderModuleHandle = vk::raii::ShaderModule(device.getLogicalDeviceHandle(), shaderModuleInfo);
	}

	const Shader::Config& Shader::getConfig() const
	{
		return config;
	}

	const vk::raii::ShaderModule& Shader::getShaderModuleHandle() const
	{
		return shaderModuleHandle;
	}

}