#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>

namespace pyra
{

	class Shader : public Base
	{
	public : 
		struct Config
		{
			std::string file;
			std::vector<std::string> vertexShaderEntries; 
			std::vector<std::string> fragmentShaderEntries;
		};
	private :
		vk::raii::ShaderModule shaderModuleHandle = nullptr;
		const Shader::Config config;
	public : 
		Shader(const Device& device,const Shader::Config config);
		const Shader::Config& getConfig() const;
		const vk::raii::ShaderModule& getShaderModuleHandle() const; 
	};

}