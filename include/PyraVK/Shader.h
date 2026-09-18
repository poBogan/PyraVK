#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>

namespace pyra
{

	class Shader : public Base
	{
	private :
		vk::raii::ShaderModule shaderModule = nullptr;
		std::string file;
		std::vector<std::string> vertexShaderEntries; 
		std::vector<std::string> fragmentShaderEntries;
	public : 
		Shader(Device& device,std::string file,std::vector<std::string>vertexShaderEntries, std::vector<std::string>fragmentShaderEntries);

		const vk::raii::ShaderModule& getShaderModuleHandle() const;
		std::string getFileName() const;
		std::vector<std::string> getVertexShaderEntries() const; 
		std::vector<std::string> getFragmentShaderEntries() const;
		bool hasVertexShaderEntry(std::string entry) const; 
		bool hasFragmentShaderEntry(std::string entry) const; 
	};

}