#pragma once
#include <PyraVK/Definitions.h>


namespace pyra
{

	class VulkanInstance : public Base
	{
	private : 
		vk::raii::Instance handle = nullptr;

		uint32_t vulkan_api_version = 0;
		std::vector<const char*> extentions{};
		std::vector<const char*> layers{};
	public:
		VulkanInstance(const vk::raii::Context& context,std::vector<const char*> extentions, std::vector<const char*> layers, uint32_t vulkan_version);
		const std::vector<const char*>& getExtensions() const; 
		const std::vector<const char*>& getLayers() const; 
		uint32_t getVulkanVersion() const; 
		const vk::raii::Instance& getVulkanInstanceHandle() const;
	};

}