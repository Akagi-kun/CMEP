#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include <optional>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "glfw/include/GLFW/glfw3.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
//#include "GL/glcorearb.h"

#pragma region forward decls

namespace Engine
{
	class AssetManager;
	class ImageObject;
	class TextObject;
	class Generic3DObject;
	class Object;
	namespace Rendering
	{
		typedef enum class Texture_InitFiletypeEnum
		{
			FILE_RAW = 1,
			FILE_NETPBM = 2,
			FILE_PNG = 3
		} Texture_InitFiletype;

		class Renderer;
		class Texture;
		class Window;
		class Shader;
		class Font;
		class Mesh;
	}
}

#pragma endregion

namespace Engine
{
	namespace Rendering
	{

#pragma region Mesh.hpp

		class Mesh final
		{
		private:
		public:
			std::vector<glm::vec3> mesh_vertices;
			std::vector<glm::vec2> mesh_uvs;
			std::vector<glm::vec3> mesh_normals;

			void CreateMeshFromObj(std::string path);
		};

#pragma endregion

#pragma region Font.hpp

		struct FontChar
		{
			int x, y, width, height, xoffset, yoffset, xadvance, page, channel;
		};

		class Font final
		{
		private:
			AssetManager* asset_manager;
			std::string fntfile;
			unsigned int char_count = 0;

			// Data from fnt file
			std::unordered_map<std::string, std::string> info;
			std::unordered_map<int, Texture*> pages;
			std::unordered_map<int, FontChar> chars;

			void EvalBmfont(FILE* file) noexcept;
			void EvalBmfontLine(int type, char* data) noexcept;
		public:
			Font() noexcept;
			Font(const Font& other) noexcept = delete;
			Font(const Font&& other) noexcept = delete;
			Font& operator=(const Font& other) noexcept = delete;
			Font& operator=(const Font&& other) noexcept = delete;
			~Font() noexcept;

			int Init(std::string path) noexcept;

			FontChar* GetChar(char ch) noexcept;
			Texture* GetPageTexture(int page) noexcept;
		};

#pragma endregion

#pragma region Shader.hpp

		class Shader final
		{
		private:
			int program = 0;

			static int SetupShader(const char* vert, const char* frag) noexcept;

		public:
			Shader(const char* vert, const char* frag) noexcept;

			int GetProgram() const noexcept;

			bool IsValid() const noexcept;
		};

#pragma endregion

#pragma region Texture.hpp

		class Texture final
		{
		private:
			char* data = nullptr;
			unsigned int x = 0, y = 0;
			int color_fmt = 0;
			int texture = 0;

		public:
			Texture() noexcept;
			Texture(const Texture& other) noexcept;
			Texture(const Texture&& other) noexcept;
			Texture& operator=(const Texture& other) noexcept;
			Texture& operator=(const Texture&& other) noexcept;
			~Texture() noexcept;

			int InitRaw(const char* const raw_data, int color_format, unsigned int x, unsigned int y);
			int InitFile(Texture_InitFiletype filetype, std::string path, unsigned int sizex = 0, unsigned int sizey = 0);

			void GetSize(unsigned int& x, unsigned int& y) const noexcept;
			char* GetData() const noexcept;
			int GetTexture() const noexcept;
			int GetColorFormat() const noexcept;
		};

#pragma endregion

#pragma region IRenderer.hpp

		/// <summary>
		/// Interface describing GL Renderer API for UI renderables.
		/// </summary>
		class IRenderer
		{
		protected:
			/// <summary>
			/// Renderable's position.
			/// </summary>
			glm::vec3 _pos = glm::vec3();
			/// <summary>
			/// Renderable's size.
			/// </summary>
			glm::vec3 _size = glm::vec3();
			/// <summary>
			/// Renderable's rotation.
			/// </summary>
			glm::vec3 _rotation = glm::vec3();

			/// <summary>
			/// Screen size as reported by <seealso cref="Object"/>.
			/// </summary>
			uint_fast16_t _screenx = 0, _screeny = 0;

			/// <summary>
			/// Used by <seealso cref="UpdateMesh"/> to optimize Updates to when necessary.
			/// </summary>
			bool has_updated_mesh = false;

		public:
			IRenderer() {};

			/// <summary>
			/// Updates data for renderer.
			/// </summary>
			/// <param name="pos">Position of renderable.</param>
			/// <param name="size">Size of renderable.</param>
			/// <param name="screenx">X size of screen.</param>
			/// <param name="screeny">Y size of screen.</param>
			virtual void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept = 0;

			/// <summary>
			/// Updates mesh of renderable.
			/// </summary>
			virtual void UpdateMesh() = 0;

			/// <summary>
			/// Render the renderable represented by this <seealso cref="Renderer"/>.
			/// </summary>
			virtual void Render() = 0;
		};

#pragma endregion

#pragma region TextRenderer.hpp

		/// <summary>
		/// Implementation of <seealso cref="IRenderer"/> for text renderables.
		/// </summary>
		/// <inheritdoc cref="IRenderer"/>
		class TextRenderer final : public IRenderer
		{
		private:
			/// <summary>
			/// GL Vertex Array Object
			/// </summary>
			unsigned int vao = 0;
			/// <summary>
			/// GL Vertex Buffer Object
			/// </summary>
			unsigned int vbo = 0;
			/// <summary>
			/// Count of vertices in Vertex Buffer Object
			/// </summary>
			size_t vbo_vert_count = 0;
			/// <summary>
			/// Text to be rendered
			/// </summary>
			std::string text = "";

			/// <summary>
			/// Currently used shader
			/// </summary>
			std::unique_ptr<Rendering::Shader> program;
			/// <summary>
			/// Currently used font
			/// </summary>
			std::unique_ptr<Rendering::Font> font;

		public:
			TextRenderer();
			~TextRenderer();

			void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept override;

			/// <summary>
			/// Update font used by renderer. See <see cref="font"/>.
			/// </summary>
			/// <param name="font">New font.</param>
			int UpdateFont(Rendering::Font* const font) noexcept;

			/// <summary>
			/// Update rendered text.
			/// </summary>
			/// <param name="text">New text.</param>
			int UpdateText(const std::string text) noexcept;

			void UpdateMesh() override;
			void Render() override;
		};

#pragma endregion
		
#pragma region SpriteRenderer.hpp

		/// <summary>
		/// Implementation of <seealso cref="IRenderer"/> for 2D sprite renderables.
		/// </summary>
		/// <inheritdoc cref="IRenderer"/>
		class SpriteRenderer final : public IRenderer
		{
		private:
			/// <summary>
			/// GL Vertex Array Object
			/// </summary>
			unsigned int vao = 0;
			/// <summary>
			/// GL Vertex Buffer Object
			/// </summary>
			unsigned int vbo = 0;

			std::unique_ptr<Rendering::Shader> program;
			std::unique_ptr<const Rendering::Texture> texture;

		public:
			SpriteRenderer();
			~SpriteRenderer();

			void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept override;
			void UpdateTexture(const Rendering::Texture* texture) noexcept;
			void UpdateMesh() noexcept override;

			void Render() override;
		};

#pragma endregion

#pragma region MeshRenderer.hpp

		/// <summary>
		/// Implementation of <seealso cref="IRenderer"/> for custom mesh renderables.
		/// </summary>
		/// <inheritdoc cref="IRenderer"/>
		class MeshRenderer final : public IRenderer
		{
		private:
			/// <summary>
			/// GL Vertex Array Object
			/// </summary>
			unsigned int vao = 0;
			/// <summary>
			/// GL Vertex Buffer Object
			/// </summary>
			unsigned int vbo = 0;

			glm::mat4 MVP;

			/// <summary>
			/// Currently used shader
			/// </summary>
			std::unique_ptr<Shader> program;
			std::unique_ptr<const Rendering::Texture> texture;

			std::unique_ptr<Mesh> mesh;
		public:
			MeshRenderer();
			~MeshRenderer();

			void AssignMesh(Mesh& new_mesh);

			void UpdateTexture(const Rendering::Texture* texture) noexcept;
			void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept override;
			void UpdateMesh() noexcept override;

			void Render() override;
		};

#pragma endregion

#pragma region VulkanRenderingEngine.hpp

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct GLFWwindowData
	{
		GLFWwindow* window;
		unsigned int windowX = 0, windowY = 0;
		std::string windowTitle;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct VulkanBuffer
	{
		VkBuffer buffer;
		VkDeviceMemory bufferMemory;
		void* mappedMemory;
	};

	struct VulkanImage
	{
		VkImage image;
		VkDeviceMemory imageMemory;
		VkFormat imageFormat;
		VkImageView imageView;
	};

	struct VulkanTextureImage
	{
		VulkanImage* image;
		VkSampler textureSampler;
	};

	struct VulkanDescriptorLayoutSettings
	{
		std::vector<uint32_t> binding;
		std::vector<VkDescriptorType> types;
		std::vector<VkShaderStageFlags> stageFlags;
		std::vector<uint32_t> descriptorCount;
	};

	struct VulkanPipelineSettings
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssembly;
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportState;
		VkPipelineRasterizationStateCreateInfo rasterizer;
		VkPipelineMultisampleStateCreateInfo multisampling;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlending;
		VkPipelineDepthStencilStateCreateInfo depthStencil;

		VulkanDescriptorLayoutSettings descriptorLayoutSettings;
	};

	struct VulkanPipeline
	{
		VkPipeline pipeline;
		VkPipelineLayout vkPipelineLayout;
		VkDescriptorPool vkDescriptorPool;
		VkDescriptorSetLayout vkDescriptorSetLayout;
		std::vector<VkDescriptorSet> vkDescriptorSets{};
		std::vector<VulkanBuffer*> uniformBuffers;
	};

	enum VulkanTopologySelection
	{
		VULKAN_RENDERING_ENGINE_TOPOLOGY_TRIANGLE_LIST,
		VULKAN_RENDERING_ENGINE_TOPOLOGY_LINE_LIST
	};

	struct RenderingVertex {
		glm::vec3 pos{};
		glm::vec3 color{};
		glm::vec2 texcoord{};
		glm::vec3 normal{};

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};

			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(RenderingVertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
			
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(RenderingVertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(RenderingVertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(RenderingVertex, texcoord);
			
			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(RenderingVertex, normal);

			return attributeDescriptions;
		}
	};

	class VulkanRenderingEngine
	{
	private:
		const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

		GLFWwindow* window = nullptr;
		unsigned int windowX = 0, windowY = 0;
		std::string windowTitle{};

		static std::vector<char> readShaderFile(std::string path);

		uint32_t currentFrame = 0;
		bool framebufferResized = false;

		// Vulkan instance
		VkInstance vkInstance = VK_NULL_HANDLE;
		
		// Queues
		QueueFamilyIndices graphicsQueueIndices{};
		VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
		VkQueue vkPresentQueue = VK_NULL_HANDLE;

		// Devices
		VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
		VkDevice vkLogicalDevice = VK_NULL_HANDLE;

		// Surfaces
		VkSurfaceKHR vkSurface = VK_NULL_HANDLE;

		// Swap chains
		VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> vkSwapChainImages{};
		VkFormat vkSwapChainImageFormat{};
		VkExtent2D vkSwapChainExtent{};
		std::vector<VkImageView> vkSwapChainImageViews{};

		// Multisampling
		VulkanImage* multisampledColorImage{};

		// Framebuffers
		std::vector<VkFramebuffer> vkSwapChainFramebuffers{};
		
		// Command pools and buffers
		VkCommandPool vkCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> vkCommandBuffers{};

		// Synchronisation
		std::vector<VkSemaphore> imageAvailableSemaphores{};
		std::vector<VkSemaphore> renderFinishedSemaphores{};
		std::vector<VkFence> inFlightFences{};

		// Pipeline
		VulkanPipeline* graphicsPipelineDefault = nullptr;
		VkRenderPass vkRenderPass = VK_NULL_HANDLE;

		// Depth buffers
		VulkanImage* vkDepthBuffer = nullptr;

		// External callback for rendering
		std::function<void(VkCommandBuffer, uint32_t)> external_callback;

		// Required extensions to be supported
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			VK_EXT_ROBUSTNESS_2_EXTENSION_NAME
		};
		// Required validation layers to be supported
		const std::vector<const char*> vkValidationLayers = {
			"VK_LAYER_KHRONOS_validation",
		};

		// Validation layers
		VkDebugUtilsMessengerEXT vkDebugMessenger = VK_NULL_HANDLE;
#ifndef _DEBUG
		const bool enableVkValidationLayers = false;
#else
		const bool enableVkValidationLayers = true;
#endif

		// Leak check counters
		size_t leakBufferCounter = 0;
		size_t leakUniformBufferCounter = 0;
		size_t leakImageCounter = 0;
		size_t leakTextureImageCounter = 0;
		size_t leakPipelineCounter = 0;

		// Physical device functions
		int checkVulkanPhysicalDeviceScore(VkPhysicalDevice device);
		QueueFamilyIndices findVulkanQueueFamilies(VkPhysicalDevice device);
		bool checkVulkanDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails queryVulkanSwapChainSupport(VkPhysicalDevice device);
		VkSampleCountFlagBits getMaxUsableSampleCount();

		// VkFormat functions
		VkFormat findVulkanSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat findVulkanSupportedDepthFormat();
		bool doesVulkanFormatHaveStencilComponent(VkFormat format);

		// Swap chain functions
		VkSurfaceFormatKHR chooseVulkanSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseVulkanSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseVulkanSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void createVulkanSwapChainViews();
		void recreateVulkanSwapChain();
		void cleanupVulkanSwapChain();

		// Command buffer functions
		void recordVulkanCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		// Shaders functions
		VkShaderModule createVulkanShaderModule(const std::vector<char>& code);

		// Image view functions
		VkImageView createVulkanImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

		// Init functions
		bool checkVulkanValidationLayers();
		void initVulkanInstance();
		void initVulkanDevice();
		void createVulkanLogicalDevice();
		void createVulkanSurface();
		void createVulkanSwapChain();
		void createVulkanDefaultGraphicsPipeline();
		void createVulkanRenderPass();
		void createVulkanFramebuffers();
		void createVulkanCommandPools();
		void createVulkanCommandBuffers();
		void createVulkanSyncObjects();
		void createVulkanDepthResources();
		void createMultisampledColorResources();

	public:
		VulkanRenderingEngine() {}

		// Signaling function for framebuffer resize
		void SignalFramebufferResizeGLFW();
		
		// Cleanup functions
		void cleanup();
		void cleanupVulkanBuffer(VulkanBuffer* buffer);
		void cleanupVulkanTextureImage(VulkanTextureImage* buffer);
		void cleanupVulkanPipeline(VulkanPipeline* pipeline);
		void cleanupVulkanImage(VulkanImage* image);
		
		// Init
		void init(unsigned int xsize, unsigned int ysize, std::string title);

		// Engine functions
		void drawFrame();
		void SetRenderCallback(std::function<void(VkCommandBuffer, uint32_t)> callback);
		
		// Buffer functions
		VulkanBuffer* createVulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		void bufferVulkanTransferCopy(VulkanBuffer* src, VulkanBuffer* dest, VkDeviceSize size);
		VulkanBuffer* createVulkanVertexBufferFromData(std::vector<RenderingVertex> vertices);
		VulkanBuffer* createVulkanStagingBufferPreMapped(VkDeviceSize dataSize);
		VulkanBuffer* createVulkanStagingBufferWithData(void* data, VkDeviceSize dataSize);

		// Image functions
		VulkanImage* createVulkanImage(uint32_t width, uint32_t height, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
		VulkanTextureImage* createVulkanTextureImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
		void copyVulcanBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		void appendVulkanImageViewToVulkanTextureImage(VulkanTextureImage* teximage);
		void appendVulkanSamplerToVulkanTextureImage(VulkanTextureImage* teximage);
		void transitionVulkanImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

		// Command buffer functions
		VkCommandBuffer beginVulkanSingleTimeCommandsCommandBuffer();
		void endVulkanSingleTimeCommandsCommandBuffer(VkCommandBuffer commandBuffer);

		// Pipeline functions
		VulkanPipelineSettings getVulkanDefaultPipelineSettings();
		VulkanPipeline* createVulkanPipelineFromPrealloc(VulkanPipeline* pipeline, VulkanPipelineSettings& settings, std::string vert_path, std::string frag_path);
		VulkanPipeline* createVulkanPipeline(VulkanPipelineSettings& settings, std::string vert_path, std::string frag_path);

		// Pipeline descriptor functions
		void createVulkanDescriptorSetLayout(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings);
		void createVulkanUniformBuffers(VulkanPipeline* pipeline);
		void createVulkanDescriptorPool(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings);
		void createVulkanDescriptorSets(VulkanPipeline* pipeline, VulkanDescriptorLayoutSettings settings);

		// Getters
		VkDevice GetLogicalDevice();
		GLFWwindowData const GetWindow();
		const uint32_t GetMaxFramesInFlight();

		// Utility functions
		uint32_t findVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};

#pragma endregion

	}

	namespace Scripting
	{
		enum class ExecuteType {
			EventHandler,
			ObjectScript
		};

#pragma region LuaScript.hpp
		class LuaScript
		{
		protected:
			void* state;

		public:
			std::string path;

			LuaScript(std::string path);
			~LuaScript();

			void* GetState();
		};
#pragma endregion

#pragma region LuaScriptExecutor.hpp
		class LuaScriptExecutor
		{
		protected:
			static void registerCallbacks(void* state);
		public:
			LuaScriptExecutor() {};
			~LuaScriptExecutor() {};

			static void CallIntoScript(ExecuteType etype, LuaScript* script, std::string function, void* data);

			static int LoadAndCompileScript(LuaScript* script);
		};
#pragma endregion
	}

#pragma region EventHandling.hpp

	namespace EventHandling
	{
		enum class EventType
		{
			ON_INIT,
			ON_UPDATE,
			ON_KEYDOWN,
			ON_MOUSEMOVED
		};

		class Event final
		{
		private:
		public:
			const EventType event_type;

			double deltaTime = 0.0;
			union
			{
				unsigned char keycode = 0; // ON_KEYDOWN event
				struct {
					uint32_t x;
					uint32_t y;
				} mouse; // ON_MOUSEMOVED event
			};

			Event(const EventType eventtype) : event_type(eventtype) {};
			~Event() {};
		};
	}

#pragma endregion

#pragma region Engine.hpp

	class Engine final
	{
	private:
		// Keeps internal loop running
		bool run_threads = true;

		std::string config_path = "";

		// Window
		//GLFWwindow* window = nullptr;
		unsigned int windowX = 0, windowY = 0;
		std::string windowTitle;
		unsigned int framerateTarget = 30;

		double lastDeltaTime = 0.0;

		// Engine parts
		Rendering::VulkanRenderingEngine* rendering_engine = nullptr;
		AssetManager* asset_manager = nullptr;
		Scripting::LuaScriptExecutor* script_executor = nullptr;

		// Event handler storage
		std::vector<std::pair<EventHandling::EventType, std::function<int(EventHandling::Event&)>>> event_handlers;
		std::vector<std::tuple<EventHandling::EventType, Scripting::LuaScript*, std::string>> lua_event_handlers;
		
		
		static void spinSleep(double seconds);

		static void RenderCallback(VkCommandBuffer commandBuffer, uint32_t currentFrame);

		static void OnWindowFocusCallback(GLFWwindow* window, int focused);
		static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
		static void CursorEnterLeaveCallback(GLFWwindow* window, int entered);
		static void OnKeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

		void handleInput(const double deltaTime) noexcept;

		void engineLoop();

		void HandleConfig();

	public:
		Engine(const char* windowTitle, const unsigned windowX, const unsigned windowY) noexcept;
		~Engine() noexcept;

		void SetFramerateTarget(unsigned framerate) noexcept;

		void Init();
		void Run();

		void ConfigFile(std::string path);
		void RegisterEventHandler(EventHandling::EventType event_type, std::function<int(EventHandling::Event&)> function);
		void RegisterLuaEventHandler(EventHandling::EventType event_type, Scripting::LuaScript* script, std::string function);
		
		int FireEvent(EventHandling::Event& event);

		double GetLastDeltaTime();

		AssetManager* GetAssetManager() noexcept;
		Rendering::VulkanRenderingEngine* GetRenderingEngine() noexcept;
	};

	Engine* initializeEngine(const char* windowTitle, const unsigned windowX, const unsigned windowY);

	int deinitializeEngine();

	Engine* global_engine;
#pragma endregion

#pragma region Object.hpp

	class Object
	{
	protected:
		/// <summary>
		/// Position of object in worldspace.
		/// </summary>
		glm::vec3 _pos = glm::vec3();

		/// <summary>
		/// Size of object.
		/// </summary>
		glm::vec3 _size = glm::vec3();

		/// <summary>
		/// Rotation of object.
		/// </summary>
		glm::vec3 _rotation = glm::vec3();

		unsigned int screenx = 0, screeny = 0;

		std::function<void(Object*)> _onClick = nullptr;

	public:
		Rendering::IRenderer* renderer = nullptr;

		Object() noexcept {}

		void ScreenSizeInform(unsigned int screenx, unsigned int screeny) noexcept
		{
			this->screenx = screenx;
			this->screeny = screeny;
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny); }
		}

		virtual void UpdatePosition(const glm::vec3 pos) noexcept
		{
			this->_pos = pos;
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny); }
		}

		virtual void UpdateSize(const glm::vec3 size) noexcept
		{
			this->_size = size;
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny); }
		}

		virtual void Rotate(const glm::vec3 rotation) noexcept
		{
			this->_rotation = rotation;
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny); }
		}

		virtual int Render()
		{
			if (this->renderer != nullptr) { this->renderer->Render(); }
			return 0;
		}

		void RegisterOnClick(std::function<void(Object*)> f) noexcept { this->_onClick = f; };
		void onClick()
		{
			try { this->_onClick(this); }
			// std::bad_function_call is thrown when this->_onClick has no function assigned, ignore
			catch (std::bad_function_call e) { /* exception ignored */ }
		}

		glm::vec3 pos() const noexcept { return this->_pos; }
		glm::vec3 size() const noexcept { return this->_size; }
		glm::vec3 rotation() const noexcept { return this->_rotation; }

	};

#pragma endregion

#pragma region AssetManager.hpp

	class AssetManager final
	{
	private:
		std::unordered_map<std::string, Rendering::Texture*> textures;
		std::unordered_map<std::string, Rendering::Shader*> shaders;
		std::unordered_map<std::string, Rendering::Font*> fonts;
	public:
		AssetManager() {};

		void AddShader(std::string name, std::string vert_source, std::string frag_source);
		void AddTexture(std::string name, std::string path, Rendering::Texture_InitFiletype filetype);
		void AddFont(std::string name, std::string path);

		Rendering::Shader* GetShader(std::string path);
		Rendering::Texture* GetTexture(std::string path);
		Rendering::Font* GetFont(std::string path);
	};

#pragma endregion

#pragma region GlobalSceneManager.hpp

	class GlobalSceneManager final
	{
	private:
		std::unordered_map<std::string, Object*> objects;

		glm::vec3 cameraTransform; // XYZ position
		glm::vec2 cameraHVRotation; // Horizontal and Vertical rotation
	public:
		GlobalSceneManager();
		~GlobalSceneManager();

		const std::unordered_map<std::string, Object*>* const GetAllObjects() noexcept;

		void AddObject(std::string name, Object* ptr);
		Object* FindObject(std::string name);
		size_t RemoveObject(std::string name) noexcept;

		glm::vec3 GetCameraTransform();
		glm::vec2 GetCameraHVRotation();
		glm::mat4 GetCameraViewMatrix();

		void SetCameraTransform(glm::vec3 transform);
		void SetCameraHVRotation(glm::vec2 hvrotation);
	};

	GlobalSceneManager* global_scene_manager;

#pragma endregion

	namespace ObjectFactory
	{
#pragma region ObjectFactory.hpp

		Object* CreateImageObject(double x, double y, double sizex, double sizey, ::Engine::Rendering::Texture* image);
		Object* CreateTextObject(double x, double y, int size, std::string text, ::Engine::Rendering::Font* font);
		Object* CreateGeneric3DObject(double x, double y, double z, double sizex, double sizey, double sizez, double rotx, double roty, double rotz, ::Engine::Rendering::Mesh mesh);

#pragma endregion
	}
}
