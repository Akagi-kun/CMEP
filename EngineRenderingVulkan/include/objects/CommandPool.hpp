#pragma once

#include "fwd.hpp"

#include "ImportVulkan.hpp"
#include "common/HandleWrapper.hpp"
#include "common/InstanceOwned.hpp"

#include <cassert>

namespace Engine::Rendering::Vulkan
{
	class CommandPool : public InstanceOwned, public HandleWrapper<vk::raii::CommandPool>
	{
	public:
		CommandPool(InstanceOwned::value_t with_instance);

		~CommandPool() = default;

		[[nodiscard]] CommandBuffer* AllocateCommandBuffer();
		[[nodiscard]] CommandBuffer ConstructCommandBuffer();
	};
} // namespace Engine::Rendering::Vulkan
