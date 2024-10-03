#include "Rendering/MeshBuilders/TextMeshBuilder.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/MeshBuilders/IMeshBuilder.hpp"
#include "Rendering/MeshBuilders/MeshBuildContext.hpp"
#include "Rendering/SupplyData.hpp"
#include "Rendering/Transform.hpp"

#include "Logging/Logging.hpp"

#include "Engine.hpp"
#include "Exception.hpp"

#include "glm/glm.hpp"

#include <algorithm>
#include <cassert>
#include <format>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace Engine::Rendering
{
	void TextMeshBuilder::supplyData(const MeshBuilderSupplyData& data)
	{
		IMeshBuilder::supplyData(data);

		switch (data.type)
		{
			case MeshBuilderSupplyData::Type::FONT:
			{
				const auto& payload_ref = std::get<std::weak_ptr<Font>>(data.payload);
				assert(!payload_ref.expired() && "Cannot lock expired payload!");

				font = payload_ref.lock();
				break;
			}
			case MeshBuilderSupplyData::Type::TEXT:
			{
				const auto& payload_ref = std::get<std::string>(data.payload);

				text.assign(payload_ref);
				break;
			}
			default:
			{
				break;
			}
		}
	}

	void TextMeshBuilder::build()
	{
		if (context.vbo != nullptr)
		{
			delete context.vbo;

			context.vbo = nullptr;
			mesh.clear();
		}

		const auto* window_data = owner_engine->getVulkanInstance()->getWindow();
		screen_size				= window_data->getFramebufferSize();

		auto size_param = font->getFontInfoParameter("size");
		if (!size_param.has_value())
		{
			throw ENGINE_EXCEPTION(
				"Could not find parameter 'size' in font! Malformed font file?"
			);
		}
		int font_size = std::stoi(size_param.value());

		std::vector<RenderingVertex> generated_mesh;

		// Positions that denote the origin for a specific character
		// Moved as the mesh is built
		float char_origin_x = 0.f;
		float char_origin_y = 0.f;

		// Iteratively generate a quad for each character based on font information
		for (char character : text)
		{
			// Following calculations assume that font sizes are specified in pixels
			// Selected font size is assumed to be 1px in all calculations
			//
			// Ratio between 1px and font size (as defined by the .fnt file)
			// (as font size increases, this value decreases)
			const auto font_size_ratio = (1.0f / static_cast<float>(font_size));

			// Ratio between 1px and the screen size
			// (as screen size increases, this value decreases)
			const auto screen_size_ratio = (1.0f / static_cast<float>(screen_size.y));

			if (character == '\n')
			{
				// Simulate a newline
				char_origin_y += screen_size_ratio;
				char_origin_x = 0.f;
			}
			else
			{
				// Get character info
				const Rendering::FontChar* char_data = nullptr;

				auto char_data_opt = font->getChar(character);
				if (char_data_opt.has_value()) { char_data = char_data_opt.value(); }
				else
				{
					/**
					 * @todo Handle characters not in the font
					 */
					throw ENGINE_EXCEPTION(std::format(
						"Tried to access font character outside of range (invalid "
						"character "
						"'{}')",
						character
					));
				}

				// Check if font contains this character
				if (char_data == nullptr)
				{
					logger->simpleLog<void>(
						Logging::LogLevel::Error,
						"Char 0x%x is not found in set font",
						character
					);
					continue;
				}

				// Get texture information
				ImageSize texture_size = font->getPageTexture(char_data->page)->getSize();

				assert(texture_size.x > 0 && texture_size.y > 0);

				// Character parameters as specified in .fnt file
				const auto char_x	   = static_cast<float>(char_data->x);
				const auto char_y	   = static_cast<float>(char_data->y);
				const auto char_width  = static_cast<float>(char_data->width);
				const auto char_height = static_cast<float>(char_data->height);

				// Offset origin by xoffset (specified in .fnt file) of this char
				const float position_x =
					char_origin_x +
					((static_cast<float>(char_data->xoffset) * font_size_ratio) /
					 static_cast<float>(screen_size.x));
				const float position_y = char_origin_y;
				const float position_z = 0.0f;

				// position_x is already set so we can
				// move origin to the next character using xadvance from font
				char_origin_x +=
					(static_cast<float>(char_data->xadvance) * font_size_ratio) /
					static_cast<float>(screen_size.x);

				// If current character is space we can skip generating a quad for it
				if (character == ' ') { continue; }

				// Convert character size to screen-space coordinates
				// in renderer, multiply with selected size
				//
				// Final size equation: ((character_width_px / font_size_px) /
				// screen_width_ss) * selected_size_px
				//
				const float char_width_ratio = (char_width / static_cast<float>(font_size));
				const float char_height_ratio =
					(char_height / static_cast<float>(font_size));
				const float size_x = (char_width_ratio / static_cast<float>(screen_size.x));
				const float size_y =
					(char_height_ratio / static_cast<float>(screen_size.y));

				// Color data
				const float color_r = 1.0f;
				const float color_g = 1.0f;
				const float color_b = 1.0f;

				const std::vector<RenderingVertex> vertices = {
					{
						glm::vec3(position_x, position_y + size_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x) / static_cast<float>(texture_size.x),
							(char_y + char_height) / static_cast<float>(texture_size.y)
						),
					},
					{
						glm::vec3(position_x + size_x, position_y + size_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x + char_width) / static_cast<float>(texture_size.x),
							(char_y + char_height) / static_cast<float>(texture_size.y)
						),
					},
					{
						glm::vec3(position_x, position_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x) / static_cast<float>(texture_size.x),
							(char_y) / static_cast<float>(texture_size.y)
						),
					},
					{
						glm::vec3(position_x + size_x, position_y + size_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x + char_width) / static_cast<float>(texture_size.x),
							(char_y + char_height) / static_cast<float>(texture_size.y)
						),
					},
					{
						glm::vec3(position_x + size_x, position_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x + char_width) / static_cast<float>(texture_size.x),
							(char_y) / static_cast<float>(texture_size.y)
						),
					},
					{
						glm::vec3(position_x, position_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x) / static_cast<float>(texture_size.x),
							(char_y) / static_cast<float>(texture_size.y)
						),
					}
				};

				generated_mesh
					.insert(generated_mesh.end(), vertices.begin(), vertices.end());
			}
		}

		// Create context
		std::copy(generated_mesh.begin(), generated_mesh.end(), std::back_inserter(mesh));

		context = MeshBuildContext();
		context.rebuildVBO(instance, mesh);
		needs_rebuild = false;
	}
} // namespace Engine::Rendering
