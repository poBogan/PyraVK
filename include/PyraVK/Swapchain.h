#pragma once
#include <PyraVK/Object.h>

namespace pyra
{

	class Swapchain : public Object
	{
	public : 
		struct Config : Object::Config
		{
			vk::Extent2D imageExtent;
			uint32_t imageCount;
			vk::Format imageFormat; 
			vk::ColorSpaceKHR colorSpace; 
			vk::PresentModeKHR presentMode;
		};
	private:
		Swapchain::Config config;
		vk::raii::SwapchainKHR swapchain = nullptr;
		std::vector<vk::Image> images;
		std::vector<vk::raii::ImageView> imageViews;
		std::vector<vk::raii::Semaphore> imagePresentableSemaphores;
	public : 
		Swapchain(const Swapchain::Config config, const class Device& device,const class Surface& surface);

		const vk::raii::SwapchainKHR& getSwapchain() const;
		const std::vector<vk::Image>& getImages() const; 
		const std::vector<vk::raii::ImageView>& getImageViews() const;
		const std::vector<vk::raii::Semaphore>& getImagePresentableSemaphores() const;
		const Swapchain::Config& getConfig() const;
	};
}