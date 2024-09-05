#pragma once

#include "fwd.hpp"

#include "common/InstanceOwned.hpp"
#include "common/StructDefs.hpp"
#include "objects/CommandBuffer.hpp"

#include <vector>

namespace Engine::Rendering::Vulkan
{
	struct SyncObjects
	{
		vk::raii::Semaphore image_available = nullptr;
		vk::raii::Semaphore present_ready	= nullptr; // render_finished_semaphores
		vk::raii::Fence		in_flight		= nullptr;

		SyncObjects() = default;
		SyncObjects(vk::raii::Device& with_device);
	};

	struct FramebufferData
	{
		const vk::ImageView* depth		   = nullptr;
		const vk::ImageView* color		   = nullptr;
		const vk::ImageView* color_resolve = nullptr;
	};

	struct RenderTarget final
	{
		SyncObjects	   sync_objects;
		CommandBuffer* command_buffer = nullptr;

		vk::raii::Framebuffer* framebuffer = nullptr;

		RenderTarget() = default;
		RenderTarget(
			vk::raii::RenderPass& with_render_pass,
			vk::Extent2D		  with_extent,
			FramebufferData		  with_fb_data,
			CommandPool*		  with_command_pool,
			vk::raii::Device&	  with_device
		);
		~RenderTarget();

		RenderTarget(RenderTarget&&)			= default;
		RenderTarget& operator=(RenderTarget&&) = default;
	};
	static_assert(!std::is_copy_constructible_v<RenderTarget> && !std::is_copy_assignable_v<RenderTarget>);
	static_assert(std::is_move_constructible_v<RenderTarget> && std::is_move_assignable_v<RenderTarget>);

	class Swapchain final : public InstanceOwned, public HandleWrapper<vk::raii::SwapchainKHR>
	{
	public:
		Swapchain(
			InstanceOwned::value_t with_instance,
			Surface*			   with_surface,
			vk::Extent2D		   with_extent,
			uint32_t			   with_count
		);
		~Swapchain();

		void BeginRenderPass(CommandBuffer* with_buffer, size_t image_index);
		void RenderFrame(
			CommandBuffer*														command_buffer,
			uint32_t															image_index,
			const std::function<void(Vulkan::CommandBuffer*, uint32_t, void*)>& callback,
			void*																user_data
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

		[[nodiscard]] const vk::Extent2D& GetExtent() const
		{
			return extent;
		}

		[[nodiscard]] RenderPass* GetRenderPass()
		{
			return render_pass;
		}

	private:
		std::vector<vk::Image>			 image_handles;
		std::vector<vk::raii::ImageView> image_view_handles;

		ViewedImage* depth_image = nullptr;
		ViewedImage* color_image = nullptr;

		per_frame_array<RenderTarget*> render_targets;

		vk::SurfaceFormatKHR surface_format;
		const vk::Extent2D	 extent;

		RenderPass* render_pass = nullptr;
	};
} // namespace Engine::Rendering::Vulkan
