#pragma once

#include "Asset.hpp"
#include "InternalEngineObject.hpp"

#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace Engine::Rendering
{
	class Texture;

	struct FontChar
	{
		using value_t = int;

		value_t x;
		value_t y;
		value_t width;
		value_t height;
		value_t xoffset;
		value_t yoffset;
		value_t xadvance;
		value_t page;
		value_t channel;
	};

	struct FontData
	{
		using page_t = std::shared_ptr<Texture>;

		std::unordered_map<std::string, std::string> info;
		std::unordered_map<int, page_t>				 pages;
		std::unordered_map<int, FontChar>			 chars;
		unsigned int								 char_count = 0;
	};

	class Font final : public InternalEngineObject, public Asset
	{
	public:
		Font(Engine* with_engine, std::unique_ptr<FontData> init_data);
		~Font();

		// Use FontChar::page, will throw exception if page isn't found
		/**
		 * @brief Get the texture representing a fonts page
		 *
		 * @param page The index of page, should be some @ref FontChar::page value
		 * @return A smart pointer to the texture
		 */
		[[nodiscard]] std::shared_ptr<Texture> getPageTexture(int page);
		/**
		 * @note const version
		 * @copydoc getPageTexture(int)
		 */
		[[nodiscard]] std::shared_ptr<const Texture> getPageTexture(int page) const;

		[[nodiscard]] std::optional<const FontChar*> getChar(char character) const;
		[[nodiscard]] std::optional<std::string>
		getFontInfoParameter(const std::string& name) const;

	private:
		std::unique_ptr<FontData> data;
	};
	static_assert(!std::is_move_constructible_v<Font> && !std::is_move_assignable_v<Font>);
} // namespace Engine::Rendering
