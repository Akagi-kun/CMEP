#pragma once

#include "InternalEngineObject.hpp"

#include <unordered_map>

namespace Engine
{
	class AssetManager;
	namespace Rendering
	{
		class Texture;
		struct FontChar
		{
			int x, y, width, height, xoffset, yoffset, xadvance, page, channel;
		};

		struct FontData
		{
			std::unordered_map<std::string, std::string> info;
			std::unordered_map<int, std::shared_ptr<Texture>> pages;
			std::unordered_map<int, FontChar> chars;
			unsigned int char_count = 0;
		};

		class Font final : public InternalEngineObject
		{
		private:
			AssetManager* asset_manager;
			std::string fntfile;

			// Data from fnt file
			std::unique_ptr<FontData> data;

		public:
			Font(AssetManager* managed_by = nullptr);
			~Font();

			void Init(std::unique_ptr<FontData> init_data);

			FontChar* GetChar(char ch);
			std::shared_ptr<Texture> GetPageTexture(int page);
			std::string* GetFontInfoParameter(std::string name);
		};
	} // namespace Rendering
} // namespace Engine
