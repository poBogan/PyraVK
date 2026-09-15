#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/VulkanInstance.h>
#include <PyraVK/Surface.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>
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
		std::optional<GraphicsPipeline> graphicsPipeline;
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

		void constructGraphicsPipeline(
			vk::Viewport viewport,
			vk::Rect2D scissor,
			vk::PrimitiveTopology primitiveTopology = vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits faceCulling = vk::CullModeFlagBits::eBack,
			vk::FrontFace frontFace = vk::FrontFace::eClockwise,
			vk::PolygonMode polygonMode = vk::PolygonMode::eFill,
			vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1
		);

		void drawFrames();

		void destroyVulkanInstance();
		void destroyWindowSurface();
		void destroyRenderDevice();
		void destroySwapchain();
		void destroyGraphicsPipeline();

		const vk::raii::Context& getContext() const;
		const VulkanInstance& getVulkanInstance() const;
		const Surface& getWindowSurface() const;
		const Device& getRenderDevice() const;
		const Swapchain& getSwapchain() const;
		const GraphicsPipeline& getGraphicsPipeline() const;

		bool vulkanInstanceExists() const; 
		bool windowSurfaceExists() const;
		bool renderDeviceExists() const; 
		bool swapchainExists() const; 
		bool graphicsPipelineExists() const;
	};

};