#pragma once

#include "VulkanStructDefs.hpp"
#include "Wrappers/CommandBuffer.hpp"
#include "framework.hpp"

#include <vector>

namespace Engine::Rendering::Vulkan
{
	struct SyncObjects
	{
		vk::raii::Semaphore image_available = nullptr;
		vk::raii::Semaphore present_ready	= nullptr; // render_finished_semaphores
		vk::raii::Fence in_flight			= nullptr;
	};

	struct RenderTarget final
	{
		SyncObjects sync_objects; //	  = nullptr;
		CommandBuffer* command_buffer = nullptr;

		RenderTarget() = default;
		RenderTarget(CommandPool* with_command_pool, vk::raii::Device& with_device);
		~RenderTarget();

		RenderTarget(RenderTarget&)	 = delete;
		RenderTarget(RenderTarget&&) = default;

		RenderTarget& operator=(RenderTarget&)	= delete;
		RenderTarget& operator=(RenderTarget&&) = default;
	};
	static_assert(std::is_move_constructible<RenderTarget>() && std::is_move_assignable<RenderTarget>());

	class Swapchain final : public InstanceOwned, public HandleWrapper<vk::raii::SwapchainKHR>
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

		[[nodiscard]] RenderTarget& GetRenderTarget(size_t index)
		{
			return *render_targets[index];
		}

		[[nodiscard]] vk::Format GetImageFormat() const
		{
			return surface_format.format;
		}

		[[nodiscard]] std::vector<vk::raii::ImageView>& GetImageViewHandles()
		{
			return image_view_handles;
		}

		[[nodiscard]] const VkExtent2D& GetExtent() const
		{
			return extent;
		}

		[[nodiscard]] RenderPass* GetRenderPass()
		{
			return render_pass;
		}

	private:
		std::vector<vk::Image> image_handles;
		std::vector<vk::raii::ImageView> image_view_handles;

		per_frame_array<RenderTarget*> render_targets;

		vk::SurfaceFormatKHR surface_format;
		const VkExtent2D extent;

		RenderPass* render_pass = nullptr;

		// TODO: move framebuffers into RenderTargetData
		std::vector<vk::raii::Framebuffer*> framebuffers;

		Image* multisampled_color_image = nullptr;
		Image* depth_buffer				= nullptr;
	};
} // namespace Engine::Rendering::Vulkan
