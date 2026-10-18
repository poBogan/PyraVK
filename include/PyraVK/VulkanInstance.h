#pragma once
#include <PyraVK/Object.h>


namespace pyra
{

	class VulkanInstance : public Object
	{
	public : 
		struct Config
		{
			uint32_t vulkan_api_version = vk::ApiVersion13;
			std::vector<const char*> extensions{};
			std::vector<const char*> layers{};
		};
	private : 
		vk::raii::Context context;
		vk::raii::Instance instance = nullptr;
		const VulkanInstance::Config config;
	public:
		explicit VulkanInstance(const VulkanInstance::Config& config);
		[[nodiscard]] const vk::raii::Instance& getInstance() const;
		[[nodiscard]] const VulkanInstance::Config& getConfig() const;
	};

}