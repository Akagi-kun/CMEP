#pragma once

#include "Assets/Font.hpp"

#include "InternalEngineObject.hpp"

namespace Engine::Factories
{
	class FontFactory : public InternalEngineObject
	{
	private:
		// AssetManager* asset_manager;

		void EvalBmfont(std::unique_ptr<Rendering::FontData>& font, std::ifstream& fontFile);
		void EvalBmfontLine(std::unique_ptr<Rendering::FontData>& font, int type, char* data);

	public:
		using InternalEngineObject::InternalEngineObject;
		// FontFactory(AssetManager* manager);

		std::shared_ptr<Rendering::Font> InitBMFont(std::string fontPath);
	};
} // namespace Engine::Factories
