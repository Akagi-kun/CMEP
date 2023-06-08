#pragma once

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

		class __declspec(dllexport) Font final
		{
		private:
			AssetManager* asset_manager;
			std::string fntfile;
			unsigned int char_count = 0;

			// Data from fnt file
			std::unordered_map<std::string, std::string> info;
			std::unordered_map<int, Texture*> pages;
			std::unordered_map<int, FontChar> chars;

			void EvalBmfont(FILE* file) noexcept;
			void EvalBmfontLine(int type, char* data) noexcept;
		public:
			Font(AssetManager* managed_by = nullptr) noexcept;
			Font(const Font& other) noexcept = delete;
			Font(const Font&& other) noexcept = delete;
			Font& operator=(const Font& other) noexcept = delete;
			Font& operator=(const Font&& other) noexcept = delete;
			~Font() noexcept;

			int Init(std::string path) noexcept;

			FontChar* GetChar(char ch) noexcept;
			Texture* GetPageTexture(int page) noexcept;
			std::string* GetFontInfoParameter(std::string name) noexcept;
		};
	}
}