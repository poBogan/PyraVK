#include <PyraVK/Shader.h>
#include <PyraVK/Device.h>
#include <fstream>

namespace pyra
{
	static std::vector<char> readShader(const std::string& fileName)
	{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			spdlog::error("Shader file \"{:}\" could not be opened!",fileName.c_str());
			return {};
		}

		const int fileSize = file.tellg();
		file.seekg(std::ios::beg);

		std::vector<char> buffer(fileSize);
		file.read(buffer.data(), fileSize);

		return buffer;
	}

	Shader::Shader(const Shader::Config& config, const class Device& device) : Object(), config(config)
	{
		std::vector<char> moduleCode = readShader(config.file);
		pyra_assert(!moduleCode.empty());
		moduleCode.shrink_to_fit();

		vk::ShaderModuleCreateInfo shaderModuleInfo;
		shaderModuleInfo.codeSize = static_cast<uint32_t>(moduleCode.size()),
		shaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(moduleCode.data()),

		shaderModule = vk::raii::ShaderModule(device.getLogicalDevice(), shaderModuleInfo);
	}

	const Shader::Config& Shader::getConfig() const
	{
		return config;
	}

	const vk::raii::ShaderModule& Shader::getShaderModule() const
	{
		return shaderModule;
	}

}