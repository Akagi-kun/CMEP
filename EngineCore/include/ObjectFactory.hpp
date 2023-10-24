#pragma once

#include "Rendering/Mesh.hpp"
#include "Object.hpp"

class Engine::Rendering::Texture;
class Engine::Rendering::Font;

namespace Engine::ObjectFactory
{
	__declspec(dllexport) Object* CreateImageObject(double x, double y, double sizex, double sizey, ::Engine::Rendering::Texture* image);
	__declspec(dllexport) Object* CreateTextObject(double x, double y, int size, std::string text, ::Engine::Rendering::Font* font);
	__declspec(dllexport) Object* CreateGeneric3DObject(double x, double y, double z, double sizex, double sizey, double sizez, double rotx, double roty, double rotz, ::Engine::Rendering::Mesh* mesh);
}