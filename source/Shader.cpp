#include <PyraVK/Shader.h>
#include <PyraVK/EngineContext.h>
#include <utility>

namespace pyra
{
	Shader::Shader(ShaderConfig&& config, const EngineContext& engineContext) : m_config(std::move(config))
	{
		pyra_assert(m_config.c_shader_code.empty() == false);

		vk::ShaderModuleCreateInfo shaderModuleInfo;
		shaderModuleInfo.codeSize = static_cast<uint32_t>(m_config.c_shader_code.size() * sizeof(uint32_t)),
		shaderModuleInfo.pCode = m_config.c_shader_code.data();

		m_shader_module = vk::raii::ShaderModule(engineContext.getDevice().get_logical_device(), shaderModuleInfo);
		m_config.c_shader_code.clear();
	}

	const vk::raii::ShaderModule& Shader::get_shader_module() const
	{
		return m_shader_module;
	}

	bool Shader::has_vertex_shader_entry(const std::string& vertex_shader_entry) const
	{
		return m_config.c_vertex_shader_entries.contains(vertex_shader_entry);
	}

	bool Shader::has_fragment_shader_entry(const std::string& fragment_shader_entry) const
	{
		return m_config.c_fragment_shader_entries.contains(fragment_shader_entry);
	}
}
