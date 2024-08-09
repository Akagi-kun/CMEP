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
		{"renderer_2d"sv, value_type::RENDERER_2D},
		{"renderer_3d"sv, value_type::RENDERER_3D},
	};

	template <>
	EnumStringConvertor<MeshBuilderType>::map_type EnumStringConvertor<MeshBuilderType>::type_map = {
		{"sprite"sv, value_type::SPRITE},
		{"text"sv, value_type::TEXT},
		{"axis"sv, value_type::AXIS},
		{"generator"sv, value_type::GENERATOR},
	};

	template <>
	EnumStringConvertor<VkFilter>::map_type EnumStringConvertor<VkFilter>::type_map = {
		{"nearest"sv, VK_FILTER_NEAREST},
		{"linear"sv, VK_FILTER_LINEAR},
	};

	template <>
	EnumStringConvertor<VkSamplerAddressMode>::map_type EnumStringConvertor<VkSamplerAddressMode>::type_map = {
		{"repeat"sv, VK_SAMPLER_ADDRESS_MODE_REPEAT},
		{"clamp"sv, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE},
		{"clamp_border"sv, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER},
	};

	template <>
	EnumStringConvertor<VkPrimitiveTopology>::map_type EnumStringConvertor<VkPrimitiveTopology>::type_map = {
		{"lines"sv, VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
		{"triangles"sv, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST}
	};

	using namespace Rendering;

	template <>
	EnumStringConvertor<RendererSupplyDataType>::map_type EnumStringConvertor<RendererSupplyDataType>::type_map = {
		{"texture"sv, value_type::TEXTURE},
		{"font"sv, value_type::FONT},
		{"text"sv, value_type::TEXT},
		{"generator_script"sv, value_type::GENERATOR_SCRIPT},
		{"generator_supplier"sv, value_type::GENERATOR_SUPPLIER}
	};
} // namespace Engine
