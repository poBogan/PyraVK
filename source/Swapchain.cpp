#include <PyraVK/Swapchain.h>
#include <PyraVK/Device.h>
#include <PyraVK/Surface.h>

namespace pyra
{

	Swapchain::Swapchain(const Swapchain::Config config, const Device& device, const Surface& surface) : Object(config, Object::Type::SWAPCHAIN), config(config)
	{
		auto surface_capabilities = device.getPhysicalDevice().getSurfaceCapabilitiesKHR(surface.getSurface());

		if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			this->config.imageExtent = surface_capabilities.currentExtent;
		}

		else
		{
			this->config.imageExtent.width = std::clamp<uint32_t>(this->config.imageExtent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
			this->config.imageExtent.height = std::clamp<uint32_t>(this->config.imageExtent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
		}

		auto maxImageCount = surface_capabilities.maxImageCount != 0 ? surface_capabilities.maxImageCount : std::numeric_limits<uint32_t>::max();
		this->config.imageCount = std::clamp<uint32_t>(this->config.imageCount, surface_capabilities.minImageCount + 1, maxImageCount);

		std::vector<vk::SurfaceFormatKHR> supported_formats = device.getPhysicalDevice().getSurfaceFormatsKHR(surface.getSurface());
		pyra_assert(!supported_formats.empty(), "Cannot create swapchain.No supported image formats");
		if (std::ranges::find(supported_formats, vk::SurfaceFormatKHR(config.imageFormat, config.colorSpace)) == supported_formats.end())
		{
			this->config.imageFormat = supported_formats.front().format; 
			this->config.colorSpace = supported_formats.front().colorSpace; 
		}

		std::vector<vk::PresentModeKHR> supported_present_modes = device.getPhysicalDevice().getSurfacePresentModesKHR(surface.getSurface());
		pyra_assert(!supported_present_modes.empty(), "Cannot create swapchain.No supported present modes");
		if (std::ranges::find(supported_present_modes, config.presentMode) == supported_present_modes.end())
		{
			this->config.presentMode = vk::PresentModeKHR::eFifo;
		}		

		VkSwapchainCreateInfoKHR swapchainInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = NULL,
			.flags = NULL,
			.surface = *surface.getSurface(),
			.minImageCount = this->config.imageCount,
			.imageFormat = (VkFormat)this->config.imageFormat,
			.imageColorSpace = (VkColorSpaceKHR)this->config.colorSpace,
			.imageExtent = (VkExtent2D)this->config.imageExtent,
			.imageArrayLayers = 1,
			.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &device.getQueueFamilyIndex(),
			.preTransform = (VkSurfaceTransformFlagBitsKHR)surface_capabilities.currentTransform,
			.compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = (VkPresentModeKHR)this->config.presentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		swapchain = vk::raii::SwapchainKHR(device.getLogicalDevice(), vk::SwapchainCreateInfoKHR(swapchainInfo));

		images = swapchain.getImages();
		this->config.imageCount = (uint32_t)images.size();

		for (auto& image : images)
		{
			
			VkImageViewCreateInfo imageViewInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = NULL,
				.flags = NULL,
				.image = (VkImage)image,
				.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
				.format = (VkFormat)this->config.imageFormat,
				.components = { VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY ,VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY },
				.subresourceRange = {
					.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};
			imageViews.push_back(vk::raii::ImageView(device.getLogicalDevice(), vk::ImageViewCreateInfo(imageViewInfo)));

			VkSemaphoreCreateInfo semaphoreInfo{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = NULL,
				.flags = {}
			};
			imagePresentableSemaphores.push_back(vk::raii::Semaphore(device.getLogicalDevice(), vk::SemaphoreCreateInfo(semaphoreInfo)));
		}

	}

	const Swapchain::Config& Swapchain::getConfig() const
	{
		return config;
	}

	const vk::raii::SwapchainKHR& Swapchain::getSwapchain() const
	{
		return swapchain;
	}

	const std::vector<vk::Image>& Swapchain::getImages() const
	{
		return images;
	}

	const std::vector<vk::raii::ImageView>& Swapchain::getImageViews() const
	{
		return imageViews;
	}

	const std::vector<vk::raii::Semaphore>& Swapchain::getImagePresentableSemaphores() const
	{
		return imagePresentableSemaphores;
	}

}