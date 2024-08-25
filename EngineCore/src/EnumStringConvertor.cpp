#include "EnumStringConvertor.hpp"

#include "Factories/ObjectFactory.hpp"

#include "EventHandling.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace Engine
{
	using namespace std::literals::string_view_literals;

	template <>
	EnumStringConvertor<EventHandling::EventType>::map_t EnumStringConvertor<EventHandling::EventType>::type_map = {
		{"on_init"sv, value_t::ON_INIT},
		{"on_mouse_moved"sv, value_t::ON_MOUSEMOVED},
		{"on_key_down"sv, value_t::ON_KEYDOWN},
		{"on_key_up"sv, value_t::ON_KEYUP},
		{"on_update"sv, value_t::ON_UPDATE},
	};

	using namespace Factories::ObjectFactory;

	template <>
	EnumStringConvertor<RendererType>::map_t EnumStringConvertor<RendererType>::type_map = {
		{"renderer_2d"sv, value_t::RENDERER_2D},
		{"renderer_3d"sv, value_t::RENDERER_3D},
	};

	template <>
	EnumStringConvertor<MeshBuilderType>::map_t EnumStringConvertor<MeshBuilderType>::type_map = {
		{"sprite"sv, value_t::SPRITE},
		{"text"sv, value_t::TEXT},
		{"axis"sv, value_t::AXIS},
		{"generator"sv, value_t::GENERATOR},
	};

	template <>
	EnumStringConvertor<vk::Filter>::map_t EnumStringConvertor<vk::Filter>::type_map = {
		{"nearest"sv, vk::Filter::eNearest},
		{"linear"sv, vk::Filter::eLinear},
	};

	template <>
	EnumStringConvertor<vk::SamplerAddressMode>::map_t EnumStringConvertor<vk::SamplerAddressMode>::type_map = {
		{"repeat"sv, vk::SamplerAddressMode::eRepeat},
		{"clamp"sv, vk::SamplerAddressMode::eClampToEdge},
		{"clamp_border"sv, vk::SamplerAddressMode::eClampToBorder},
	};

	template <>
	EnumStringConvertor<VkPrimitiveTopology>::map_t EnumStringConvertor<VkPrimitiveTopology>::type_map = {
		{"lines"sv, VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
		{"triangles"sv, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST}
	};

	using namespace Rendering;

	template <>
	EnumStringConvertor<RendererSupplyDataType>::map_t EnumStringConvertor<RendererSupplyDataType>::type_map = {
		{"texture"sv, value_t::TEXTURE},
		{"font"sv, value_t::FONT},
		{"text"sv, value_t::TEXT},
		{"generator_script"sv, value_t::GENERATOR_SCRIPT},
		{"generator_supplier"sv, value_t::GENERATOR_SUPPLIER}
	};
} // namespace Engine
