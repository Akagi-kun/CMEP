#include "Rendering/TextMeshBuilder.hpp"

#include "Assets/Font.hpp"
#include "Assets/Texture.hpp"
#include "Rendering/Vulkan/VulkanStructDefs.hpp"

#include <cstdint>
#include <iterator>

namespace Engine::Rendering
{
	void TextMeshBuilder::SupplyData(const RendererSupplyData& data)
	{
		switch (data.type)
		{
			case RendererSupplyDataType::FONT:
			{
				this->font = std::static_pointer_cast<Font>(data.payload_ptr);
				break;
			}
			case RendererSupplyDataType::TEXT:
			{
				this->text.assign(data.payload_string);
				break;
			}
			default:
			{
				break;
			}
		}
	}

	void TextMeshBuilder::Build()
	{
		if (this->context.vbo != nullptr)
		{
			this->renderer->SyncDeviceWaitIdle();

			delete this->context.vbo;

			this->context.vbo = nullptr;
			this->context.mesh.clear();
		}
		this->context.been_rebuilt = true;

		auto window_data  = this->renderer->GetWindow();
		this->screen_size = {
			static_cast<uint_fast16_t>(window_data.window_x),
			static_cast<uint_fast16_t>(window_data.window_y)
		};

		int font_size = std::stoi(*this->font->GetFontInfoParameter("size"));

		std::vector<RenderingVertex> generated_mesh;

		// Positions that denote the origin for a specific character
		// Moved as the mesh is built
		float char_origin_x = 0.f;
		float char_origin_y = 0.f;

		// Iteratively generate a quad for each character based on font information
		for (char text_char : this->text)
		{
			// Following calculations assume that font sizes are specified in pixels
			// Selected font size is assumed to be 1px in all calculations
			//
			// Ratio between 1px and font size (as defined by the .fnt file)
			// (as font size increases, this value decreases)
			const auto font_size_ratio = (1.0f / static_cast<float>(font_size));

			// Ratio between 1px and the screen size
			// (as screen size increases, this value decreases)
			const auto screen_size_ratio = (1.0f / static_cast<float>(this->screen_size.y));

			if (text_char == '\n')
			{
				// Simulate a newline
				char_origin_y += screen_size_ratio;
				char_origin_x = 0.f;
			}
			else
			{
				// Get character info
				Rendering::FontChar* chardata = this->font->GetChar(text_char);

				// Check if font contains this character
				if (chardata == nullptr)
				{
					this->logger->SimpleLog(Logging::LogLevel::Error, "Char 0x%x is not found in set font", text_char);
					continue;
				}

				// Get texture information
				TextureSize texture_size;
				std::shared_ptr<Texture> texture = this->font->GetPageTexture(chardata->page);
				assert(texture != nullptr);
				texture->GetSize(texture_size);
				assert(texture_size.x > 0 && texture_size.y > 0);

				// Character parameters as specified in .fnt file
				const auto char_x	   = static_cast<float>(chardata->x);
				const auto char_y	   = static_cast<float>(chardata->y);
				const auto char_width  = static_cast<float>(chardata->width);
				const auto char_height = static_cast<float>(chardata->height);

				// Offset origin by xoffset (specified in .fnt file) of this char
				const float position_x = char_origin_x + ((static_cast<float>(chardata->xoffset) * font_size_ratio) /
														  static_cast<float>(this->screen_size.x));
				const float position_y = char_origin_y;
				const float position_z = 0.0f;

				// position_x is already set so we can
				// move origin to the next character using xadvance from font
				char_origin_x += (static_cast<float>(chardata->xadvance) * font_size_ratio) /
								 static_cast<float>(this->screen_size.x);

				// If current character is space we can skip generating a mesh for it
				if (text_char == ' ')
				{
					continue;
				}

				// Convert character size to screen-space coordinates
				// in renderer, multiply with selected size
				//
				// Final size equation: ((character_width_px / font_size_px) / screen_width_ss) * selected_size_px
				//
				const float char_width_ratio  = (char_width / static_cast<float>(font_size));
				const float char_height_ratio = (char_height / static_cast<float>(font_size));
				const float size_x			  = (char_width_ratio / static_cast<float>(this->screen_size.x));
				const float size_y			  = (char_height_ratio / static_cast<float>(this->screen_size.y));

				// Color data
				const float color_r = 1.0f;
				const float color_g = 1.0f;
				const float color_b = 1.0f;

				const std::vector<RenderingVertex> vertices = {
					RenderingVertex{
						glm::vec3(position_x, position_y + size_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x) / static_cast<float>(texture_size.x),
							(char_y + char_height) / static_cast<float>(texture_size.y)
						)
					},
					RenderingVertex{
						glm::vec3(position_x + size_x, position_y + size_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x + char_width) / static_cast<float>(texture_size.x),
							(char_y + char_height) / static_cast<float>(texture_size.y)
						)
					},
					RenderingVertex{
						glm::vec3(position_x, position_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x) / static_cast<float>(texture_size.x),
							(char_y) / static_cast<float>(texture_size.y)
						)
					},
					RenderingVertex{
						glm::vec3(position_x + size_x, position_y + size_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x + char_width) / static_cast<float>(texture_size.x),
							(char_y + char_height) / static_cast<float>(texture_size.y)
						)
					},
					RenderingVertex{
						glm::vec3(position_x + size_x, position_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x + char_width) / static_cast<float>(texture_size.x),
							(char_y) / static_cast<float>(texture_size.y)
						)
					},
					RenderingVertex{
						glm::vec3(position_x, position_y, position_z),
						glm::vec3(color_r, color_g, color_b),
						glm::vec2(
							(char_x) / static_cast<float>(texture_size.x),
							(char_y) / static_cast<float>(texture_size.y)
						)
					}
				};

				generated_mesh.insert(generated_mesh.end(), vertices.begin(), vertices.end());

				this->texture_image = texture->GetTextureImage();
			}
		}

		// Copy into context
		this->context.mesh.reserve(generated_mesh.size());
		std::copy(generated_mesh.begin(), generated_mesh.end(), std::back_inserter(this->context.mesh));

		this->context.vbo			 = this->renderer->CreateVulkanVertexBufferFromData(generated_mesh);
		this->context.vbo_vert_count = generated_mesh.size();
	}
} // namespace Engine::Rendering
