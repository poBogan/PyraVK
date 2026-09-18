#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/VulkanInstance.h>
#include <PyraVK/Surface.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>
#include <PyraVk/Shader.h>
#include <PyraVK/GraphicsPipeline.h>

namespace pyra
{

	class Renderer : public Base
	{
	private:
		static std::optional<Renderer> singletonRenderer;
		vk::raii::Context context;
	private : 
		std::optional<VulkanInstance> vulkanInstance;
		std::optional<Surface> windowSurface; 
		std::optional<Device> renderDevice; 
		std::optional<Swapchain> swapchain;
		std::unordered_map<std::string, std::shared_ptr<Shader>> shaderEntries;
		std::unordered_map<std::string, GraphicsPipeline> graphicsPipelines;
	public:
		static void createRenderer();
		static Renderer& getRenderer();

		bool isExtentionSupported(const char* instance_extention); 
		bool isLayerSupported(const char* instance_layer);

		void constructVulkanInstance(std::vector<const char*> extentions, std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"});
		void constructWindowSurface(VkSurfaceKHR surface);

		std::vector<Device> getSuitableRenderDevices();
		void selectRenderDevice(Device&& render_device);

		void constructSwapchain(
			vk::Extent2D window_size_in_pixels,
			uint32_t targetImageCount = 3,
			std::vector<vk::SurfaceFormatKHR> imageFormatPreferences = { {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear},{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear}},
			std::vector<vk::PresentModeKHR> presentModePreferences = {vk::PresentModeKHR::eMailbox}
		);

		void addShaderEntries(std::string file, std::vector<std::string>vertexShaderEntries, std::vector<std::string>fragmentShaderEntries);
		
		void addGraphicsPipeline(
			std::string pipeline_name,
			std::string vertexShaderEntry,
			std::string fragmentShaderEntry,
			vk::PrimitiveTopology primitiveTopology = vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits faceCullingMode = vk::CullModeFlagBits::eBack,
			vk::FrontFace frontFace = vk::FrontFace::eClockwise,
			vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1
		);

		void destroyVulkanInstance();
		void destroyWindowSurface();
		void destroyRenderDevice();
		void destroySwapchain(); 
		void destroyGraphicsPipelines(); 
		void deleteGraphicsPipeline(std::string name);

		const vk::raii::Context& getContext() const;
		const std::optional<VulkanInstance>& getVulkanInstance() const;
		const std::optional<Surface>& getWindowSurface() const;
		const std::optional<Device>& getRenderDevice() const;
		const std::optional<Swapchain>& getSwapchain() const;
		const std::unordered_map<std::string, std::shared_ptr<Shader>>& getShaderEntries() const;
		const std::unordered_map<std::string, GraphicsPipeline>& getGraphicsPipelines() const;

	};

};