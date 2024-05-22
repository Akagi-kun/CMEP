#pragma once

#include "Rendering/Texture.hpp"
#include "Rendering/Font.hpp"

#include "InternalEngineObject.hpp"

namespace Engine::Factories
{
	class FontFactory : public InternalEngineObject
	{
	private:
		AssetManager* asset_manager;

		void EvalBmfont(Rendering::FontData& font, std::ifstream& fontFile);
		void EvalBmfontLine(Rendering::FontData& font, int type, char* data);
	public:
		FontFactory(AssetManager* manager);

		std::shared_ptr<Rendering::Font> InitBMFont(std::string fontPath);
	};
}