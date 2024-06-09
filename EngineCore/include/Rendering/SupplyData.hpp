#pragma once

#include <memory>

namespace Engine::Rendering
{
	class Font;
	class Texture;
	class Mesh;

	enum class RendererSupplyDataType
	{
		MIN_ENUM = 0x0000,

		TEXTURE = 1,
		MESH = 2,
		FONT = 3,
		TEXT = 4,

		MAX_ENUM = 0xFFFF,
	};

	typedef struct structRendererSupplyData
	{
		RendererSupplyDataType type = RendererSupplyDataType::MIN_ENUM;
		std::shared_ptr<void> payload = nullptr;
		
		structRendererSupplyData() = default;
		structRendererSupplyData(RendererSupplyDataType type, std::shared_ptr<void> data)
		{
			this->type = type;
			this->payload = data;
		}

		~structRendererSupplyData() = default;
	} RendererSupplyData;
} // namespace Engine::Rendering
