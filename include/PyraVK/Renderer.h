#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/VulkanInstance.h>
#include <PyraVK/Surface.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>

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
	public:
		static void createRenderer();
		static Renderer& getRenderer();

		bool isExtentionSupported(const char* instance_extention); 
		bool isLayerSupported(const char* instance_layer);

		void setVulkanInstance(std::vector<const char*> extentions, std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"});

		void setWindowSurface(vk::SurfaceKHR surface);

		std::vector<Device> getSuitableDevices();
		void selectDevice(Device&& render_device);

		void setSwapchain(
			vk::Extent2D window_size_in_pixels,
			uint32_t targetImageCount = 3,
			std::vector<vk::SurfaceFormatKHR> imageFormatPreferences = {{vk::Format::eB8G8R8A8Snorm, vk::ColorSpaceKHR::eSrgbNonlinear},{vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear}},
			std::vector<vk::PresentModeKHR> presentModePreferences = {vk::PresentModeKHR::eMailbox}
		);

		const vk::raii::Context& getContext() const;
		const VulkanInstance& getVulkanInstance() const;
		const Surface& getWindowSurface() const;
		const Device& getRenderDevice() const;
		const Swapchain& getSwapchain() const;
	};

};