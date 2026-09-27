#pragma once
#include <PyraVK/Definitions.h>
#include <PyraVK/VulkanInstance.h>
#include <PyraVK/Surface.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>
#include <PyraVk/Shader.h>
#include <PyraVK/GraphicsPipeline.h>

namespace pyra
{
	class Renderer : public Base
	{
	private:
		static std::optional<Renderer> singletonRenderer;
	private : 
		std::optional<VulkanInstance> vulkanInstance;
		std::optional<Surface> windowSurface; 
		std::optional<Device> renderDevice; 
		std::optional<Swapchain> swapchain;
		std::unordered_map<std::string, std::shared_ptr<Shader>> shaderEntries;
		std::unordered_map<GraphicsPipeline::Config,GraphicsPipeline,GraphicsPipeline::Hasher> graphicsPipelines;

		uint32_t currentFrame = 0;
		uint32_t framesInFlight = 0; 
		vk::raii::CommandBuffers commandBuffers = nullptr; 
		std::vector<vk::raii::Fence> commandBufferFences; 
		std::vector<vk::raii::Semaphore> commandBufferSemaphores;
	public:
		static void createRenderer();
		static Renderer& getRenderer();

		void constructVulkanInstance(VulkanInstance::Config config);
		void constructWindowSurface(Surface::Config config);

		std::vector<Device::Config> getSuitableRenderDevices();
		void selectRenderDevice(Device::Config config);
		Swapchain::Config constructSwapchain(Swapchain::Config requestedConfig);

		void constructShaders(Shader::Config config);
		void constructGraphicsPipeline(GraphicsPipeline::Config config);

		void constructFramesInFlight(uint32_t framesInFlightCount);

		void Draw();

		void destroyVulkanInstance();
		void destroyWindowSurface();
		void destroyRenderDevice();
		void destroySwapchain(); 

		const std::optional<VulkanInstance>& getVulkanInstance() const;
		const std::optional<Surface>& getWindowSurface() const;
		const std::optional<Device>& getRenderDevice() const;
		const std::optional<Swapchain>& getSwapchain() const;
		const std::unordered_map<std::string, std::shared_ptr<Shader>>& getShaderEntries() const;
		const std::unordered_map<GraphicsPipeline::Config, GraphicsPipeline, GraphicsPipeline::Hasher>& getGraphicsPipelines() const;
		const GraphicsPipeline::Config& getCurrentGraphicsPipelineConfig() const;
	};

};