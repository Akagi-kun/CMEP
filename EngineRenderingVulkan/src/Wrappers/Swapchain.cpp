#include "Wrappers/Swapchain.hpp"

#include "VulkanStructDefs.hpp"
#include "VulkanUtilities.hpp"
#include "Wrappers/CommandBuffer.hpp"
#include "Wrappers/CommandPool.hpp"
#include "Wrappers/Image.hpp"
#include "Wrappers/Instance.hpp"
#include "Wrappers/RenderPass.hpp"

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
			*instance->GetPhysicalDevice()
		);
		surface_format = Vulkan::Utils::ChooseVulkanSwapSurfaceFormat(swap_chain_support.formats);

		vk::PresentModeKHR present_mode = Vulkan::Utils::ChooseVulkanSwapPresentMode(swap_chain_support.present_modes);

		QueueFamilyIndices queue_indices = logical_device->GetQueueFamilies();

		uint32_t queue_family_indices[] = {queue_indices.graphics_family, queue_indices.present_family};

		bool queue_families_same = queue_indices.graphics_family != queue_indices.present_family;

		vk::SwapchainCreateInfoKHR create_info(
			{},
			with_surface->native_handle,
			with_count,
			surface_format.format,
			surface_format.colorSpace,
			with_extent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment,
			queue_families_same ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
			queue_families_same ? 2 : uint32_t{},
			queue_families_same ? queue_family_indices : nullptr,
			swap_chain_support.capabilities.currentTransform,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			present_mode,
			vk::True
		);

		native_handle = logical_device->GetHandle().createSwapchainKHR(create_info);

		image_handles = native_handle.getImages();

		// Create image views
		for (size_t i = 0; i < image_handles.size(); i++)
		{
			vk::ImageViewCreateInfo view_create_info(
				{},
				image_handles[i],
				vk::ImageViewType::e2D,
				surface_format.format,
				{},
				{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
			);

			image_view_handles.push_back(logical_device->GetHandle().createImageView(view_create_info));
		}

		vk::Format depth_format = instance->GetPhysicalDevice()->FindSupportedDepthFormat();

		depth_buffer = new Image(
			instance,
			{extent.width, extent.height},
			instance->GetMSAASamples(),
			depth_format,
			vk::ImageUsageFlagBits::eDepthStencilAttachment
		);
		depth_buffer->AddImageView(vk::ImageAspectFlagBits::eDepth);

		multisampled_color_image = new Image(
			instance,
			{extent.width, extent.height},
			instance->GetMSAASamples(),
			surface_format.format,
			vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment
		);
		multisampled_color_image->AddImageView(vk::ImageAspectFlagBits::eColor);

		render_pass = new RenderPass(instance, surface_format.format);

		framebuffers.resize(image_view_handles.size());

		for (size_t i = 0; i < image_view_handles.size(); i++)
		{
			per_frame_array<vk::ImageView> attachments = {
				**multisampled_color_image->GetNativeViewHandle(),
				**depth_buffer->GetNativeViewHandle(),
				*image_view_handles[i]
			};

			vk::FramebufferCreateInfo
				framebuffer_create_info({}, *render_pass->native_handle, attachments, extent.width, extent.height, 1);

			framebuffers[i] = new vk::raii::Framebuffer(
				logical_device->GetHandle().createFramebuffer(framebuffer_create_info)
			);
		}

		for (auto& target : render_targets)
		{
			target = new RenderTarget(instance->GetCommandPool(), instance->GetLogicalDevice()->GetHandle());
		}
	}

	Swapchain::~Swapchain()
	{
		for (auto* framebuffer : framebuffers)
		{
			delete framebuffer;
		}

		delete multisampled_color_image;
		delete depth_buffer;

		delete render_pass;

		for (auto* target : render_targets)
		{
			delete target;
		}
	}

	void Swapchain::BeginRenderPass(CommandBuffer* with_buffer, size_t image_index)
	{
		std::array<vk::ClearValue, 2> clear_values{};
		clear_values[0].setColor({0.0f, 0.0f, 0.0f, 1.0f}); // TODO: configurable
		clear_values[1].setDepthStencil({1.f, 0});

		vk::RenderPassBeginInfo render_pass_info(
			*render_pass->native_handle,
			**framebuffers[image_index],
			{{0, 0}, extent},
			clear_values,
			{}
		);

		with_buffer->BeginRenderPass(render_pass_info);
	}

	void Swapchain::RenderFrame(
		CommandBuffer* command_buffer,
		uint32_t image_index,
		const std::function<void(Vulkan::CommandBuffer*, uint32_t, void*)>& callback,
		void* user_data
	)
	{
		command_buffer->BeginCmdBuffer({});

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

	RenderTarget::RenderTarget(CommandPool* with_command_pool, vk::raii::Device& with_device)
	{
		command_buffer = with_command_pool->AllocateCommandBuffer();

		static constexpr vk::SemaphoreCreateInfo semaphore_create_info({}, {});
		static constexpr vk::FenceCreateInfo fence_create_info(vk::FenceCreateFlagBits::eSignaled, {});

		sync_objects = SyncObjects{
			with_device.createSemaphore(semaphore_create_info),
			with_device.createSemaphore(semaphore_create_info),
			with_device.createFence(fence_create_info),
		};
	}

	RenderTarget::~RenderTarget()
	{
		// delete sync_objects;
		delete command_buffer;
	}

#pragma region Private

} // namespace Engine::Rendering::Vulkan
