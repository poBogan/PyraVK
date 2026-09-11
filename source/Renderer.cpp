#include <PyraVK/Renderer.h>


namespace pyra
{

	std::unique_ptr<Renderer> Renderer::singletonRenderer = nullptr;

	const std::unique_ptr<Renderer>& Renderer::createRenderer(std::unique_ptr<VulkanInstance>&& instance, std::unique_ptr<Surface>&& surface)
	{
		if (singletonRenderer == nullptr)
		{
			singletonRenderer = std::unique_ptr<Renderer>(new Renderer(std::move(instance), std::move(surface)));
		}

		return singletonRenderer;
	}

	const std::unique_ptr<Renderer>& Renderer::getRenderer()
	{
		return singletonRenderer;
	}

	Renderer::Renderer(std::unique_ptr<VulkanInstance>&& instance, std::unique_ptr<Surface>&& surface)
	{
		this->vulkanInstance = std::move(instance);
		this->surface = std::move(surface);
	}

	std::vector<std::unique_ptr<RenderDevice>> Renderer::getSuitableDevices(std::vector<const char*> device_extentions)
	{
		return RenderDevice::getSuitableDevices(vulkanInstance, surface, device_extentions);
	}

	void Renderer::useRenderDevice(std::unique_ptr<RenderDevice>&& device)
	{
		renderDevice = std::move(device);
	}

}