#include <PyraVK/Renderer.h>


namespace pyra
{

	std::optional<Renderer> Renderer::singletonRenderer;

	void pyra::Renderer::createRenderer()
	{
		if (!singletonRenderer.has_value())
			singletonRenderer.emplace(Renderer());
	}

	Renderer& pyra::Renderer::getRenderer()
	{
		pyra_assert(singletonRenderer.has_value(), "Cannot get renderer. It is not initilaized!");
		return singletonRenderer.value();
	}

	void Renderer::setVulkanInstance(std::vector<const char*> extentions,std::vector<const char*> layers)
	{
		swapchain.reset(); 
		renderDevice.reset(); 
		windowSurface.reset(); 

		vulkanInstance.emplace(VulkanInstance(context, std::move(extentions), std::move(layers), vk::ApiVersion13));
	}

	void Renderer::setWindowSurface(vk::SurfaceKHR surface)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot init surface. No vulkan instance found");

		swapchain.reset();
		renderDevice.reset();

		windowSurface.emplace(Surface(vulkanInstance.value(),surface));
	}

	std::vector<Device> Renderer::getSuitableDevices()
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot fetch devices. No vulkan instance found!"); 
		pyra_assert(windowSurface.has_value(), "Cannot fetch devices. No surface found");
		return Device::getSuitableDevices(vulkanInstance.value(), windowSurface.value(), { vk::KHRSwapchainExtensionName });
	}

	void Renderer::selectDevice(Device&& render_device)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot select device. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot select device. No surface found!");

		swapchain.reset();
		render_device.constructDeviceHandles();
		renderDevice.emplace(std::move(render_device));
	}

	void Renderer::setSwapchain(
		vk::Extent2D window_size_in_pixels,
		uint32_t targetImageCount,
		std::vector<vk::SurfaceFormatKHR> imageFormatPreferences,
		std::vector<vk::PresentModeKHR> presentModePreferences
		)
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot create swapchain. No vulkan instance found!");
		pyra_assert(windowSurface.has_value(), "Cannot create swapchain. No surface found!");
		pyra_assert(renderDevice.has_value(), "Cannot create swapchain. No render device selected!");

		swapchain.emplace(Swapchain(renderDevice.value(), windowSurface.value(), window_size_in_pixels, targetImageCount, imageFormatPreferences, presentModePreferences));
	}

	const vk::raii::Context& Renderer::getContext() const
	{
		return context;
	}
	const VulkanInstance& Renderer::getVulkanInstance() const
	{
		pyra_assert(vulkanInstance.has_value(), "Cannot return vulkan instance! No instance found");
		return vulkanInstance.value();
	}
	const Surface& Renderer::getWindowSurface() const
	{
		pyra_assert(windowSurface.has_value(), "Cannot return surface. No surface found");
		return windowSurface.value();
	}

	const Device& Renderer::getRenderDevice() const
	{
		pyra_assert(renderDevice.has_value(), "Cannot return render device. No device selected!");
		return renderDevice.value();
	}

	const Swapchain& Renderer::getSwapchain() const
	{
		pyra_assert(swapchain.has_value(), "Cannot return swapchain. No swapchain found!");
		return swapchain.value();
	}

}