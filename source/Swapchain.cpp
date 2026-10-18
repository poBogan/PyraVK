#include <PyraVK/Swapchain.h>
#include <PyraVK/Device.h>
#include <PyraVK/Surface.h>

namespace pyra
{

	Swapchain::Swapchain(const Swapchain::Config& config, const Device& device, const Surface& surface) : Object(), config(config)
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
		pyra_assert(!supported_formats.empty());
		if (std::ranges::find(supported_formats, vk::SurfaceFormatKHR(config.imageFormat, config.colorSpace)) == supported_formats.end())
		{
			this->config.imageFormat = supported_formats.front().format; 
			this->config.colorSpace = supported_formats.front().colorSpace; 
		}

		std::vector<vk::PresentModeKHR> supported_present_modes = device.getPhysicalDevice().getSurfacePresentModesKHR(surface.getSurface());
		pyra_assert(!supported_present_modes.empty());
		if (std::ranges::find(supported_present_modes, config.presentMode) == supported_present_modes.end())
		{
			this->config.presentMode = vk::PresentModeKHR::eFifo;
		}		

		vk::SwapchainCreateInfoKHR swapchainInfo;
		swapchainInfo.surface = *surface.getSurface();
		swapchainInfo.minImageCount = this->config.imageCount;
		swapchainInfo.imageFormat = this->config.imageFormat;
		swapchainInfo.imageColorSpace = this->config.colorSpace;
		swapchainInfo.imageExtent = this->config.imageExtent;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
		swapchainInfo.imageSharingMode = vk::SharingMode::eExclusive;
		swapchainInfo.queueFamilyIndexCount = 1;
		swapchainInfo.pQueueFamilyIndices = &device.getQueueFamilyIndex();
		swapchainInfo.preTransform = surface_capabilities.currentTransform;
		swapchainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		swapchainInfo.presentMode = this->config.presentMode;
		swapchainInfo.clipped = vk::True;
		swapchainInfo.oldSwapchain = nullptr;

		swapchain = vk::raii::SwapchainKHR(device.getLogicalDevice(), swapchainInfo);

		images = swapchain.getImages();
		this->config.imageCount = images.size();

		for (auto& image : images)
		{
			vk::ImageViewCreateInfo imageViewInfo;
			imageViewInfo.image = image;
			imageViewInfo.viewType = vk::ImageViewType::e2D;
			imageViewInfo.format = this->config.imageFormat;
			imageViewInfo.components = { vk::ComponentSwizzle::eIdentity,vk::ComponentSwizzle::eIdentity,vk::ComponentSwizzle::eIdentity,vk::ComponentSwizzle::eIdentity };
			imageViewInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
			imageViews.emplace_back(device.getLogicalDevice(), imageViewInfo);

			vk::SemaphoreCreateInfo semaphoreInfo;
			imagePresentableSemaphores.emplace_back(device.getLogicalDevice(), semaphoreInfo);
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