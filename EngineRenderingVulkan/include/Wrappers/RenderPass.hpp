#pragma once

#include "ImportVulkan.hpp"
#include "InstanceOwned.hpp"
#include "framework.hpp"

namespace Engine::Rendering::Vulkan
{
	struct RenderPass final : public InstanceOwned
	{
		vk::raii::RenderPass native_handle = nullptr;

		RenderPass(InstanceOwned::value_t with_instance, vk::Format with_format);
		~RenderPass() = default;
	};
} // namespace Engine::Rendering::Vulkan
