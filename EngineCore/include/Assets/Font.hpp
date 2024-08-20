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

		struct FontData
		{
			std::unordered_map<std::string, std::string> info;
			std::unordered_map<int, std::shared_ptr<Texture>> pages;
			std::unordered_map<int, FontChar> chars;
			unsigned int char_count = 0;
		};

		class Font final : public InternalEngineObject
		{
		public:
			using InternalEngineObject::InternalEngineObject;
			~Font();

			void Init(std::unique_ptr<FontData> init_data);

			FontChar* GetChar(char character_id);
			std::shared_ptr<Texture> GetPageTexture(int page);
			std::string* GetFontInfoParameter(const std::string& name);

		private:
			std::unique_ptr<FontData> data;
		};
	} // namespace Rendering
} // namespace Engine
