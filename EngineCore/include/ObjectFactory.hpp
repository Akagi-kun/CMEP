#pragma once

#include "Rendering/Mesh.hpp"
#include "Object.hpp"
#include "PlatformSemantics.hpp"

namespace Engine::Rendering
{
	class Texture;
	class Font;
}

namespace Engine::ObjectFactory
{
	CMEP_EXPORT Object* CreateSpriteObject(double x, double y, double sizex, double sizey, std::shared_ptr<::Engine::Rendering::Texture> sprite);
	CMEP_EXPORT Object* CreateTextObject(double x, double y, int size, std::string text, ::Engine::Rendering::Font* font);
	CMEP_EXPORT Object* CreateGeneric3DObject(double x, double y, double z, double sizex, double sizey, double sizez, double rotx, double roty, double rotz, std::shared_ptr<::Engine::Rendering::Mesh> mesh);
}