#include <PyraVK/Shader.h>
#include "PyraVK/Device.h"

namespace pyra
{
	Shader::Shader(const ShaderConfig& config, const Device& device, const VulkanSurface& surface)
	{
		pyra_assert(config.c_shader_code.empty() == false);

		vk::ShaderModuleCreateInfo shaderModuleInfo;
		shaderModuleInfo.codeSize = static_cast<uint32_t>(config.c_shader_code.size() * sizeof(uint32_t)),
		shaderModuleInfo.pCode = config.c_shader_code.data();

		m_shader_module = vk::raii::ShaderModule(device.get_logical_device(), shaderModuleInfo);

		for (const auto& vertex_shader_entry : config.c_vertex_shader_entries)
		{
			m_vertex_shader_entries.insert(vertex_shader_entry);
		}

		for (const auto& fragment_shader_entry : config.c_fragment_shader_entries)
		{
			m_fragment_shader_entries.insert(fragment_shader_entry);
		}
	}

	const vk::raii::ShaderModule& Shader::get_shader_module() const
	{
		return m_shader_module;
	}

	bool Shader::has_vertex_shader_entry(const std::string& vertex_shader_entry) const
	{
		return m_vertex_shader_entries.contains(vertex_shader_entry);
	}

	bool Shader::has_fragment_shader_entry(const std::string& fragment_shader_entry) const
	{
		return m_fragment_shader_entries.contains(fragment_shader_entry);
	}
}
