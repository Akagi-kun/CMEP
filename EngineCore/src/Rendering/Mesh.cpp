#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float2.hpp>

#include "Logging/Logging.hpp"
#include "Rendering/Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "Rendering/tinyobjloader/tiny_obj_loader.h"

namespace Engine::Rendering
{
	void Mesh::CreateMeshFromObj(std::string path)
	{
		tinyobj::ObjReaderConfig reader_config;
		reader_config.mtl_search_path = "./data/models/"; // Path to material files

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(path, reader_config)) {
			if (!reader.Error().empty()) {
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "TinyObjReader: %s", reader.Error().c_str());
			}
			exit(1);
		}

		if (!reader.Warning().empty()) {
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Warning, "TinyObjReader: %s", reader.Warning().c_str());
		}

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		int diffuse_count = 0;
		int bump_count = 0;
		int roughness_count = 0;
		int metallic_count = 0;
		int reflection_count = 0;
		
		for (size_t i = 0; i < materials.size(); i++)
		{

			tinyobj::material_t material = materials[i];
			if (material.diffuse_texname != "")
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "TinyObjReader: Diffuse texture: '%s' for material: '%s'", material.diffuse_texname.c_str(), material.name.c_str());
				Rendering::Texture textureDiffuse = Rendering::Texture();
				textureDiffuse.InitFile(Rendering::Texture_InitFiletype::FILE_PNG, material.diffuse_texname);
				this->diffuse_textures.push_back(std::make_shared<Rendering::Texture>(textureDiffuse));
				diffuse_count++;
			}
			else
			{
				this->diffuse_textures.push_back(nullptr);
			}

			if (material.bump_texname != "")
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "TinyObjReader: Bump texture: '%s' for material: '%s'", material.bump_texname.c_str(), material.name.c_str());
				Rendering::Texture textureBump = Rendering::Texture();
				textureBump.InitFile(Rendering::Texture_InitFiletype::FILE_PNG, material.bump_texname);
				this->bump_textures.push_back(std::make_shared<Rendering::Texture>(textureBump));
				bump_count++;
			}
			else
			{
				this->bump_textures.push_back(nullptr);
			}

			if (material.roughness_texname != "")
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "TinyObjReader: Roughness texture: '%s' for material: '%s'", material.roughness_texname.c_str(), material.name.c_str());
				Rendering::Texture textureRoughness = Rendering::Texture();
				textureRoughness.InitFile(Rendering::Texture_InitFiletype::FILE_PNG, material.roughness_texname);
				this->roughness_textures.push_back(std::make_shared<Rendering::Texture>(textureRoughness));
				roughness_count++;
			}
			else
			{
				this->roughness_textures.push_back(nullptr);
			}

			if (material.metallic_texname != "")
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "TinyObjReader: Metallic texture: '%s' for material: '%s'", material.metallic_texname.c_str(), material.name.c_str());
				Rendering::Texture textureMetallic = Rendering::Texture();
				textureMetallic.InitFile(Rendering::Texture_InitFiletype::FILE_PNG, material.metallic_texname);
				this->metallic_textures.push_back(std::make_shared<Rendering::Texture>(textureMetallic));
				metallic_count++;
			}
			else
			{
				this->metallic_textures.push_back(nullptr);
			}

			if (material.reflection_texname != "")
			{
				Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Debug2, "TinyObjReader: Reflection texture: '%s' for material: '%s'", material.reflection_texname.c_str(), material.name.c_str());
				Rendering::Texture textureReflection = Rendering::Texture();
				textureReflection.InitFile(Rendering::Texture_InitFiletype::FILE_PNG, material.reflection_texname);
				this->reflective_textures.push_back(std::make_shared<Rendering::Texture>(textureReflection));
				reflection_count++;
			}
			else
			{
				this->reflective_textures.push_back(nullptr);
			}
		}

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "\nTinyObjReader:\n  Loaded %u materials\n  %u diffuse textures\n  %u bump textures\n  %u roughness textures\n  %u metallic textures\n  %u reflective textures",
			materials.size(), diffuse_count, bump_count, roughness_count, metallic_count, reflection_count);

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++)
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
				
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
					this->mesh_vertices.push_back(glm::vec3(vx, vy, vz));

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0)
					{
						tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
						tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
						tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
						this->mesh_normals.push_back(glm::vec3(nx, ny, nz));
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0)
					{
						tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
						tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
						this->mesh_uvs.push_back(glm::vec2(tx, ty));
					}

					// per-vertex material id
					this->matids.push_back(shapes[s].mesh.material_ids[f]);

					// per-vertex material data
					if (materials.size() > 0)
					{
						const tinyobj::real_t* face_ambient = materials[shapes[s].mesh.material_ids[f]].ambient;
						const tinyobj::real_t* face_diffuse = materials[shapes[s].mesh.material_ids[f]].diffuse;
						const tinyobj::real_t* face_specular = materials[shapes[s].mesh.material_ids[f]].specular;
						const tinyobj::real_t face_dissolve = materials[shapes[s].mesh.material_ids[f]].dissolve;
						const tinyobj::real_t* face_emission = materials[shapes[s].mesh.material_ids[f]].emission;

						this->mesh_ambient.push_back(glm::vec3(face_ambient[0], face_ambient[1], face_ambient[2]));
						this->mesh_diffuse.push_back(glm::vec3(face_diffuse[0], face_diffuse[1], face_diffuse[2]));
						this->mesh_specular.push_back(glm::vec3(face_specular[0], face_specular[1], face_specular[2]));
						this->mesh_dissolve.push_back(face_dissolve);
						this->mesh_emission.push_back(glm::vec3(face_emission[0], face_emission[1], face_emission[2]));
					}
				}
				index_offset += fv;

			}

		}
		
		for (size_t idx = 0; idx < this->mesh_vertices.size(); idx += 3)
		{
			// Shortcuts for vertices
			glm::vec3& v0 = this->mesh_vertices[idx + 0];
			glm::vec3& v1 = this->mesh_vertices[idx + 1];
			glm::vec3& v2 = this->mesh_vertices[idx + 2];

			// Shortcuts for UVs
			glm::vec2& uv0 = this->mesh_uvs[idx + 0];
			glm::vec2& uv1 = this->mesh_uvs[idx + 1];
			glm::vec2& uv2 = this->mesh_uvs[idx + 2];

			// Edges of the triangle : position delta
			glm::vec3 deltaPos1 = v1 - v0;
			glm::vec3 deltaPos2 = v2 - v0;

			// UV delta
			glm::vec2 deltaUV1 = uv1 - uv0;
			glm::vec2 deltaUV2 = uv2 - uv0;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
			glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

			this->mesh_tangents.push_back(tangent);
			this->mesh_tangents.push_back(tangent);
			this->mesh_tangents.push_back(tangent);

			// Same thing for bitangents
			this->mesh_bitangents.push_back(bitangent);
			this->mesh_bitangents.push_back(bitangent);
			this->mesh_bitangents.push_back(bitangent);
		}

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "Reading OBJ file: Successfully read and parsed file '%s", path.c_str());
	}
}