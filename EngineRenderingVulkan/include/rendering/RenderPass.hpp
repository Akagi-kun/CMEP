#pragma once

#include "fwd.hpp"

#include "ImportVulkan.hpp"
#include "common/InstanceOwned.hpp"

namespace Engine::Rendering::Vulkan
{
	class RenderPass final : public InstanceOwned
	{
	public:
		vk::raii::RenderPass native_handle = nullptr;

		RenderPass(InstanceOwned::value_t with_instance, vk::Format with_format);
		~RenderPass() = default;
	};
} // namespace Engine::Rendering::Vulkan
