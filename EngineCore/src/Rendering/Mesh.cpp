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
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
				

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
					this->mesh_vertices.push_back(glm::vec3(vx, vy, vz));

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0) {
						tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
						tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
						tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
						this->mesh_normals.push_back(glm::vec3(nx, ny, nz));
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0) {
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

						this->mesh_ambient.push_back(glm::vec3(face_ambient[0], face_ambient[1], face_ambient[2]));
						this->mesh_diffuse.push_back(glm::vec3(face_diffuse[0], face_diffuse[1], face_diffuse[2]));
						this->mesh_specular.push_back(glm::vec3(face_specular[0], face_specular[1], face_specular[2]));
						this->mesh_dissolve.push_back(face_dissolve);
					}
				}
				index_offset += fv;
			}

		}
		
		/*
		this->mesh_vertices.clear();
		this->mesh_uvs.clear();
		this->mesh_normals.clear();

		std::vector <unsigned int> vertexIndices, uvIndices, normalIndices;
		std::vector <glm::vec3> temp_vertices;
		std::vector <glm::vec2> temp_uvs;
		std::vector <glm::vec3> temp_normals;

		FILE* file = fopen(path.c_str(), "r");
		if (file == NULL) {
			Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Reading OBJ file: Could not open file '%s'", path.c_str());
			return;
		}

		while (1)
		{

			char lineHeader[128];
			// read the first word of the line
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
			{
				break; // EOF = End Of File. Quit the loop.
			}

			if (strcmp(lineHeader, "v") == 0)
			{
				glm::vec3 vertex;
				(void)fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0)
			{
				glm::vec2 uv;
				(void)fscanf(file, "%f %f\n", &uv.x, &uv.y);
				temp_uvs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0)
			{
				glm::vec3 normal;
				(void)fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				temp_normals.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0)
			{
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9) {
					Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Error, "Reading OBJ file: Triangle indice format malformed or incompatible.");
					return;
				}
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
			}
		}

		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			unsigned int vertexIndex = vertexIndices[i];
			glm::vec3 vertex = temp_vertices[vertexIndex - 1];
			this->mesh_vertices.push_back(vertex);
		}

		for (unsigned int j = 0; j < uvIndices.size(); j++)
		{
			unsigned int uvIndex = uvIndices[j];
			glm::vec2 uv = temp_uvs[uvIndex - 1];
			this->mesh_uvs.push_back(uv);
		}

		for (unsigned int k = 0; k < normalIndices.size(); k++)
		{
			unsigned int normalIndex = normalIndices[k];
			glm::vec3 normal = temp_normals[normalIndex - 1];
			this->mesh_normals.push_back(normal);
		}
		*/

		Logging::GlobalLogger->SimpleLog(Logging::LogLevel::Info, "Reading OBJ file: Successfully read and parsed file '%s", path.c_str());
	}
}