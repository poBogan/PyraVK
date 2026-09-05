#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace pyra
{

	class Renderer
	{
	private :
		static Renderer* singletonRenderer;
		Renderer(const char* const* required_instance_extentions,uint32_t instance_extention_count);
	private : 
		vk::raii::Context context;
		vk::raii::Instance vulkanInstance = nullptr;
	public : 
		Renderer() = delete; 
		Renderer(Renderer&) = delete;

		static void createRenderer(std::vector<const char*> required_instance_extentions);
		static Renderer* getRenderer();
	};
}