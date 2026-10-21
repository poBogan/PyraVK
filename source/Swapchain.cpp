#include <PyraVK/Swapchain.h>
#include "PyraVK/Device.h"
#include "PyraVK/VulkanSurface.h"

namespace pyra
{
    Swapchain::Swapchain(const SwapchainConfig& config,const Device& device, const VulkanSurface& surface) : m_config(config)
    {
    	pyra_assert(device.surface_is_compatible(surface));

        auto surface_capabilities = device.get_physical_device().getSurfaceCapabilitiesKHR(surface.get_surface());

		if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			m_config.c_swapchain_image_extent = surface_capabilities.currentExtent;
		}

		else
		{
			m_config.c_swapchain_image_extent.width = std::clamp<uint32_t>(m_config.c_swapchain_image_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
			m_config.c_swapchain_image_extent.height = std::clamp<uint32_t>(m_config.c_swapchain_image_extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
		}

		auto maxImageCount = surface_capabilities.maxImageCount != 0 ? surface_capabilities.maxImageCount : std::numeric_limits<uint32_t>::max();
		m_config.c_swapchain_image_count = std::clamp<uint32_t>(m_config.c_swapchain_image_count, surface_capabilities.minImageCount + 1, maxImageCount);

		std::vector<vk::SurfaceFormatKHR> supported_formats = device.get_physical_device().getSurfaceFormatsKHR(surface.get_surface());
		if (std::ranges::find(supported_formats, vk::SurfaceFormatKHR(m_config.c_swapchain_image_color_format, m_config.c_swapchain_image_color_space)) == supported_formats.end())
		{
			m_config.c_swapchain_image_color_format = supported_formats.front().format;
			m_config.c_swapchain_image_color_space = supported_formats.front().colorSpace;
		}

		std::vector<vk::PresentModeKHR> supported_present_modes = device.get_physical_device().getSurfacePresentModesKHR(surface.get_surface());
		if (std::ranges::find(supported_present_modes, m_config.c_swapchain_present_mode) == supported_present_modes.end())
		{
			m_config.c_swapchain_present_mode = vk::PresentModeKHR::eFifo;
		}

		vk::SwapchainCreateInfoKHR swapchainInfo;
		swapchainInfo.surface = *surface.get_surface();
		swapchainInfo.minImageCount = m_config.c_swapchain_image_count;
		swapchainInfo.imageFormat = m_config.c_swapchain_image_color_format;
		swapchainInfo.imageColorSpace = m_config.c_swapchain_image_color_space;
		swapchainInfo.imageExtent = m_config.c_swapchain_image_extent;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		swapchainInfo.imageSharingMode = vk::SharingMode::eExclusive;
		swapchainInfo.queueFamilyIndexCount = 1;
		swapchainInfo.pQueueFamilyIndices = &device.get_queue_family_index();
		swapchainInfo.preTransform = surface_capabilities.currentTransform;
		swapchainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchainInfo.presentMode = m_config.c_swapchain_present_mode;
		swapchainInfo.clipped = vk::True;
		swapchainInfo.oldSwapchain = nullptr;

		m_swapchain = vk::raii::SwapchainKHR(device.get_logical_device(), swapchainInfo);

		m_swapchain_images = m_swapchain.getImages();
		m_config.c_swapchain_image_count = m_swapchain_images.size();

		for (auto& image : m_swapchain_images)
		{
			vk::ImageViewCreateInfo imageViewInfo;
			imageViewInfo.image = image;
			imageViewInfo.viewType = vk::ImageViewType::e2D;
			imageViewInfo.format = m_config.c_swapchain_image_color_format;
			imageViewInfo.components = { vk::ComponentSwizzle::eIdentity,vk::ComponentSwizzle::eIdentity,vk::ComponentSwizzle::eIdentity,vk::ComponentSwizzle::eIdentity };
			imageViewInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
			m_swapchain_image_views.emplace_back(device.get_logical_device(), imageViewInfo);

			vk::SemaphoreCreateInfo semaphoreInfo;
			m_swapchain_image_presentable_semaphores.emplace_back(device.get_logical_device(), semaphoreInfo);
		}
    }

    const vk::raii::SwapchainKHR& Swapchain::get_swapchain() const
    {
    	return m_swapchain;
    }

    const std::vector<vk::Image>& Swapchain::get_swapchain_images() const
    {
    	return m_swapchain_images;
    }

    const std::vector<vk::raii::ImageView>& Swapchain::get_swapchain_image_views() const
    {
    	return m_swapchain_image_views;
    }

    const std::vector<vk::raii::Semaphore>& Swapchain::get_swapchain_image_presentable_semaphores() const
    {
    	return m_swapchain_image_presentable_semaphores;
    }

    const SwapchainConfig& Swapchain::get_config() const
    {
    	return m_config;
    }

}
