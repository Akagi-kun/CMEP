#pragma once

#include "VulkanStructDefs.hpp"
#include "Wrappers/CommandBuffer.hpp"
#include "framework.hpp"

#include <vector>

namespace Engine::Rendering::Vulkan
{
	struct RenderTargetData final
	{
		SyncObjects sync_objects;
		CommandBuffer* command_buffer;
	};

	class Swapchain final : public InstanceOwned, public HandleWrapper<vk::SwapchainKHR>
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
		void RenderFrame(
			CommandBuffer* command_buffer,
			uint32_t image_index,
			const std::function<void(Vulkan::CommandBuffer*, uint32_t, void*)>& callback,
			void* user_data
		);

		[[nodiscard]] RenderTargetData& GetRenderTarget(size_t index)
		{
			return this->render_targets[index];
		}

		[[nodiscard]] vk::Format GetImageFormat() const
		{
			return this->image_format;
		}

		[[nodiscard]] std::vector<vk::ImageView>& GetImageViewHandles()
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
		std::vector<vk::Image> image_handles;
		std::vector<vk::ImageView> image_view_handles;

		per_frame_array<RenderTargetData> render_targets;

		vk::Format image_format{};
		const VkExtent2D extent;

		RenderPass* render_pass = nullptr;

		// TODO: move framebuffers into RenderTargetData
		std::vector<vk::Framebuffer> framebuffers;

		// Multisampling
		Image* multisampled_color_image = nullptr;
		// Depth buffers
		Image* depth_buffer				= nullptr;

		void CreateRenderTarget(RenderTargetData& target);
		void CreateSyncObjects(SyncObjects& sync_objects);

		void CleanupRenderTarget(RenderTargetData& target);
	};
} // namespace Engine::Rendering::Vulkan
