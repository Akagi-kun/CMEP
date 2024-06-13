#pragma once

#include "Rendering/Transform.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace Engine::Rendering
{
	class Font;
	class Texture;
	class Mesh;

	enum class RendererSupplyDataType
	{
		MIN_ENUM = 0x00,

		TEXTURE = 1,
		MESH = 2,
		FONT = 3,

		TEXT = 32,

		MAX_ENUM = 0xFF
	};

	struct RendererTransformUpdate
	{
		Transform current;
		Transform parent;
		uint_fast16_t screen_x, screen_y;
	};

	typedef struct RendererSupplyData_struct
	{
		RendererSupplyDataType type = RendererSupplyDataType::MIN_ENUM;

		std::shared_ptr<void> payload_ptr = nullptr;
		std::string payload_string;

		RendererSupplyData_struct() = default;

		RendererSupplyData_struct(RendererSupplyDataType with_type, std::shared_ptr<void> data)
		{
			this->type = with_type;
			this->payload_ptr = data;
		}

		RendererSupplyData_struct(RendererSupplyDataType with_type, std::string data)
		{
			this->type = with_type;
			this->payload_string = data;
		}

		~RendererSupplyData_struct()
		{
		}
	} RendererSupplyData;
} // namespace Engine::Rendering
