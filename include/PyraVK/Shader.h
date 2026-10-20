#pragma once
#include <PyraVK/Object.h>

namespace pyra
{

	struct ShaderConfig
	{
		std::vector<uint32_t> c_shader_code{};
		std::unordered_set<std::string> c_vertex_shader_entries{};
		std::unordered_set<std::string> c_fragment_shader_entries{};
	};

	class Shader : public Object
	{
		vk::raii::ShaderModule m_shader_module = nullptr;
		ShaderConfig m_config;
	public :
		explicit Shader(ShaderConfig&& config, const class EngineContext& engineContext);
		const vk::raii::ShaderModule& get_shader_module() const;
		bool has_vertex_shader_entry(const std::string& vertex_shader_entry) const;
		bool has_fragment_shader_entry(const std::string& fragment_shader_entry) const;
	};

}