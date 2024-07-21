#include "EnumStringConvertor.hpp"

#include "Factories/ObjectFactory.hpp"

#include "EventHandling.hpp"
#include "vulkan/vulkan_core.h"

namespace Engine
{
	using namespace std::literals::string_view_literals;

	template <>
	EnumStringConvertor<EventHandling::EventType>::map_type EnumStringConvertor<EventHandling::EventType>::type_map = {
		{"on_init"sv, value_type::ON_INIT},
		{"on_mouse_moved"sv, value_type::ON_MOUSEMOVED},
		{"on_key_down"sv, value_type::ON_KEYDOWN},
		{"on_key_up"sv, value_type::ON_KEYUP},
		{"on_update"sv, value_type::ON_UPDATE},
	};

	using namespace Factories::ObjectFactory;

	template <>
	EnumStringConvertor<RendererType>::map_type EnumStringConvertor<RendererType>::type_map = {
		{"generic_2d"sv, value_type::GENERIC_2D},
		{"generic_3d"sv, value_type::GENERIC_3D},
	};

	template <>
	EnumStringConvertor<MeshBuilderType>::map_type EnumStringConvertor<MeshBuilderType>::type_map = {
		{"sprite"sv, value_type::SPRITE},
		{"text"sv, value_type::TEXT},
		{"axis"sv, value_type::AXIS},
		{"mesh"sv, value_type::MESH},
	};

	template <>
	EnumStringConvertor<VkFilter>::map_type EnumStringConvertor<VkFilter>::type_map = {
		{"nearest", VK_FILTER_NEAREST},
		{"linear", VK_FILTER_LINEAR},
	};

	template <>
	EnumStringConvertor<VkSamplerAddressMode>::map_type EnumStringConvertor<VkSamplerAddressMode>::type_map = {
		{"repeat", VK_SAMPLER_ADDRESS_MODE_REPEAT},
		{"clamp", VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE},
		{"clamp_border", VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER},
	};
} // namespace Engine
