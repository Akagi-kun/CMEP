#pragma once

#include "InternalEngineObject.hpp"
#include "PlatformSemantics.hpp"

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

		class CMEP_EXPORT Font final : public InternalEngineObject
		{
		private:
			AssetManager* asset_manager;
			std::string fntfile;

			// Data from fnt file
			FontData data;

		public:
			Font(AssetManager* managed_by = nullptr);
			~Font();

			int Init(FontData data);

			FontChar* GetChar(char ch);
			std::shared_ptr<Texture> GetPageTexture(int page);
			std::string* GetFontInfoParameter(std::string name);
		};
	}
}