#pragma once

#include <vector>
#include <string>

#include "Rendering/tinyobjloader/tiny_obj_loader.h"
#include "Rendering/Texture.hpp"
#include "PlatformSemantics.hpp"
#include "InternalEngineObject.hpp"

namespace Engine::Rendering
{
	class CMEP_EXPORT Mesh final : public InternalEngineObject
	{
	private:
	public:
		std::vector<glm::vec3> mesh_vertices;
		std::vector<glm::vec2> mesh_uvs;
		std::vector<glm::vec3> mesh_normals;

		std::vector<glm::vec3> mesh_tangents;
		std::vector<glm::vec3> mesh_bitangents;
		
		std::vector<unsigned int> matids;
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

		Mesh();
		~Mesh();

		void CreateMeshFromObj(std::string path);
	};
}