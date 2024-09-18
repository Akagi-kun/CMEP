#include "rendering/Swapchain.hpp"

#include "backend/Instance.hpp"
#include "common/StructDefs.hpp"
#include "common/Utility.hpp"
#include "objects/CommandBuffer.hpp"
#include "objects/CommandPool.hpp"
#include "objects/Image.hpp"
#include "rendering/PipelineSettings.hpp"
#include "rendering/RenderPass.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	Swapchain::Swapchain(
		InstanceOwned::value_t with_instance,
		Surface*			   with_surface,
		vk::Extent2D		   with_extent,
		uint32_t			   with_count
	)
		: InstanceOwned(with_instance), extent(with_extent)
	{
		const PhysicalDevice* physical_device = instance->getPhysicalDevice();
		LogicalDevice*		  logical_device  = instance->getLogicalDevice();

		// Query details for support of swapchains
		SwapChainSupportDetails swap_chain_support =
			with_surface->querySwapChainSupport(*physical_device);
		surface_format =
			Vulkan::Utility::chooseSwapSurfaceFormat(swap_chain_support.formats);

		vk::PresentModeKHR present_mode =
			Vulkan::Utility::chooseSwapPresentMode(swap_chain_support.present_modes);

		QueueFamilyIndices queue_indices = logical_device->getQueueFamilies();

		uint32_t queue_family_indices[] = {
			queue_indices.graphics_family,
			queue_indices.present_family
		};

		bool queue_families_same = queue_indices.graphics_family !=
								   queue_indices.present_family;

		vk::SwapchainCreateInfoKHR create_info{
			.surface			   = with_surface->native_handle,
			.minImageCount		   = with_count,
			.imageFormat		   = surface_format.format,
			.imageColorSpace	   = surface_format.colorSpace,
			.imageExtent		   = with_extent,
			.imageArrayLayers	   = 1,
			.imageUsage			   = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode	   = queue_families_same ? vk::SharingMode::eConcurrent
														 : vk::SharingMode::eExclusive,
			.queueFamilyIndexCount = queue_families_same ? 2 : uint32_t{},
			.pQueueFamilyIndices   = queue_families_same ? queue_family_indices : nullptr,
			.preTransform		   = swap_chain_support.capabilities.currentTransform,
			.compositeAlpha		   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode		   = present_mode,
			.clipped			   = vk::True
		};

		native_handle = logical_device->createSwapchainKHR(create_info);

		image_handles = native_handle.getImages();

		// Create image views
		// these will serve as the color resolve attachment
		for (auto image_handle : image_handles)
		{
			vk::ImageViewCreateInfo view_create_info{
				.image	  = image_handle,
				.viewType = vk::ImageViewType::e2D,
				.format	  = surface_format.format,
				.subresourceRange =
					{.aspectMask	 = vk::ImageAspectFlagBits::eColor,
					 .baseMipLevel	 = 0,
					 .levelCount	 = 1,
					 .baseArrayLayer = 0,
					 .layerCount	 = 1}
			};

			image_view_handles.push_back(logical_device->createImageView(view_create_info)
			);
		}

		vk::Format depth_format = physical_device->findSupportedDepthFormat();

		// Create depth buffer
		depth_image = new ViewedImage(
			instance,
			{extent.width, extent.height},
			physical_device->getMSAASamples(),
			depth_format,
			vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::ImageAspectFlagBits::eDepth
		);

		// Pre-resolve color buffer
		color_image = new ViewedImage(
			instance,
			{extent.width, extent.height},
			physical_device->getMSAASamples(),
			surface_format.format,
			vk::ImageUsageFlagBits::eTransientAttachment |
				vk::ImageUsageFlagBits::eColorAttachment,
			vk::ImageAspectFlagBits::eColor
		);

		render_pass =
			new RenderPass(physical_device, logical_device, surface_format.format);

		size_t view_idx = 0;
		for (auto& target : render_targets)
		{
			FramebufferData fb_data = {
				&*depth_image->getNativeViewHandle(),
				&*color_image->getNativeViewHandle(),
				&*image_view_handles[view_idx]
			};
			++view_idx;

			target = new RenderTarget(
				render_pass->getHandle(),
				extent,
				fb_data,
				instance->getCommandPool(),
				*instance->getLogicalDevice()
			);
		}
	}

	Swapchain::~Swapchain()
	{
		delete color_image;
		delete depth_image;

		delete render_pass;

		for (auto* target : render_targets)
		{
			delete target;
		}
	}

	void Swapchain::beginRenderPass(CommandBuffer* with_buffer, size_t image_index)
	{
		/**
		 * @todo configurable
		 */
		std::array<float, 4> color_clear = {0.0f, 0.0f, 0.0f, 1.0f};

		std::array<vk::ClearValue, 2> clear_values{};
		clear_values[0].setColor({color_clear});
		clear_values[1].setDepthStencil({1.f, 0});

		vk::RenderPassBeginInfo render_pass_info{
			.renderPass		 = *render_pass->getHandle(),
			.framebuffer	 = **render_targets[image_index]->framebuffer,
			.renderArea		 = {{0, 0}, extent},
			.clearValueCount = static_cast<uint32_t>(clear_values.size()),
			.pClearValues	 = clear_values.data()
		};

		with_buffer->beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
	}

	void Swapchain::renderFrame(
		CommandBuffer* command_buffer,
		uint32_t	   image_index,
		const std::function<void(Vulkan::CommandBuffer*, uint32_t, void*)>& callback,
		void*																user_data
	)
	{
		command_buffer->begin({});

		beginRenderPass(command_buffer, image_index);

		vk::Viewport viewport = PipelineSettings::getViewportSettings(extent);

		command_buffer->setViewport(0, viewport);

		vk::Rect2D scissor{.offset = {0, 0}, .extent = extent};
		command_buffer->setScissor(0, scissor);

		// Call render callback, this does the actual render
		assert(callback && "Tried to perform frame render without a callback!");
		callback(command_buffer, image_index, user_data);

		command_buffer->endRenderPass();
		command_buffer->end();
	}

	SyncObjects::SyncObjects(vk::raii::Device& with_device)
	{
		static constexpr vk::SemaphoreCreateInfo semaphore_create_info{};
		static constexpr vk::FenceCreateInfo	 fence_create_info{
				.flags = vk::FenceCreateFlagBits::eSignaled
		};

		image_available = with_device.createSemaphore(semaphore_create_info);
		present_ready	= with_device.createSemaphore(semaphore_create_info);
		in_flight		= with_device.createFence(fence_create_info);
	}

	RenderTarget::RenderTarget(
		vk::raii::RenderPass& with_render_pass,
		vk::Extent2D		  with_extent,
		FramebufferData		  with_fb_data,
		CommandPool*		  with_command_pool,
		vk::raii::Device&	  with_device
	)
		: sync_objects(with_device),
		  command_buffer(with_command_pool->allocateCommandBuffer())
	{
		per_frame_array<vk::ImageView> attachments = {
			*with_fb_data.color,		// color
			*with_fb_data.depth,		// depth
			*with_fb_data.color_resolve // resolve color (post-multisample)
		};

		vk::FramebufferCreateInfo framebuffer_create_info{
			.renderPass		 = *with_render_pass,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments	 = attachments.data(),
			.width			 = with_extent.width,
			.height			 = with_extent.height,
			.layers			 = 1
		};

		framebuffer = new vk::raii::Framebuffer(
			with_device.createFramebuffer(framebuffer_create_info)
		);
	}

	RenderTarget::~RenderTarget()
	{
		delete command_buffer;

		delete framebuffer;
	}

} // namespace Engine::Rendering::Vulkan
