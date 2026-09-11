#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/VulkanInstance.h>
#include <PyraVK/Surface.h>
#include <PyraVK/RenderDevice.h>

namespace pyra
{

	class Renderer
	{
	private:
		static std::unique_ptr<Renderer> singletonRenderer;

		std::unique_ptr<VulkanInstance> vulkanInstance = nullptr;
		std::unique_ptr<Surface>		surface = nullptr;
		std::unique_ptr<RenderDevice>	renderDevice = nullptr;
	private:
		Renderer(std::unique_ptr<VulkanInstance>&& instance, std::unique_ptr<Surface>&& surface);
	public:
		Renderer() = delete;
		Renderer(Renderer&) = delete;
		Renderer(Renderer&&) = delete;

		static const std::unique_ptr<Renderer>& createRenderer(std::unique_ptr<VulkanInstance>&& instance, std::unique_ptr<Surface>&& surface);
		static const std::unique_ptr<Renderer>& getRenderer();

		std::vector<std::unique_ptr<RenderDevice>> getSuitableDevices(std::vector<const char*>device_extentions = {vk::KHRSwapchainExtensionName});
		void useRenderDevice(std::unique_ptr<RenderDevice>&& device);
	};

};