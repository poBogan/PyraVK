#pragma once
#include <PyraVK/Definitions.h>


namespace pyra
{

	class VulkanInstance : public Base
	{
	public : 
		struct Config
		{
			const uint32_t vulkan_api_version = vk::ApiVersion13;
			std::vector<const char*> extentions{};
			std::vector<const char*> layers{};
		};
	private : 
		vk::raii::Context context;
		vk::raii::Instance handle = nullptr;
		const VulkanInstance::Config config;
	public:
		VulkanInstance(const VulkanInstance::Config config);
		const VulkanInstance::Config& getConfig() const;
		const vk::raii::Instance& getVulkanInstanceHandle() const;
	};

}