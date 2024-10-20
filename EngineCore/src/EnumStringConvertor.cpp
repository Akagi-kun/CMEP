#include "EnumStringConvertor.hpp"

#include "Assets/AssetManager.hpp"

#include "Factories/ObjectFactory.hpp"

#include "EventHandling.hpp"

namespace Engine
{
	using namespace std::literals::string_view_literals;

	template <>
	EnumStringConvertor<EventHandling::EventType>::map_t
		EnumStringConvertor<EventHandling::EventType>::value_map = {
			{"on_init"sv, value_t::onInit},
			{"on_mouse_moved"sv, value_t::onMouseMoved},
			{"on_key_down"sv, value_t::onKeyDown},
			{"on_key_up"sv, value_t::onKeyUp},
			{"on_update"sv, value_t::onUpdate},
	};

	using namespace Factories::ObjectFactory;

	template <>
	EnumStringConvertor<RendererType>::map_t EnumStringConvertor<RendererType>::value_map = {
		{"renderer_2d"sv, value_t::RENDERER_2D},
		{"renderer_3d"sv, value_t::RENDERER_3D},
	};

	template <>
	EnumStringConvertor<MeshBuilderType>::map_t EnumStringConvertor<MeshBuilderType>::value_map = {
		{"sprite"sv, value_t::SPRITE},
		{"text"sv, value_t::TEXT},
		{"axis"sv, value_t::AXIS},
		{"generator"sv, value_t::GENERATOR},
	};

	template <>
	EnumStringConvertor<vk::Filter>::map_t EnumStringConvertor<vk::Filter>::value_map = {
		{"nearest"sv, vk::Filter::eNearest},
		{"linear"sv, vk::Filter::eLinear},
	};

	template <>
	EnumStringConvertor<vk::SamplerAddressMode>::map_t
		EnumStringConvertor<vk::SamplerAddressMode>::value_map = {
			{"repeat"sv, vk::SamplerAddressMode::eRepeat},
			{"clamp"sv, vk::SamplerAddressMode::eClampToEdge},
			{"clamp_border"sv, vk::SamplerAddressMode::eClampToBorder},
	};

	template <>
	EnumStringConvertor<VkPrimitiveTopology>::map_t
		EnumStringConvertor<VkPrimitiveTopology>::value_map = {
			{"lines"sv, VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
			{"triangles"sv, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST}
	};

	using namespace Rendering;

	template <>
	EnumStringConvertor<RendererSupplyData::Type>::map_t
		EnumStringConvertor<RendererSupplyData::Type>::value_map = {
			{"texture"sv, value_t::TEXTURE},
			{"font"sv, value_t::FONT},
	};

	template <>
	EnumStringConvertor<MeshBuilderSupplyData::Type>::map_t
		EnumStringConvertor<MeshBuilderSupplyData::Type>::value_map = {
			{"text"sv, value_t::TEXT},
			{"generator"sv, value_t::GENERATOR},
	};

	template <>
	EnumStringConvertor<AssetType>::map_t EnumStringConvertor<AssetType>::value_map = {
		{"font"sv, value_t::FONT},
		{"texture"sv, value_t::TEXTURE},
		{"script", value_t::SCRIPT},
	};
} // namespace Engine
