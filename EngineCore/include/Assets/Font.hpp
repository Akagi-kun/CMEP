#pragma once

#include "InternalEngineObject.hpp"

#include <unordered_map>

namespace Engine
{
	class AssetManager;
	namespace Rendering
	{
		class Texture;

#pragma pack(1)
		struct FontChar
		{
			int x;
			int y;
			int width;
			int height;
			int xoffset;
			int yoffset;
			int xadvance;
			int page;
			int channel;
		};
#pragma pack()

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
			std::string fntfile;

			// Data from fnt file
			std::unique_ptr<FontData> data;

		public:
			using InternalEngineObject::InternalEngineObject;
			// Font(AssetManager* managed_by = nullptr);
			~Font();

			void Init(std::unique_ptr<FontData> init_data);

			FontChar* GetChar(char ch);
			std::shared_ptr<Texture> GetPageTexture(int page);
			std::string* GetFontInfoParameter(std::string name);
		};
	} // namespace Rendering
} // namespace Engine
