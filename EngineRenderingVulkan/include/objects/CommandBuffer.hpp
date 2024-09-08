#pragma once

#include "fwd.hpp"

#include "common/HandleWrapper.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <functional>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	class CommandBuffer final : public HandleWrapper<vk::raii::CommandBuffer>
	{
	public:
		CommandBuffer(LogicalDevice* with_device, vk::raii::CommandPool& from_pool);
		~CommandBuffer() = default;

		// TODO: Remove this in the future
		void recordCmds(std::function<void(vk::raii::CommandBuffer*)> const& lambda);

		void copyBufferBuffer(
			Buffer*						from_buffer,
			Buffer*						to_buffer,
			std::vector<vk::BufferCopy> regions
		);
		void copyBufferImage(Buffer* from_buffer, Image* to_image);

		static vk::CommandBufferBeginInfo getBeginInfo(vk::CommandBufferUsageFlags usage_flags);

	private:
		LogicalDevice* device;
	};
} // namespace Engine::Rendering::Vulkan
