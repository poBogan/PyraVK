#pragma once
#include <PyraVK/VulkanInstance.h>
#include <PyraVK/Surface.h>
#include <PyraVK/Device.h>
#include <PyraVK/Swapchain.h>
#include <PyraVk/Shader.h>
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

	struct ResourceID
	{
		uint32_t generation;
		uint32_t index;
	};

	template<class T> struct Slot
	{
		std::optional<T> object; 
		uint32_t generation = 1;

		Slot<T>() noexcept = default;
		~Slot<T>() noexcept = default;

		Slot(const Slot<T>&) = delete;
		const Slot<T>& operator=(const Slot<T>&) = delete; 

		Slot(Slot<T>&&) noexcept = default; 
		Slot<T>& operator=(Slot<T>&&) noexcept = default;
	};

	struct RenderTarget
	{
		ResourceID graphics_pipeline; 
		ResourceID vertex_buffer; 
		
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

		std::optional<vk::raii::CommandPool> frameCommandPool;
		uint32_t currentFrameIdx = 0;
		uint32_t concurrentFrameCount = 0;
		std::vector<Frame> concurrentFrames;

		std::unordered_map<std::string, std::shared_ptr<Shader>> shaderEntries;

		std::optional<vk::raii::PipelineCache> graphicsPipelineCache;
		std::queue<uint32_t> emptyGraphicsPipelinePoolSlots;
		std::vector<Slot<GraphicsPipeline>> graphicsPipelinePool;

		std::optional<vma::raii::Allocator> memoryAllocator;
		std::optional<vk::raii::CommandPool> copy_CommandPool;
		std::optional<vk::raii::CommandBuffer> copy_CommandBuffer;

		std::queue<uint32_t> emptyVertexBufferPoolSlots;
		std::vector<Slot<VertexBuffer>> vertexBufferPool;

		std::queue<uint32_t> emptyIndexBufferPoolSlots;
		std::vector<Slot<IndexBuffer>> indexBufferPool;
	public:
		Renderer() = default;
		Renderer(const Renderer&) = delete; 
		Renderer(Renderer&&) = default;
		~Renderer();

		Renderer& operator=(const Renderer&) = delete; 
		Renderer& operator=(Renderer&&) = default;

		static void createRenderer(); 
		static bool rendererExists();
		static Renderer& getRenderer();

		void constructVulkanInstance(VulkanInstance::Config config); 
		bool vulkanInstanceExists() const;
		const VulkanInstance& getVulkanInstance() const;
		void destroyVulkanInstance();

		void constructWindowSurface(Surface::Config config);
		bool windowSurfaceExists() const;
		const Surface& getWindowSurface() const;
		void destroyWindowSurface();

		std::vector<Device::Config> getSuitableDevices();
		void selectDevice(Device::Config config);
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

		void constructShaders(Shader::Config config);
		bool shaderEntryExists(std::string entryName) const;
		void destroyAllShaders();

		ResourceID constructGraphicsPipeline(GraphicsPipeline::Config config);
		bool graphicsPipelineExists(ResourceID id) const;
		void destroyGraphicsPipeline(ResourceID id); 
		void destroyAllGraphicsPipelines();

		ResourceID constructVertexBuffer(const std::vector<Vertex>& verticies);
		bool vertexBufferExists(ResourceID id) const;
		void destroyVertexBuffer(ResourceID id);

		ResourceID constructIndexBuffer(const std::vector<uint32_t>& indicies);
		void destroyIndexBuffer(ResourceID id); 

		void Draw(RenderTarget target,glm::vec4 clearColor);
	};
};