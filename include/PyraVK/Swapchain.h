#pragma once
#include <PyraVK/PyraBase.h>

namespace pyra
{

	struct SwapchainConfig
	{
		vk::Extent2D c_swapchain_image_extent{};
		uint32_t c_swapchain_image_count{};
		vk::Format c_swapchain_image_color_format{};
		vk::ColorSpaceKHR c_swapchain_image_color_space{};
		vk::PresentModeKHR c_swapchain_present_mode{};
	};

	class Swapchain : public PyraBase
	{
		SwapchainConfig m_config;
		vk::raii::SwapchainKHR m_swapchain = nullptr;
		std::vector<vk::Image> m_swapchain_images;
		std::vector<vk::raii::ImageView> m_swapchain_image_views;
		std::vector<vk::raii::Semaphore> m_swapchain_image_presentable_semaphores;
	public : 
		explicit Swapchain(const SwapchainConfig& config,const class Device& device, const class VulkanSurface& surface);
		[[nodiscard]] const vk::raii::SwapchainKHR& get_swapchain() const;
		[[nodiscard]] const std::vector<vk::Image>& get_swapchain_images() const;
		[[nodiscard]] const std::vector<vk::raii::ImageView>& get_swapchain_image_views() const;
		[[nodiscard]] const std::vector<vk::raii::Semaphore>& get_swapchain_image_presentable_semaphores() const;
		[[nodiscard]] const SwapchainConfig& get_config() const;
	};
}