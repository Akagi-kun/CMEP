#pragma once
// IWYU pragma: private; include Rendering/Vulkan/rendering.hpp

#include "fwd.hpp"

#include "common/HandleWrapper.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace Engine::Rendering::Vulkan
{
	class RenderPass final : public HandleWrapper<vk::raii::RenderPass, false>
	{
	public:
		RenderPass(
			const PhysicalDevice* with_physical_device,
			LogicalDevice*		  with_logical_device,
			vk::Format			  with_format
		);
		~RenderPass() = default;
	};
} // namespace Engine::Rendering::Vulkan
