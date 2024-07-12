#pragma once

#include "Assets/Font.hpp"

#include "InternalEngineObject.hpp"

namespace Engine::Factories
{
	class FontFactory : public InternalEngineObject
	{
	private:
		void EvalBmfont(std::unique_ptr<Rendering::FontData>& font, std::ifstream& fontFile);
		void EvalBmfontLine(std::unique_ptr<Rendering::FontData>& font, int type, char* data);

	public:
		using InternalEngineObject::InternalEngineObject;

		std::shared_ptr<Rendering::Font> InitBMFont(const std::string& fontPath);
	};
} // namespace Engine::Factories
