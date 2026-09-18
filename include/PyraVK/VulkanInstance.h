#pragma once
#include <PyraVK/Definitions.h>


namespace pyra
{

	class VulkanInstance : public Base
	{
	private : 
		vk::raii::Instance handle = nullptr;

		const uint32_t vulkan_api_version = vk::ApiVersion13;
		std::vector<const char*> extentions{};
		std::vector<const char*> layers{};
	public:
		VulkanInstance(const vk::raii::Context& context,std::vector<const char*> extentions, std::vector<const char*> layers);
		std::vector<const char*> getExtentionNames() const; 
		std::vector<const char*> getLayerNames() const; 
		uint32_t getVulkanVersion() const; 
		const vk::raii::Instance& getVulkanInstanceHandle() const;
	};

}