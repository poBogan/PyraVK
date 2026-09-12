#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/Device.h>

namespace pyra
{

	class Swapchain : public Base
	{
	private : 
		vk::raii::SwapchainKHR handle = nullptr; 
		std::vector<vk::Image> swapchainImages; 
		std::vector<vk::raii::ImageView> swapchainImageViews;

		vk::Extent2D swapchainImageExtent;
		vk::SurfaceFormatKHR swapchainImageFormat;
	public : 
		Swapchain(
			Device& renderDevice, 
			Surface& surface, 
			vk::Extent2D windowSizeInPixels,
			uint32_t targetImageCount, 
			std::vector<vk::SurfaceFormatKHR> imageFormatPreferences,
			std::vector<vk::PresentModeKHR> presentModePreferences
		);
		
		const vk::raii::SwapchainKHR& getSwapchainHandle() const;
		const std::vector<vk::Image>& getSwapchainImages() const; 
		const std::vector<vk::raii::ImageView>& getSwapchainImageViews() const;

		vk::Extent2D getSwapchainImageExtent() const; 
		uint32_t getSwapchainImageCount() const;
		vk::SurfaceFormatKHR getSwapchainImageFormat() const;
	};
}