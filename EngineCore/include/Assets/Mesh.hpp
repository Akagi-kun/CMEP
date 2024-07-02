#pragma once

#include "Assets/Texture.hpp"
#include "Rendering/tinyobjloader/tiny_obj_loader.h"

#include "InternalEngineObject.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace Engine::Rendering
{
	class Mesh final : public InternalEngineObject
	{
	private:
	public:
		std::vector<glm::vec3> mesh_vertices;
		std::vector<glm::vec2> mesh_uvs;
		std::vector<glm::vec3> mesh_normals;

		std::vector<glm::vec3> mesh_tangents;
		std::vector<glm::vec3> mesh_bitangents;

		std::vector<int_fast16_t> matids;
		std::vector<tinyobj::material_t> materials;
		std::vector<glm::vec3> mesh_ambient;
		std::vector<glm::vec3> mesh_diffuse;
		std::vector<glm::vec3> mesh_specular;
		std::vector<float> mesh_dissolve;
		std::vector<glm::vec3> mesh_emission;

		std::vector<std::shared_ptr<Rendering::Texture>> diffuse_textures;
		// std::vector<std::shared_ptr<Rendering::Texture>> bump_textures;
		// std::vector<std::shared_ptr<Rendering::Texture>> roughness_textures;
		// std::vector<std::shared_ptr<Rendering::Texture>> reflective_textures;

		// public:
		using InternalEngineObject::InternalEngineObject;
		// Mesh();
		~Mesh();

		void CreateMeshFromObj(std::string path);
	};
} // namespace Engine::Rendering
