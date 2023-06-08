#include <assert.h>

#include "Rendering/SpriteRenderer.hpp"
#include "Rendering/MeshRenderer.hpp"
#include "Rendering/TextRenderer.hpp"
#include "Rendering/Texture.hpp"
#include "Rendering/Shader.hpp"
#include "Rendering/Font.hpp"
#include "ObjectFactory.hpp"
#include "glm/vec3.hpp"

#include "Rendering/GLCommon.hpp"

namespace Engine::ObjectFactory
{
	Object* CreateImageObject(double x, double y, double sizex, double sizey, ::Engine::Rendering::Texture* image)
	{
		assert(image != nullptr);
		Engine::Object* object = new Engine::Object();
		object->renderer = new Rendering::SpriteRenderer();
		object->Translate(glm::vec3(x, y, 0));
		object->Scale(glm::vec3(sizex, sizey, 0));
		((Rendering::SpriteRenderer*)object->renderer)->UpdateTexture(image);
		((Rendering::SpriteRenderer*)object->renderer)->UpdateMesh();
		
		return object;
	}

	Object* CreateTextObject(double x, double y, int size, std::string text, ::Engine::Rendering::Font* font)
	{
		assert(font != nullptr);
		Engine::Object* object = new Engine::Object();
		object->renderer = new Rendering::TextRenderer();
		object->Translate(glm::vec3(x, y, 0));
		object->Scale(glm::vec3(size, size, 0));
		((Rendering::TextRenderer*)object->renderer)->UpdateFont(font);
		((Rendering::TextRenderer*)object->renderer)->UpdateText(text);
		((Rendering::TextRenderer*)object->renderer)->UpdateMesh();
		
		return object;
	}

	Object* CreateGeneric3DObject(double x, double y, double z, double sizex, double sizey, double sizez, double rotx, double roty, double rotz, ::Engine::Rendering::Mesh mesh)
	{
		Engine::Object* object = new Engine::Object();
		object->renderer = new Rendering::MeshRenderer();
		object->Translate(glm::vec3(x, y, z));
		object->Scale(glm::vec3(sizex, sizey, sizez));
		object->Rotate(glm::vec3(rotx, roty, rotz));
		((Rendering::MeshRenderer*)object->renderer)->AssignMesh(mesh);
		
		return object;
	}
}