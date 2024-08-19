#include "Wrappers/Swapchain.hpp"

#include "VulkanStructDefs.hpp"
#include "VulkanUtilities.hpp"
#include "Wrappers/CommandBuffer.hpp"
#include "Wrappers/CommandPool.hpp"
#include "Wrappers/Image.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/RenderPass.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	Swapchain::Swapchain(
		InstanceOwned::value_t with_instance,
		Surface* with_surface,
		VkExtent2D with_extent,
		uint32_t with_count
	)
		: InstanceOwned(with_instance), extent(with_extent)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		// Query details for support of swapchains
		SwapChainSupportDetails swap_chain_support = with_surface->QueryVulkanSwapChainSupport(
			instance->GetPhysicalDevice()
		);
		vk::SurfaceFormatKHR surface_format = Vulkan::Utils::ChooseVulkanSwapSurfaceFormat(swap_chain_support.formats);

		vk::PresentModeKHR present_mode = Vulkan::Utils::ChooseVulkanSwapPresentMode(swap_chain_support.present_modes);

		QueueFamilyIndices queue_indices = logical_device->GetQueueFamilies();
		uint32_t queue_family_indices[] = {queue_indices.graphics_family.value(), queue_indices.present_family.value()};

		bool identical_queue_families = queue_indices.graphics_family != queue_indices.present_family;

		image_format = surface_format.format;

		vk::SwapchainCreateInfoKHR create_info(
			{},
			with_surface->native_handle,
			with_count,
			surface_format.format,
			surface_format.colorSpace,
			with_extent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment,
			identical_queue_families ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
			identical_queue_families ? 2 : uint32_t{},
			identical_queue_families ? queue_family_indices : nullptr,
			swap_chain_support.capabilities.currentTransform,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			present_mode,
			vk::True
		);

		native_handle = logical_device->GetHandle().createSwapchainKHR(create_info);

		image_handles = logical_device->GetHandle().getSwapchainImagesKHR(native_handle);

		// Create image views
		this->image_view_handles.resize(this->image_handles.size());
		for (size_t i = 0; i < this->image_handles.size(); i++)
		{
			vk::ImageViewCreateInfo view_create_info(
				{},
				image_handles[i],
				vk::ImageViewType::e2D,
				surface_format.format,
				{},
				{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
			);

			image_view_handles[i] = logical_device->GetHandle().createImageView(view_create_info);
		}

		vk::Format depth_format = instance->GetPhysicalDevice().FindSupportedDepthFormat();

		this->depth_buffer = new Image(
			instance,
			{this->extent.width, this->extent.height},
			instance->GetMSAASamples(),
			depth_format,
			vk::ImageUsageFlagBits::eDepthStencilAttachment
		);
		this->depth_buffer->AddImageView(vk::ImageAspectFlagBits::eDepth);

		this->multisampled_color_image = new Image(
			instance,
			{this->extent.width, this->extent.height},
			instance->GetMSAASamples(),
			image_format,
			vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment
		);
		this->multisampled_color_image->AddImageView(vk::ImageAspectFlagBits::eColor);

		this->render_pass = new RenderPass(instance, image_format);

		this->framebuffers.resize(image_view_handles.size());

		for (size_t i = 0; i < image_view_handles.size(); i++)
		{
			per_frame_array<vk::ImageView> attachments = {
				multisampled_color_image->GetNativeViewHandle(),
				depth_buffer->GetNativeViewHandle(),
				image_view_handles[i]
			};

			vk::FramebufferCreateInfo
				framebuffer_create_info({}, render_pass->native_handle, attachments, extent.width, extent.height, 1);

			framebuffers[i] = logical_device->GetHandle().createFramebuffer(framebuffer_create_info);
		}

		for (auto& target : this->render_targets)
		{
			this->CreateRenderTarget(target);
		}
	}

	Swapchain::~Swapchain()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		for (auto framebuffer : framebuffers)
		{
			logical_device->GetHandle().destroyFramebuffer(framebuffer);
		}

		for (auto image_view : image_view_handles)
		{
			logical_device->GetHandle().destroyImageView(image_view);
		}

		logical_device->GetHandle().destroySwapchainKHR(native_handle);

		delete multisampled_color_image;
		delete depth_buffer;

		delete render_pass;

		for (auto& target : this->render_targets)
		{
			this->CleanupRenderTarget(target);
		}
	}

	void Swapchain::BeginRenderPass(CommandBuffer* with_buffer, size_t image_index)
	{
		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType			   = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass		   = this->render_pass->native_handle;
		render_pass_info.framebuffer	   = this->framebuffers[image_index];
		render_pass_info.renderArea.offset = {0, 0};
		render_pass_info.renderArea.extent = this->extent;

		std::array<VkClearValue, 2> clear_values{};
		clear_values[0].color		 = {{0.0f, 0.0f, 0.0f, 1.0f}}; // TODO: configurable
		clear_values[1].depthStencil = {1.0f, 0};

		render_pass_info.clearValueCount = 2;
		render_pass_info.pClearValues	 = clear_values.data();

		with_buffer->BeginRenderPass(&render_pass_info);
	}

	void Swapchain::RenderFrame(
		CommandBuffer* command_buffer,
		uint32_t image_index,
		const std::function<void(Vulkan::CommandBuffer*, uint32_t, void*)>& callback,
		void* user_data
	)
	{
		command_buffer->BeginCmdBuffer(0);

		BeginRenderPass(command_buffer, image_index);

		vk::Viewport viewport(0.f, 0.f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.f, 1.f);
		command_buffer->GetHandle().setViewport(0, viewport);

		vk::Rect2D scissor({0, 0}, extent);
		command_buffer->GetHandle().setScissor(0, scissor);

		// Perform actual render
		if (!callback)
		{
			throw std::invalid_argument("Tried to perform frame render without a callback!");
		}

		assert(callback);
		callback(command_buffer, image_index, user_data);

		command_buffer->EndRenderPass();
		command_buffer->EndCmdBuffer();
	}

#pragma region Private

	void Swapchain::CreateRenderTarget(RenderTargetData& target)
	{
		target.command_buffer = instance->GetCommandPool()->AllocateCommandBuffer();

		this->CreateSyncObjects(target.sync_objects);
	}

	void Swapchain::CreateSyncObjects(SyncObjects& sync_objects)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		static constexpr vk::SemaphoreCreateInfo semaphore_create_info({}, {});
		static constexpr vk::FenceCreateInfo fence_create_info(vk::FenceCreateFlagBits::eSignaled, {});

		sync_objects.image_available = logical_device->GetHandle().createSemaphore(semaphore_create_info);
		sync_objects.present_ready	 = logical_device->GetHandle().createSemaphore(semaphore_create_info);
		sync_objects.in_flight		 = logical_device->GetHandle().createFence(fence_create_info);
	}

	void Swapchain::CleanupRenderTarget(RenderTargetData& target)
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		logical_device->GetHandle().destroySemaphore(target.sync_objects.present_ready);
		logical_device->GetHandle().destroySemaphore(target.sync_objects.image_available);
		logical_device->GetHandle().destroyFence(target.sync_objects.in_flight);

		delete target.command_buffer;
	}
} // namespace Engine::Rendering::Vulkan
