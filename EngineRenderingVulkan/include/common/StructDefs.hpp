#pragma once
// IWYU pragma: private; include Rendering/Vulkan/common.hpp

#include "glm/glm.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Engine::Rendering
{
	static constexpr uint16_t max_frames_in_flight = 3;
	template <typename value_type>
	using per_frame_array = std::array<value_type, max_frames_in_flight>;

	struct RendererMatrixData
	{
		glm::mat4 mat_vp{};
		glm::mat4 mat_model{};
	};

	struct QueueFamilyIndices
	{
		uint32_t graphics_family;
		uint32_t present_family;
	};

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR		  capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR>	  present_modes;
	};

	struct RenderingVertex
	{
		glm::vec3 pos{};
		glm::vec3 color{};
		glm::vec2 texcoord{};
		glm::vec3 normal{};

		RenderingVertex(
			const glm::vec3 with_pos,
			const glm::vec3 with_color	  = {},
			const glm::vec2 with_texcoord = {},
			const glm::vec3 with_normal	  = {}
		)
			: pos(with_pos), color(with_color), texcoord(with_texcoord),
			  normal(with_normal)
		{}

		static constexpr std::array<vk::VertexInputBindingDescription, 1>
		getBindingDescription()
		{
			return {
				vk::VertexInputBindingDescription{
					0,
					sizeof(RenderingVertex),
					vk::VertexInputRate::eVertex,
				},
			};
		}

		static constexpr std::array<vk::VertexInputAttributeDescription, 4>
		getAttributeDescriptions()
		{
			return {
				vk::VertexInputAttributeDescription{
					0,
					0,
					vk::Format::eR32G32B32Sfloat,
					offsetof(RenderingVertex, pos),
				},
				vk::VertexInputAttributeDescription{
					1,
					0,
					vk::Format::eR32G32B32Sfloat,
					offsetof(RenderingVertex, color),
				},
				vk::VertexInputAttributeDescription{
					2,
					0,
					vk::Format::eR32G32Sfloat,
					offsetof(RenderingVertex, texcoord),
				},
				vk::VertexInputAttributeDescription{
					3,
					0,
					vk::Format::eR32G32B32Sfloat,
					offsetof(RenderingVertex, normal),
				}
			};
		}
	};
} // namespace Engine::Rendering
