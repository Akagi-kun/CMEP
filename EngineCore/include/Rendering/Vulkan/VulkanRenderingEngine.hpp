#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/Buffer.hpp"
#include "Rendering/Vulkan/Wrappers/Swapchain.hpp"
#include "Rendering/Vulkan/Wrappers/framework.hpp"

#include "InternalEngineObject.hpp"
#include "Wrappers/Window.hpp"
#include "vulkan/vulkan_core.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	enum class VulkanTopologySelection : uint8_t
	{
		VULKAN_RENDERING_ENGINE_TOPOLOGY_TRIANGLE_LIST,
		VULKAN_RENDERING_ENGINE_TOPOLOGY_LINE_LIST
	};

	struct VSyncObjects
	{
		VkSemaphore image_available;
		VkSemaphore present_ready; // render_finished_semaphores
		VkFence in_flight;
	};

	class VulkanRenderingEngine : public InternalEngineObject
	{
	private:
		// Maximum number of frames in rotation/flight
		static constexpr uint16_t max_frames_in_flight = 3;

		// Swap chain data
		Swapchain* swapchain   = nullptr;
		Window* window		   = nullptr;
		uint32_t current_frame = 0;

		// Command buffers
		std::array<CommandBuffer*, max_frames_in_flight> command_buffers;

		// Synchronisation
		std::array<VSyncObjects, max_frames_in_flight> sync_objects;

		// Device manager
		std::shared_ptr<DeviceManager> device_manager;

		// External callback for rendering
		std::function<void(Vulkan::CommandBuffer*, uint32_t, Engine*)> external_callback;

		bool DoesVulkanFormatHaveStencilComponent(VkFormat format);

		// Swap chain functions
		[[nodiscard]] VkExtent2D ChooseVulkanSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		void RecreateVulkanSwapChain();
		void CleanupVulkanSwapChain();

		// Command buffer functions
		void RecordFrameRenderCommands(CommandBuffer* command_buffer, uint32_t image_index);

		// Init functions
		void CreateVulkanSwapChain();
		// void CreateVulkanRenderPass();
		void CreateVulkanSyncObjects();

		void CleanupVulkanSyncObjects();

	public:
		VulkanRenderingEngine(Engine* with_engine, ScreenSize with_window_size, std::string title);
		~VulkanRenderingEngine();

		// Signaling function for framebuffer resize
		void SignalFramebufferResizeGLFW(ScreenSize with_size);

		// Cleanup functions
		void Cleanup();

		// Engine functions
		void DrawFrame();
		void SetRenderCallback(std::function<void(Vulkan::CommandBuffer*, uint32_t, Engine*)> callback);

		// Buffer functions
		Buffer* CreateVulkanVertexBufferFromData(const std::vector<RenderingVertex>& vertices);
		Buffer* CreateVulkanStagingBufferWithData(const void* data, VkDeviceSize data_size);

		template <typename value_type> using per_frame_array = std::array<value_type, max_frames_in_flight>;

		// Getters
		[[nodiscard]] DeviceManager* GetDeviceManager()
		{
			return this->device_manager.get();
		}

		[[nodiscard]] Window* GetWindow() const
		{
			return this->window;
		}

		[[nodiscard]] static constexpr uint32_t GetMaxFramesInFlight()
		{
			return VulkanRenderingEngine::max_frames_in_flight;
		}

		[[nodiscard]] Swapchain* GetSwapchain()
		{
			return this->swapchain;
		}

		// VkFormat functions
		[[nodiscard]] static VkFormat FindVulkanSupportedFormat(
			VkPhysicalDevice with_device,
			const std::vector<VkFormat>& candidates,
			VkImageTiling tiling,
			VkFormatFeatureFlags features
		);
		[[nodiscard]] static VkFormat FindVulkanSupportedDepthFormat(VkPhysicalDevice with_device);

		void SyncDeviceWaitIdle();

		// Utility functions
		uint32_t FindVulkanMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
	};
} // namespace Engine::Rendering::Vulkan
