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

		vk::SwapchainCreateInfoKHR swapchainInfo;
		swapchainInfo.surface = *surface.getSurfaceHandle();
		swapchainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageColorSpace = swapchainImageFormat.colorSpace;
		swapchainInfo.imageExtent = swapchainImageExtent;
		swapchainInfo.imageFormat = swapchainImageFormat.format;
		swapchainInfo.presentMode = swapchainPresentMode;
		swapchainInfo.imageSharingMode = vk::SharingMode::eExclusive; 
		swapchainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment; 
		swapchainInfo.minImageCount = targetImageCount;
		swapchainInfo.clipped = true; 
		swapchainInfo.preTransform = surface_capabilities.currentTransform;
		swapchainInfo.oldSwapchain = nullptr;

		handle = vk::raii::SwapchainKHR(renderDevice.getLogicalDeviceHandle(), swapchainInfo);

		swapchainImages = handle.getImages();

		for (auto& image : swapchainImages)
		{
			vk::ImageSubresourceRange r; 
			r.aspectMask = vk::ImageAspectFlagBits::eColor;
			r.layerCount = 1; 
			r.levelCount = 1;

			vk::ImageViewCreateInfo imageViewInfo;
			imageViewInfo.viewType = vk::ImageViewType::e2D; 
			imageViewInfo.format = swapchainImageFormat.format; 
			imageViewInfo.image = image;
			imageViewInfo.components = { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity ,vk::ComponentSwizzle::eIdentity ,vk::ComponentSwizzle::eIdentity };
			imageViewInfo.subresourceRange = r;

			swapchainImageViews.push_back(vk::raii::ImageView(renderDevice.getLogicalDeviceHandle(), imageViewInfo));
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