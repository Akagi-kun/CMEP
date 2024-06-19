#pragma once

#include "Rendering/Transform.hpp"

#include <memory>
#include <string>
#include <utility>

namespace Engine::Rendering
{
	class Font;
	class Texture;
	class Mesh;

	enum class RendererSupplyDataType
	{
		MIN_ENUM = 0x00,

		TEXTURE = 1,
		MESH	= 2,
		FONT	= 3,

		TEXT = 32,

		MAX_ENUM = 0xFF
	};

	typedef struct RendererTransformUpdate_struct
	{
		Transform current;
		Transform parent;
		ScreenSize screen;
	} RendererTransformUpdate;

	typedef struct RendererSupplyData_struct
	{
		RendererSupplyDataType type = RendererSupplyDataType::MIN_ENUM;

		std::shared_ptr<void> payload_ptr = nullptr;
		std::string payload_string;

		RendererSupplyData_struct(RendererSupplyDataType with_type, std::shared_ptr<void> data)
		{
			this->type		  = with_type;
			this->payload_ptr = std::move(data);
		}

		RendererSupplyData_struct(RendererSupplyDataType with_type, std::string data)
		{
			this->type			 = with_type;
			this->payload_string = std::move(data);
		}

	} RendererSupplyData;
} // namespace Engine::Rendering
