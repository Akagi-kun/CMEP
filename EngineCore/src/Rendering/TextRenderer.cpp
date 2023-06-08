#include <assert.h>
#include <vector>
#include <array>


#include "Rendering/TextRenderer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/Font.hpp"
#include "Logging/Logging.hpp"

#include "Rendering/GLCommon.hpp"

namespace Engine::Rendering
{
	TextRenderer::TextRenderer()
	{
		static const char* vertex_shader_source =
			"#version 400 core\n"
			"layout(location = 0) in vec3 pos; layout(location = 1) in vec2 texCord; out vec2 fragTexCord;"
			"void main() { gl_Position = vec4(pos, 1.0f); fragTexCord = texCord; }";

		static const char* fragment_shader_source =
			"#version 400 core\n"
			"out vec4 color; in vec2 fragTexCord; uniform sampler2D texture0;"
			"void main() { color = texture(texture0, fragTexCord); }";

		this->program = std::make_unique<Rendering::Shader>(vertex_shader_source, fragment_shader_source);
	}

	TextRenderer::~TextRenderer()
	{
		glDeleteVertexArrays(1, &this->vao);
		glDeleteBuffers(1, &this->vbo);
	}

	void TextRenderer::Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept
	{
		this->_pos = pos;
		this->_size = size;
		this->_rotation = rotation;

		this->_screenx = screenx;
		this->_screeny = screeny;

		this->has_updated_mesh = false;
	}

	int TextRenderer::UpdateFont(Rendering::Font* const font) noexcept
	{
		this->font.reset(font);

		this->has_updated_mesh = false;

		return 0;

	}

	int TextRenderer::UpdateText(const std::string text) noexcept
	{
		this->text.assign(text);
		
		this->has_updated_mesh = false;

		return 0;
	}

	void TextRenderer::UpdateMesh()
	{
		this->has_updated_mesh = true;

		if (this->vao == 0)
		{
			glCreateVertexArrays(1, &this->vao);
		}

		if (this->vbo == 0)
		{
			glCreateBuffers(1, &this->vbo);
		}

		int fontsize = std::stoi(this->font.get()->GetFontInfoParameter("size")->c_str(), nullptr, 10);

		std::vector<GLfloat> generated_mesh = {};

		unsigned int vbo_ = 0;
		float accu_x = 0.f;
		float accu_y = 0.f;
		for (int i = 0; i < this->text.size(); i++)
		{
			vbo_ = 0;
			if (this->text[i] == '\n')
			{
				accu_y -= 0.35f * (float)(std::round(_size.y) / fontsize);
				accu_x = 0.f;
			}
			else if (this->text[i] == ' ')
			{
				accu_x += 0.05f * (float)(std::round(_size.x) / fontsize);
			}
			else
			{
				// Get character info
				Rendering::FontChar* ch = this->font->GetChar(this->text[i]);
				assert(ch != nullptr);
				if (ch == nullptr)
				{
					assert(false);
					continue;
				}

				// Get texture
				unsigned int texture_x = 0, texture_y = 0;
				const Rendering::Texture* const texture = this->font->GetPageTexture(ch->page);
				assert(texture != nullptr);
				texture->GetSize(texture_x, texture_y);
				assert(texture_x > 0 && texture_y > 0);

				// Obscure math even I don't understand, achieved with trial and error and works so just leave it like this
				const float xs = ch->width / (float)this->_screenx * 2 * (float)(std::round(_size.x) / fontsize);
				const float ys = ch->height / (float)this->_screeny * 2 * (float)(std::round(_size.y) / fontsize);
				const float x = (float)this->_pos.x * 2 - 1.f + accu_x;
				const float y = (float)this->_pos.y * 2 - 1.f + accu_y;

				std::array<GLfloat, 30> data = {
					x, ys + y, 0.f, /**/ (ch->x) / (float)texture_x, (ch->y) / (float)texture_y,
					xs + x, ys + y, 0.f, /**/ (ch->x + ch->width) / (float)texture_x, (ch->y) / (float)texture_y,
					x, y, 0.f, /**/ (ch->x) / (float)texture_x, (ch->y + ch->height) / (float)texture_y,

					xs + x, ys + y, 0.f, /**/ (ch->x + ch->width) / (float)texture_x, (ch->y) / (float)texture_y,
					xs + x, y, 0.f, /**/ (ch->x + ch->width) / (float)texture_x, (ch->y + ch->height) / (float)texture_y,
					x, y, 0.f, /**/ (ch->x) / (float)texture_x, (ch->y + ch->height) / (float)texture_y
				};

				accu_x += xs + (6.f / this->_screenx);

				generated_mesh.insert(generated_mesh.end(), data.begin(), data.end());
			}
		}

		assert(generated_mesh.size() > 0);
		glNamedBufferData(this->vbo, generated_mesh.size() * sizeof(GLfloat), (void*)generated_mesh.data(), GL_STATIC_DRAW);
		this->vbo_vert_count = generated_mesh.size() / 5;
	}

	void TextRenderer::Render()
	{
		if (this->text.size() == 0)
		{
			return;
		}

		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		if (!this->program)
		{
			throw std::exception("No program assigned to TextRenderer, cannot perform Engine::Rendering::TextRenderer::Render()");
		}

		GLuint shader = this->program->GetProgram();
		assert(shader != 0);

		glBindVertexArray(this->vao);

		glUseProgram(shader);

		glActiveTexture(GL_TEXTURE0);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

		const Rendering::FontChar* const c = this->font->GetChar(this->text.at(0));
		assert(c != nullptr);
		GLuint texture = this->font->GetPageTexture(c->page)->GetTexture();
		assert(texture != 0);

		glBindTexture(GL_TEXTURE_2D, texture);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)nullptr);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

		glDrawArrays(GL_TRIANGLES, 0, GLsizei(this->vbo_vert_count));

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);;
	}
}