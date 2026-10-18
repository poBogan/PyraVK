#pragma once
#include <PyraVK/Object.h>

namespace pyra
{

	class Swapchain : public Object
	{
	public : 
		struct Config
		{
			vk::Extent2D imageExtent{};
			uint32_t imageCount = 0;
			vk::Format imageFormat{};
			vk::ColorSpaceKHR colorSpace{};
			vk::PresentModeKHR presentMode{};
		};
	private:
		Swapchain::Config config;
		vk::raii::SwapchainKHR swapchain = nullptr;
		std::vector<vk::Image> images;
		std::vector<vk::raii::ImageView> imageViews;
		std::vector<vk::raii::Semaphore> imagePresentableSemaphores;
	public : 
		explicit  Swapchain(const Swapchain::Config& config, const class Device& device,const class Surface& surface);

		[[nodiscard]] const vk::raii::SwapchainKHR& getSwapchain() const;
		[[nodiscard]] const std::vector<vk::Image>& getImages() const;
		[[nodiscard]] const std::vector<vk::raii::ImageView>& getImageViews() const;
		[[nodiscard]] const std::vector<vk::raii::Semaphore>& getImagePresentableSemaphores() const;
		[[nodiscard]] const Swapchain::Config& getConfig() const;
	};
}