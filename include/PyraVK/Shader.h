#pragma once
#include <PyraVK/Object.h>

namespace pyra
{

	class Shader : public Object
	{
	public : 
		struct Config : Object::Config
		{
			std::string file;
			std::vector<std::string> vertexShaderEntries; 
			std::vector<std::string> fragmentShaderEntries;
		};
	private :
		vk::raii::ShaderModule shaderModule = nullptr;
		const Shader::Config config;
	public : 
		Shader(const Shader::Config config, const class Device& device);
		const Shader::Config& getConfig() const;
		const vk::raii::ShaderModule& getShaderModule() const; 
	};

}