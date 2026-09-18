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

	pyra::Shader::Shader(Device& device,std::string file,std::vector<std::string> vertexShaderEntries, std::vector<std::string> fragmentShaderEntries)
	{
		std::vector<char> moduleCode = readShader(file);
		moduleCode.shrink_to_fit();

		VkShaderModuleCreateInfo shaderModuleInfo{
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = NULL,
			.flags = {},
			.codeSize = (uint32_t)moduleCode.size(),
			.pCode = reinterpret_cast<uint32_t*>(moduleCode.data())
		};

		shaderModule = vk::raii::ShaderModule(device.getLogicalDeviceHandle(), shaderModuleInfo);

		this->vertexShaderEntries = vertexShaderEntries;
		this->fragmentShaderEntries = fragmentShaderEntries;
	}

	const vk::raii::ShaderModule& pyra::Shader::getShaderModuleHandle() const
	{
		return shaderModule;
	}

	std::string Shader::getFileName() const
	{
		return file;
	}

	std::vector<std::string> Shader::getVertexShaderEntries() const
	{
		return vertexShaderEntries;
	}

	std::vector<std::string> Shader::getFragmentShaderEntries() const
	{
		return fragmentShaderEntries;
	}

	bool pyra::Shader::hasVertexShaderEntry(std::string entry) const
	{
		return std::ranges::any_of(vertexShaderEntries, [&entry](const std::string& iteratedEntry) -> bool {return iteratedEntry == entry;});
	}
	bool pyra::Shader::hasFragmentShaderEntry(std::string entry) const
	{
		return std::ranges::any_of(fragmentShaderEntries, [&entry](const std::string& iteratedEntry) -> bool {return iteratedEntry == entry;});

	}
}