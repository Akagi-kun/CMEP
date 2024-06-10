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

		TEXT = 32,

		MAX_ENUM = 0xFF,
	};

	typedef struct RendererSupplyData_struct
	{
		RendererSupplyDataType type = RendererSupplyDataType::MIN_ENUM;

		std::shared_ptr<void> payload_ptr = nullptr;
		std::string payload_string{};
		
		RendererSupplyData_struct() = default;

		RendererSupplyData_struct(RendererSupplyDataType type, std::shared_ptr<void> data)
		{
			this->type = type;
			this->payload_ptr = data;
		}

		RendererSupplyData_struct(RendererSupplyDataType type, std::string data)
		{
			this->type = type;
			this->payload_string = data;
		}

		~RendererSupplyData_struct() {};
	} RendererSupplyData;
} // namespace Engine::Rendering
