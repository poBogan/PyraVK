#include <PyraVK/Swapchain.h>


namespace pyra
{

	Swapchain::Swapchain(
		Device& renderDevice, 
		Surface& surface, 
		vk::Extent2D windowSizeInPixels, 
		uint32_t swapchainImageCount, 
		std::vector<vk::SurfaceFormatKHR> imageFormatPreferences,
		std::vector<vk::PresentModeKHR> presentModePreferences
	)
	{
		auto surface_capabilities = renderDevice.getPhysicalDeviceHandle().getSurfaceCapabilitiesKHR(*surface.getSurfaceHandle());

		if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			swapchainImageExtent = surface_capabilities.currentExtent;
		}

		else
		{
			swapchainImageExtent.width = std::clamp<uint32_t>(windowSizeInPixels.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
			swapchainImageExtent.height = std::clamp<uint32_t>(windowSizeInPixels.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
		}

		auto maxImageCount = surface_capabilities.maxImageCount != 0 ? surface_capabilities.maxImageCount : std::numeric_limits<uint32_t>::max();
		auto targetImageCount = std::clamp<uint32_t>(swapchainImageCount, surface_capabilities.minImageCount + 1, maxImageCount);


		auto supported_image_formats = renderDevice.getPhysicalDeviceHandle().getSurfaceFormatsKHR(*surface.getSurfaceHandle());
		pyra_assert(!supported_image_formats.empty(), "No image format compatible with both render device and surface");
		swapchainImageFormat = supported_image_formats[0];
		for (auto& prefered : imageFormatPreferences)
		{
			if (std::ranges::any_of(supported_image_formats, [&prefered](vk::SurfaceFormatKHR& supported) ->bool {return supported == prefered; }))
			{
				swapchainImageFormat = prefered;
				break;
			}
		}

		auto supported_present_modes = renderDevice.getPhysicalDeviceHandle().getSurfacePresentModesKHR(*surface.getSurfaceHandle());
		pyra_assert(!supported_present_modes.empty(), "Cannot find a present mode both compatible with the render device and surface");
		auto swapchainPresentMode = vk::PresentModeKHR::eFifo;
		for (auto& prefered : presentModePreferences)
		{
			if (std::ranges::any_of(supported_present_modes, [&prefered](vk::PresentModeKHR& supported) -> bool {return supported == prefered;}))
			{
				swapchainPresentMode = prefered;
				break;
			}
		}

		VkSwapchainCreateInfoKHR swapchainInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = NULL,
			.flags = NULL,
			.surface = *surface.getSurfaceHandle(),
			.minImageCount = targetImageCount,
			.imageFormat = (VkFormat)swapchainImageFormat.format,
			.imageColorSpace = (VkColorSpaceKHR)swapchainImageFormat.colorSpace,
			.imageExtent = (VkExtent2D)swapchainImageExtent,
			.imageArrayLayers = 1,
			.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = (VkSurfaceTransformFlagBitsKHR)surface_capabilities.currentTransform,
			.compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = (VkPresentModeKHR)swapchainPresentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		handle = vk::raii::SwapchainKHR(renderDevice.getLogicalDeviceHandle(), vk::SwapchainCreateInfoKHR(swapchainInfo));

		swapchainImages = handle.getImages();

		for (auto& image : swapchainImages)
		{
			
			VkImageViewCreateInfo imageViewInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.pNext = NULL,
				.flags = NULL,
				.image = (VkImage)image,
				.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
				.format = (VkFormat)swapchainImageFormat.format,
				.components = { VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY ,VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY },
				.subresourceRange = {
					.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};
			swapchainImageViews.push_back(vk::raii::ImageView(renderDevice.getLogicalDeviceHandle(), vk::ImageViewCreateInfo(imageViewInfo)));

			VkSemaphoreCreateInfo semaphoreInfo{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = NULL,
				.flags = {}
			};
			imagePresentableSemaphores.push_back(vk::raii::Semaphore(renderDevice.getLogicalDeviceHandle(), vk::SemaphoreCreateInfo(semaphoreInfo)));
		}

	}

	const vk::raii::SwapchainKHR& Swapchain::getSwapchainHandle() const
	{
		return handle;
	}

	const std::vector<vk::Image>& Swapchain::getSwapchainImages() const
	{
		return swapchainImages;
	}

	const std::vector<vk::raii::ImageView>& Swapchain::getSwapchainImageViews() const
	{
		return swapchainImageViews;
	}

	const std::vector<vk::raii::Semaphore>& Swapchain::getImagePresentableSemaphores() const
	{
		return imagePresentableSemaphores;
	}

	vk::Extent2D Swapchain::getSwapchainImageExtent() const
	{
		return swapchainImageExtent;
	}

	uint32_t Swapchain::getSwapchainImageCount() const
	{
		return (uint32_t)swapchainImages.size();
	}

	vk::SurfaceFormatKHR Swapchain::getSwapchainImageFormat() const
	{
		return swapchainImageFormat;
	}


}