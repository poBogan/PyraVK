#pragma once
#include <PyraVK/Definitions.h>


namespace pyra
{

	class VulkanInstance
	{
		vk::raii::Context context;
		vk::raii::Instance instance_obj = nullptr;

		uint32_t vulkan_api_version = 0;
		std::vector<const char*> extentions{};
		std::vector<const char*> layers{};
	private:
		VulkanInstance(std::vector<const char*> extentions, std::vector<const char*> layers, uint32_t vulkan_version);
	public:
		VulkanInstance() = delete;
		VulkanInstance(VulkanInstance&) = delete;
		VulkanInstance(VulkanInstance&&) = delete;

		static bool extentionsSupported(std::vector<const char*> extentions);
		static bool layersSupported(std::vector<const char*> layers);
		static std::unique_ptr<VulkanInstance> createVulkanInstance(std::vector<const char*> extentions, std::vector<const char*> layers, uint32_t vulkan_version = vk::ApiVersion13);

		const std::vector<const char*>& getExtensions() const; 
		const std::vector<const char*>& getLayers() const; 
		uint32_t getVulkanVersion() const; 
		const vk::raii::Instance& getInstanceObject() const;
	};

}