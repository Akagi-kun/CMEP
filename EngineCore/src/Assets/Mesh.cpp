#include "Assets/Mesh.hpp"

#include "Rendering/Vulkan/Wrappers/VBuffer.hpp"

#include "Logging/Logging.hpp"

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "Rendering/tinyobjloader/tiny_obj_loader.h"

#include "Engine.hpp"

namespace Engine::Rendering
{
	Mesh::~Mesh()
	{
		this->diffuse_textures.clear();

		this->logger->SimpleLog(Logging::LogLevel::Debug1, "Mesh destructor called");
	}

	void Mesh::CreateMeshFromObj(std::string path)
	{
		tinyobj::ObjReaderConfig reader_config;
		reader_config.mtl_search_path = "./game/models/"; // Path to material files

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(path, reader_config))
		{
			if (!reader.Error().empty())
			{
				this->logger->SimpleLog(Logging::LogLevel::Error, "TinyObjReader: %s", reader.Error().c_str());
			}
			exit(1);
		}

		if (!reader.Warning().empty())
		{
			this->logger->SimpleLog(Logging::LogLevel::Warning, "TinyObjReader: %s", reader.Warning().c_str());
		}

		const auto& attrib	  = reader.GetAttrib();
		const auto& shapes	  = reader.GetShapes();
		const auto& materials = reader.GetMaterials();

		int diffuse_count	 = 0;
		int bump_count		 = 0;
		int roughness_count	 = 0;
		int metallic_count	 = 0;
		int reflection_count = 0;

		Vulkan::VulkanRenderingEngine* renderer = this->owner_engine->GetRenderingEngine();

		auto* premade_staging_buffer = new Vulkan::VBuffer(
			renderer->GetDeviceManager(),
			1024 * 1024 * 4,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			0
		); // 1240x1024 4-channel staging buffer

		for (size_t i = 0; i < materials.size(); i++)
		{
			/* 			tinyobj::material_t material = materials[i];
						if (material.diffuse_texname != "")
						{
							// TODO: Fix this!

							this->logger->SimpleLog(Logging::LogLevel::Debug2, "TinyObjReader: Diffuse texture: '%s' for
			   material: '%s'", material.diffuse_texname.c_str(), material.name.c_str());
							std::shared_ptr<Rendering::Texture> textureDiffuse = std::make_shared<Rendering::Texture>();
							textureDiffuse->UsePremadeStagingBuffer(premade_staging_buffer);
							textureDiffuse->InitFile(Rendering::Texture_InitFiletype::FILE_PNG,
			   material.diffuse_texname); this->diffuse_textures.push_back(textureDiffuse); diffuse_count++;
						}
						else
						{
							this->diffuse_textures.push_back(nullptr);
						}
			 */
		}

		delete premade_staging_buffer;

		this->logger->SimpleLog(
			Logging::LogLevel::Info,
			"\nTinyObjReader:\n  Loaded %u materials\n  %u diffuse textures\n"
			"  %u bump textures\n  %u roughness textures\n  %u metallic textures\n  %u reflective textures",
			materials.size(),
			diffuse_count,
			bump_count,
			roughness_count,
			metallic_count,
			reflection_count
		);

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++)
		{
			const auto& current_shape_mesh = shapes[s].mesh;

			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < current_shape_mesh.num_face_vertices.size(); f++)
			{
				size_t fv = size_t(current_shape_mesh.num_face_vertices[f]);

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					// access to vertex
					tinyobj::index_t idx = current_shape_mesh.indices[index_offset + v];
					tinyobj::real_t vx	 = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					tinyobj::real_t vy	 = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					tinyobj::real_t vz	 = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
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
					const auto& material_id = current_shape_mesh.material_ids[f];
					this->matids.push_back(material_id);

					// per-vertex material data
					if (materials.size() > 0)
					{
						const auto& current_material = materials[static_cast<size_t>(material_id)];

						const tinyobj::real_t* face_ambient	 = current_material.ambient;
						const tinyobj::real_t* face_diffuse	 = current_material.diffuse;
						const tinyobj::real_t* face_specular = current_material.specular;
						const tinyobj::real_t face_dissolve	 = current_material.dissolve;
						const tinyobj::real_t* face_emission = current_material.emission;

						this->mesh_ambient.emplace_back(face_ambient[0], face_ambient[1], face_ambient[2]);
						this->mesh_diffuse.emplace_back(face_diffuse[0], face_diffuse[1], face_diffuse[2]);
						this->mesh_specular.emplace_back(face_specular[0], face_specular[1], face_specular[2]);
						this->mesh_dissolve.push_back(face_dissolve);
						this->mesh_emission.emplace_back(face_emission[0], face_emission[1], face_emission[2]);
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
			glm::vec3 delta_pos1 = v1 - v0;
			glm::vec3 delta_pos2 = v2 - v0;

			// UV delta
			glm::vec2 delta_uv_1 = uv1 - uv0;
			glm::vec2 delta_uv_2 = uv2 - uv0;

			float r				= 1.0f / (delta_uv_1.x * delta_uv_1.y - delta_uv_1.y * delta_uv_2.x);
			glm::vec3 tangent	= (delta_pos1 * delta_uv_2.y - delta_pos2 * delta_uv_1.y) * r;
			glm::vec3 bitangent = (delta_pos2 * delta_uv_1.x - delta_pos1 * delta_uv_2.x) * r;

			this->mesh_tangents.push_back(tangent);
			this->mesh_tangents.push_back(tangent);
			this->mesh_tangents.push_back(tangent);

			// Same thing for bitangents
			this->mesh_bitangents.push_back(bitangent);
			this->mesh_bitangents.push_back(bitangent);
			this->mesh_bitangents.push_back(bitangent);
		}

		this->logger->SimpleLog(
			Logging::LogLevel::Info,
			"Reading OBJ file: Successfully read and parsed file '%s",
			path.c_str()
		);
	}
} // namespace Engine::Rendering
