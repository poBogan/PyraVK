#pragma once
#include <PyraVK/VulkanInstance.h>
#include <PyraVK/Surface.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>
#include <PyraVk/Shader.h>
#include <PyraVK/ResourcePool.hpp>
#include <PyraVK/GraphicsPipeline.h>
#include <PyraVK/VertexBuffer.h>
#include <PyraVK/IndexBuffer.h>

namespace pyra
{
	struct Frame
	{
		vk::raii::CommandBuffer commandBuffer;
		vk::raii::Fence commandBufferReadyFence;
		vk::raii::Semaphore commandBufferImagePaintableSemaphore;
	};

	struct RenderTarget
	{
		ResourceID graphics_pipeline;
		ResourceID vertex_buffer;
		vk::Viewport viewport; 
		vk::Rect2D scissor;
		
	};

	class Renderer
	{
	private:
		static std::optional<Renderer> singletonRenderer;
	private:
		std::optional<VulkanInstance> vulkanInstance;
		std::optional<Surface> windowSurface;
		std::optional<Device> renderDevice;
		std::optional<Swapchain> swapchain;

		// Concurrent frame objects
		std::optional<vk::raii::CommandPool> frameCommandPool;
		uint32_t currentFrameIdx = 0;
		std::vector<Frame> concurrentFrames;

		// Saved shader modules
		std::unordered_map<std::string, std::shared_ptr<Shader>> shaderEntries;

		// Graphics Pipeline objects
		std::optional<vk::raii::PipelineCache> graphicsPipelineCache;
		ResourcePool<GraphicsPipeline> graphicsPipelinePool;

		// Device memory allocator
		std::optional<vma::raii::Allocator> memoryAllocator;

		// Copy objects for buffer creation
		std::optional<vk::raii::CommandPool> copy_CommandPool;
		std::optional<vk::raii::CommandBuffer> copy_CommandBuffer;

		// Buffer objects
		ResourcePool<VertexBuffer> vertexBufferPool; 
		ResourcePool<IndexBuffer> indexBufferPool; 

	public:
		Renderer() = default;
		Renderer(const Renderer&) = delete; 
		Renderer(Renderer&&) = default;
		~Renderer();

		static void createRenderer(); 
		static bool rendererExists();
		static Renderer& getRenderer();

		void constructVulkanInstance(const VulkanInstance::Config& config);
		bool vulkanInstanceExists() const;
		const VulkanInstance& getVulkanInstance() const;
		void destroyVulkanInstance();

		void constructWindowSurface(const VkSurfaceKHR& surface);
		bool windowSurfaceExists() const;
		const Surface& getWindowSurface() const;
		void destroyWindowSurface();

		std::vector<Device::Config> getSuitableDevices() const;
		void selectDevice(const Device::Config& config);
		bool deviceSelected() const;
		const Device& getDevice() const;
		void unselectDevice();

		Swapchain::Config constructSwapchain(Swapchain::Config requestedConfig);
		bool swapchainExists() const;
		const Swapchain& getSwapchain() const;
		void destroySwapchain();

		void constructConcurrentFrames(uint32_t frameCount); 
		uint32_t concurrentFramesCount() const;
		void destroyConcurrentFrames();

		void constructShaders(const Shader::Config& config);
		bool shaderEntryExists(const std::string& entryName) const;
		void destroyAllShaders();

		ResourceID constructGraphicsPipeline(const GraphicsPipeline::Config& config);
		bool graphicsPipelineExists(ResourceID id) const;
		void destroyGraphicsPipeline(ResourceID id);
		void destroyAllGraphicsPipelines();

		ResourceID constructVertexBuffer(const std::vector<Vertex>& vertices);
		bool vertexBufferExists(ResourceID id) const;
		void destroyVertexBuffer(ResourceID id);
		void destroyAllVertexBuffers();

		void Draw(RenderTarget target,glm::vec4 clearColor);
	};
};