#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/Buffer.hpp"
#include "Rendering/Vulkan/Wrappers/framework.hpp"

#include "InternalEngineObject.hpp"
#include "Wrappers/Instance.hpp"
#include "vulkan/vulkan_core.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	class VulkanRenderingEngine : public InternalEngineObject
	{
	public:
		// Maximum number of frames in rotation/flight
		static constexpr uint16_t max_frames_in_flight		 = 3;
		template <typename value_type> using per_frame_array = std::array<value_type, max_frames_in_flight>;

		VulkanRenderingEngine(Engine* with_engine, ScreenSize with_window_size, const std::string& title);
		~VulkanRenderingEngine();

		// Engine functions
		void DrawFrame();
		void SetRenderCallback(std::function<void(Vulkan::CommandBuffer*, uint32_t, Engine*)> callback);

		// Buffer functions
		Buffer* CreateVulkanVertexBufferFromData(const std::vector<RenderingVertex>& vertices);
		Buffer* CreateVulkanStagingBufferWithData(const void* data, VkDeviceSize data_size);

		// Getters
		/* [[nodiscard]] Window* GetWindow() const
		{
			return instance->GetWindow();
		} */

		[[nodiscard]] Instance* GetInstance()
		{
			return instance;
		}

	private:
		// Swap chain data
		Instance* instance	   = nullptr;
		uint32_t current_frame = 0;

		// External callback for rendering
		std::function<void(Vulkan::CommandBuffer*, uint32_t, Engine*)> external_callback;

		bool DoesVulkanFormatHaveStencilComponent(VkFormat format);

		// Command buffer functions
		void RecordFrameRenderCommands(CommandBuffer* command_buffer, uint32_t image_index);
	};
} // namespace Engine::Rendering::Vulkan
