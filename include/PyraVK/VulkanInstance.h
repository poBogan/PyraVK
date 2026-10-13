#pragma once
#include <PyraVK/Object.h>


namespace pyra
{

	class VulkanInstance : public Object
	{
	public : 
		struct Config : Object::Config
		{
			uint32_t vulkan_api_version = vk::ApiVersion13;
			std::vector<const char*> extentions{};
			std::vector<const char*> layers{};
		};
	private : 
		vk::raii::Context context;
		vk::raii::Instance instance = nullptr;
		const VulkanInstance::Config config;
	public:
		VulkanInstance(const VulkanInstance::Config config);
		const vk::raii::Instance& getInstance() const;
		const VulkanInstance::Config& getConfig() const;
	};

}