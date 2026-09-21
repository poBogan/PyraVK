#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>
#include <PyraVK/Surface.h>

namespace pyra
{

	class Swapchain : public Base
	{
	public : 
		struct Config
		{
			vk::Extent2D imageExtent;
			uint32_t imageCount;
			vk::Format imageFormat; 
			vk::ColorSpaceKHR colorSpace; 
			vk::PresentModeKHR presentMode;
		};
	private:
		vk::raii::SwapchainKHR handle = nullptr;
		std::vector<vk::Image> images;
		std::vector<vk::raii::ImageView> imageViews;
		std::vector<vk::raii::Semaphore> imagePresentableSemaphores;
		Swapchain::Config config;
	public : 
		Swapchain(const Device& device,const Surface& surface,const Swapchain::Config config);

		const Swapchain::Config& getConfig() const;
		const vk::raii::SwapchainKHR& getSwapchainHandle() const;
		const std::vector<vk::Image>& getImages() const; 
		const std::vector<vk::raii::ImageView>& getImageViews() const;
		const std::vector<vk::raii::Semaphore>& getImagePresentableSemaphores() const;
	};
}