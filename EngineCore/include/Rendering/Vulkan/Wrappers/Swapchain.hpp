#pragma once

#include "Rendering/Vulkan/VulkanRenderingEngine.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"
#include "Rendering/Vulkan/Wrappers/CommandBuffer.hpp"

#include "framework.hpp"
#include "vulkan/vulkan_core.h"

#include <vector>

namespace Engine::Rendering::Vulkan
{
	struct RenderTargetData final
	{
		SyncObjects sync_objects;
		CommandBuffer* command_buffer;
		// VkFramebuffer framebuffer;
	};

	class Swapchain final : public InstanceOwned, public HandleWrapper<VkSwapchainKHR>
	{
	public:
		Swapchain(
			InstanceOwned::value_t with_instance,
			Surface* with_surface,
			VkExtent2D with_extent,
			uint32_t with_count
		);
		~Swapchain();

		void BeginRenderPass(CommandBuffer* with_buffer, size_t image_index);

		[[nodiscard]] RenderTargetData& GetRenderTarget(size_t index)
		{
			return this->render_targets[index];
		}

		[[nodiscard]] VkFormat GetImageFormat() const
		{
			return this->image_format;
		}

		[[nodiscard]] std::vector<VkImageView>& GetImageViewHandles()
		{
			return this->image_view_handles;
		}

		[[nodiscard]] const VkExtent2D& GetExtent() const
		{
			return this->extent;
		}

		[[nodiscard]] RenderPass* GetRenderPass()
		{
			return this->render_pass;
		}

	private:
		std::vector<VkImage> image_handles;
		std::vector<VkImageView> image_view_handles;

		VulkanRenderingEngine::per_frame_array<RenderTargetData> render_targets;

		VkFormat image_format{};
		const VkExtent2D extent;

		RenderPass* render_pass = nullptr;

		std::vector<VkFramebuffer> framebuffers;

		// Multisampling
		Image* multisampled_color_image = nullptr;
		// Depth buffers
		Image* depth_buffer				= nullptr;

		void CreateRenderTarget(RenderTargetData& target);
		void CreateSyncObjects(SyncObjects& sync_objects);

		void CleanupRenderTarget(RenderTargetData& target);
	};
} // namespace Engine::Rendering::Vulkan
