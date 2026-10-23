#pragma once
#include <PyraVK/PyraBase.h>

namespace pyra
{

	struct ShaderConfig
	{
		std::vector<uint32_t> c_shader_code{};
		std::unordered_set<std::string> c_vertex_shader_entries{};
		std::unordered_set<std::string> c_fragment_shader_entries{};
	};

	class Shader : public PyraBase
	{
		vk::raii::ShaderModule m_shader_module = nullptr;
		std::unordered_set<std::string> m_vertex_shader_entries{};
		std::unordered_set<std::string> m_fragment_shader_entries{};
	public :
		Shader(const ShaderConfig& config, const class Device& device);
		const vk::raii::ShaderModule& get_shader_module() const;
		const std::unordered_set<std::string>& get_vertex_shader_entries() const;
		const std::unordered_set<std::string>& get_fragment_shader_entries() const;
	};

}