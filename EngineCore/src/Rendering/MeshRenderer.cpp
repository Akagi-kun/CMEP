#include <assert.h>
#include <sstream>
#include <fstream>

#include "Rendering/MeshRenderer.hpp"
#include "GlobalSceneManager.hpp"
#include "Rendering/Texture.hpp"
#include <glm/gtx/quaternion.hpp>
#include "Rendering/Shader.hpp"
#include "Logging/Logging.hpp"

#include "Rendering/GLCommon.hpp"

namespace Engine::Rendering
{
	MeshRenderer::MeshRenderer()
	{
		std::ifstream vert("data/shaders/meshrenderer.vert");
		std::ifstream frag("data/shaders/meshrenderer.frag");
		std::ostringstream vertsstr;
		std::ostringstream fragsstr;
		vertsstr << vert.rdbuf();
		fragsstr << frag.rdbuf();

		this->program = std::make_unique<Shader>(vertsstr.str().c_str(), fragsstr.str().c_str());

		this->matMVP = glm::mat4();
	}

	MeshRenderer::~MeshRenderer()
	{
		glDeleteVertexArrays(1, &this->vao);
		glDeleteBuffers(1, &this->vbo);
		glDeleteBuffers(1, &this->mbo);
	}

	void MeshRenderer::AssignMesh(Mesh& new_mesh)
	{
		this->mesh = std::make_unique<Mesh>(new_mesh);

		this->has_updated_mesh = false;
		this->has_updated_meshdata = false;
	}

	void MeshRenderer::UpdateTexture(const Rendering::Texture* texture) noexcept
	{
		this->texture.reset(texture);

		this->has_updated_mesh = false;
	}

	void MeshRenderer::Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept
	{
		this->_pos = pos;
		this->_size = size;
		this->_rotation = rotation;

		this->_screenx = screenx;
		this->_screeny = screeny;

		this->has_updated_mesh = false;
	}

	void MeshRenderer::UpdateMesh() noexcept
	{
		if (!this->mesh)
		{
			return;
		}

		this->has_updated_mesh = true;

		// Create VBO and VAO if they arent created already
		if (this->vbo == 0)
		{
			glCreateBuffers(1, &this->vbo);
		}
		if (this->vao == 0)
		{
			glCreateVertexArrays(1, &this->vao);
		}
		if (this->mbo == 0)
		{
			glCreateBuffers(1, &this->mbo);
		}
		if (this->tbbo == 0)
		{
			glCreateBuffers(1, &this->tbbo);
		}


		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)this->_screenx / this->_screeny, 0.1f, 100.0f);
		glm::mat4 View = global_scene_manager->GetCameraViewMatrix();

		glm::quat ModelRotation = glm::quat(glm::radians(this->_rotation));
		glm::mat4 Model = glm::translate(glm::mat4(1.0f), this->_pos) * glm::mat4(1.0f) * glm::toMat4(ModelRotation);

		this->matM = Model;
		this->matV = View;
		this->matMV = View * Model;
		this->matMV3x3 = glm::mat3(View * Model);
		this->matMVP = Projection * View * Model;

		if (!this->has_updated_meshdata)
		{
			this->has_updated_meshdata = true;
		
			// Vertices, UVs and Normals
			glNamedBufferData(this->vbo,
				this->mesh->mesh_vertices.size() * sizeof(glm::vec3) + this->mesh->mesh_uvs.size() * sizeof(glm::vec2) + this->mesh->mesh_normals.size() * sizeof(glm::vec3),
				NULL, GL_STATIC_DRAW
			);

			glNamedBufferSubData(this->vbo, 
				0,
				this->mesh->mesh_vertices.size() * sizeof(glm::vec3),
				(void*)&(this->mesh->mesh_vertices[0])
			);

			glNamedBufferSubData(this->vbo,
				this->mesh->mesh_vertices.size() * sizeof(glm::vec3),
				this->mesh->mesh_uvs.size() * sizeof(glm::vec2),
				(void*)&(this->mesh->mesh_uvs[0])
			);

			glNamedBufferSubData(this->vbo,
				this->mesh->mesh_vertices.size() * sizeof(glm::vec3) + this->mesh->mesh_uvs.size() * sizeof(glm::vec2),
				this->mesh->mesh_normals.size() * sizeof(glm::vec3),
				(void*)&(this->mesh->mesh_normals[0])
			);

			// Materials
			glNamedBufferData(this->mbo,
				this->mesh->mesh_ambient.size() * sizeof(glm::vec3) +
					this->mesh->mesh_diffuse.size() * sizeof(glm::vec3) +
					this->mesh->mesh_specular.size() * sizeof(glm::vec3) +
					this->mesh->matids.size() * sizeof(GLuint) +
					this->mesh->mesh_dissolve.size() * sizeof(float),
				NULL, GL_STATIC_DRAW
			);

			glNamedBufferSubData(this->mbo,
				0,
				this->mesh->mesh_ambient.size() * sizeof(glm::vec3),
				(void*)&(this->mesh->mesh_ambient[0])
			);

			glNamedBufferSubData(this->mbo,
				this->mesh->mesh_ambient.size() * sizeof(glm::vec3),
				this->mesh->mesh_diffuse.size() * sizeof(glm::vec3),
				(void*)&(this->mesh->mesh_diffuse[0])
			);

			glNamedBufferSubData(this->mbo,
				this->mesh->mesh_ambient.size() * sizeof(glm::vec3) + this->mesh->mesh_diffuse.size() * sizeof(glm::vec3),
				this->mesh->mesh_specular.size() * sizeof(glm::vec3),
				(void*)&(this->mesh->mesh_specular[0])
			);

			glNamedBufferSubData(this->mbo,
				this->mesh->mesh_ambient.size() * sizeof(glm::vec3) + this->mesh->mesh_diffuse.size() * sizeof(glm::vec3) + this->mesh->mesh_specular.size() * sizeof(glm::vec3),
				this->mesh->matids.size() * sizeof(GLuint),
				(void*)&(this->mesh->matids[0])
			);
			
			glNamedBufferSubData(this->mbo,
				this->mesh->mesh_ambient.size() * sizeof(glm::vec3) + this->mesh->mesh_diffuse.size() * sizeof(glm::vec3) + this->mesh->mesh_specular.size() * sizeof(glm::vec3)+ this->mesh->matids.size() * sizeof(GLuint),
				this->mesh->mesh_dissolve.size() * sizeof(float),
				(void*)&(this->mesh->mesh_dissolve[0])
			);

			// Tangents and bitangents
			glNamedBufferData(this->tbbo,
				this->mesh->mesh_tangents.size() * sizeof(glm::vec3) +
				this->mesh->mesh_bitangents.size() * sizeof(glm::vec3),
				NULL, GL_STATIC_DRAW
			);

			glNamedBufferSubData(this->tbbo,
				0,
				this->mesh->mesh_tangents.size() * sizeof(glm::vec3),
				(void*)&(this->mesh->mesh_tangents[0])
			);

			glNamedBufferSubData(this->tbbo,
				this->mesh->mesh_tangents.size() * sizeof(glm::vec3),
				this->mesh->mesh_bitangents.size() * sizeof(glm::vec3),
				(void*)&(this->mesh->mesh_bitangents[0])
			);

			//Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "%u %u %u %u %u", this->mesh->mesh_vertices.size(), this->mesh->mesh_diffuse.size(), this->mesh->mesh_specular.size(), this->mesh->matids.size(), this->mesh->mesh_dissolve.size());
		}
	}

	void MeshRenderer::Render()
	{
		if (!this->has_updated_mesh)
		{
			this->UpdateMesh();
		}

		// If no shader bound, throw exception
		if (!this->program)
		{
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Exception, "No program assigned to MeshRenderer, cannot perform Engine::Rendering::MeshRenderer::Render()");
			exit(1);
		}

		glBindVertexArray(this->vao);

		// Bind shader
		GLuint shader = this->program->GetProgram();

		if (shader == 0)
		{
			throw std::exception("MeshRenderer: Shader::GetProgram() returned zero! Bad shader!");
		}

		glUseProgram(shader);

		GLint maxTextures;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);

		// Bind diffuse textures
		unsigned int textureoffset = 0;
		for (unsigned int offset = 0; offset < this->mesh->diffuse_textures.size(); offset++)
		{
			if (this->mesh->diffuse_textures[offset] != nullptr)
			{
				std::shared_ptr<Rendering::Texture> texture = this->mesh->diffuse_textures[offset];

				GLuint textureLocation = glGetUniformLocation(shader, std::string("textureDiffuse").append(std::to_string(offset)).c_str());

				glUniform1i(textureLocation, offset);
				glActiveTexture(GL_TEXTURE0 + textureoffset);
				glBindTexture(GL_TEXTURE_2D, texture->GetTexture());

				GLuint textureUsedLocation = glGetUniformLocation(shader, std::string("used_textureDiffuse").append(std::to_string(offset)).c_str());
				glUniform1i(textureUsedLocation, 1);

				if (textureoffset > maxTextures)
				{
					throw std::exception("MeshRenderer: Texture limit reached! Too many textures!");
				}
			}
			else
			{
				glActiveTexture(GL_TEXTURE0 + textureoffset);
				glBindTexture(GL_TEXTURE_2D, 0);

				GLuint textureUsedLocation = glGetUniformLocation(shader, std::string("used_textureDiffuse").append(std::to_string(offset)).c_str());
				glUniform1i(textureUsedLocation, 0);
			}
			textureoffset++;
		}

		for (unsigned int offset = 0; offset < this->mesh->bump_textures.size(); offset++)
		{
			if (this->mesh->bump_textures[offset] != nullptr)
			{
				std::shared_ptr<Rendering::Texture> texture = this->mesh->bump_textures[offset];

				GLuint textureLocation = glGetUniformLocation(shader, std::string("textureNormal").append(std::to_string(offset)).c_str());

				glUniform1i(textureLocation, offset);
				glActiveTexture(GL_TEXTURE0 + textureoffset);
				glBindTexture(GL_TEXTURE_2D, texture->GetTexture());

				GLuint textureUsedLocation = glGetUniformLocation(shader, std::string("used_textureNormal").append(std::to_string(offset)).c_str());
				glUniform1i(textureUsedLocation, 1);

				if (textureoffset > maxTextures)
				{
					throw std::exception("MeshRenderer: Texture limit reached! Too many textures!");
				}
			}
			else
			{
				glActiveTexture(GL_TEXTURE0 + textureoffset);
				glBindTexture(GL_TEXTURE_2D, 0);

				GLuint textureUsedLocation = glGetUniformLocation(shader, std::string("used_textureNormal").append(std::to_string(offset)).c_str());
				glUniform1i(textureUsedLocation, 0);
			}
			textureoffset++;
		}

		// Bind vbo and vertex data
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr); // vertices
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(this->mesh->mesh_vertices.size() * sizeof(glm::vec3))); // uvs
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(this->mesh->mesh_vertices.size() * sizeof(glm::vec3) + this->mesh->mesh_uvs.size() * sizeof(glm::vec2))); // normals

		// Bind mbo and material data
		glBindBuffer(GL_ARRAY_BUFFER, this->mbo);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr); // material ambient
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)(this->mesh->mesh_ambient.size() * sizeof(glm::vec3))); // material diffuse
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, (void*)(this->mesh->mesh_ambient.size() * sizeof(glm::vec3) + this->mesh->mesh_diffuse.size() * sizeof(glm::vec3))); // material specular
		glVertexAttribIPointer(6, 1, GL_UNSIGNED_INT, 0, (void*)(this->mesh->mesh_ambient.size() * sizeof(glm::vec3) + this->mesh->mesh_diffuse.size() * sizeof(glm::vec3) + this->mesh->mesh_specular.size() * sizeof(glm::vec3))); // material ids
		glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, 0, (void*)(this->mesh->mesh_ambient.size() * sizeof(glm::vec3) + this->mesh->mesh_diffuse.size() * sizeof(glm::vec3) + this->mesh->mesh_specular.size() * sizeof(glm::vec3) + this->mesh->matids.size() * sizeof(GLuint))); // material dissolve

		// Bind tbbo and tangent/bitangent data
		glBindBuffer(GL_ARRAY_BUFFER, this->tbbo);
		glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 0, nullptr); // material ambient
		glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, 0, (void*)(this->mesh->mesh_tangents.size() * sizeof(glm::vec3))); // material diffuse

		// Bind matrixes
		GLuint m_uniform_id = glGetUniformLocation(shader, "matM");
		glUniformMatrix4fv(m_uniform_id, 1, GL_FALSE, &this->matM[0][0]);

		GLuint v_uniform_id = glGetUniformLocation(shader, "matV");
		glUniformMatrix4fv(v_uniform_id, 1, GL_FALSE, &this->matV[0][0]);

		GLuint mv_uniform_id = glGetUniformLocation(shader, "matMV");
		glUniformMatrix4fv(mv_uniform_id, 1, GL_FALSE, &this->matMV[0][0]);

		GLuint mv3x3_uniform_id = glGetUniformLocation(shader, "matMV3x3");
		glUniformMatrix3fv(mv3x3_uniform_id, 1, GL_FALSE, &this->matMV3x3[0][0]);

		GLuint mvp_uniform_id = glGetUniformLocation(shader, "matMVP");
		glUniformMatrix4fv(mvp_uniform_id, 1, GL_FALSE, &this->matMVP[0][0]);

		GLuint light_uniform_id = glGetUniformLocation(shader, "Light");
		glm::vec3 light_position = global_scene_manager->GetLightTransform();
		glUniform3f(light_uniform_id, light_position.x, light_position.y, light_position.z);

		// Enable vertex attribs
		for (int i = 0; i < 8; i++)
		{
			glEnableVertexAttribArray(i);
		}

		glDrawArrays(GL_TRIANGLES, 0, GLsizei(this->mesh->mesh_vertices.size()));


		// Disable vertex attribs
		for (int i = 0; i < 8; i++)
		{
			glDisableVertexAttribArray(i);
		}

		// Unbind textures
		for (int i = 0; i < maxTextures; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}