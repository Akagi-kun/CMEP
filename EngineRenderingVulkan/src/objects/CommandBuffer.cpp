#include "objects/CommandBuffer.hpp"

#include "ImportVulkan.hpp"
#include "backend/Instance.hpp"
#include "backend/LogicalDevice.hpp"
#include "common/InstanceOwned.hpp"
#include "common/Utilities.hpp"
#include "objects/Buffer.hpp"
#include "objects/Image.hpp"

namespace Engine::Rendering::Vulkan
{
	CommandBuffer::CommandBuffer(InstanceOwned::value_t with_instance, vk::raii::CommandPool& from_pool)
		: InstanceOwned(with_instance), owning_pool(*from_pool)
	{
		vk::CommandBufferAllocateInfo alloc_info(owning_pool, vk::CommandBufferLevel::ePrimary, 1, {});

		LogicalDevice* logical_device = instance->GetLogicalDevice();

		native_handle = std::move(logical_device->GetHandle().allocateCommandBuffers(alloc_info)[0]);
	}

	void CommandBuffer::BeginCmdBuffer(vk::CommandBufferUsageFlags usage_flags)
	{
		native_handle.reset();

		native_handle.begin({usage_flags, {}, {}});
	}

	void CommandBuffer::RecordCmds(std::function<void(CommandBuffer*)> const& lambda)
	{
		LogicalDevice* device = instance->GetLogicalDevice();

		// TODO: Check if we should really pass non-zero here
		BeginCmdBuffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		// TODO: Don't pass this here
		lambda(this);

		native_handle.end();
		// EndCmdBuffer();
		QueueSubmit(device->GetGraphicsQueue());

		device->GetGraphicsQueue().waitIdle();
	}

	void CommandBuffer::QueueSubmit(const vk::raii::Queue& to_queue)
	{
		vk::SubmitInfo submit_info{};
		submit_info.setCommandBuffers(*native_handle);

		to_queue.submit(submit_info);
	}

	void CommandBuffer::BufferBufferCopy(Buffer* from_buffer, Buffer* to_buffer, std::vector<vk::BufferCopy> regions)
	{
		RecordCmds([&](CommandBuffer* handle) {
			handle->native_handle.copyBuffer(*from_buffer->GetHandle(), *to_buffer->GetHandle(), regions);
		});
	}

	void CommandBuffer::BufferImageCopy(Buffer* from_buffer, Image* to_image)
	{
		ImageSize image_size = to_image->GetSize();

		// Sane defaults
		vk::BufferImageCopy region(
			{},
			{},
			{},
			{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
			{0, 0, 0},
			Utility::ConvertToExtent<vk::Extent3D>(image_size, 1)
		);

		RecordCmds([&](CommandBuffer* with_buf) {
			with_buf->native_handle.copyBufferToImage(
				*from_buffer->GetHandle(),
				*to_image->GetHandle(),
				vk::ImageLayout::eTransferDstOptimal,
				region
			);
		});
	}

} // namespace Engine::Rendering::Vulkan
