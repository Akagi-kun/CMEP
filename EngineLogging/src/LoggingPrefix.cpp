#include "Logging.hpp"

namespace Engine
{
	class Engine;

	class Object;

	class Scene;
	class SceneLoader;
	class SceneManager;

	class AssetManager;

	namespace Factories
	{
		class FontFactory;
		class TextureFactory;
	} // namespace Factories

	namespace Scripting
	{
		class ILuaScript;
	}

	namespace Rendering
	{
		class Font;
		class Texture;

		class TextMeshBuilder;

		namespace Vulkan
		{
			class PipelineManager;
			class Instance;
		} // namespace Vulkan
	} // namespace Rendering
} // namespace Engine

// Use PREFIX_DECL(typename without ptr) = prefix_value;
#define PREFIX_DECL(class) template <> const char* prefix_internal<class>::value

namespace Logging
{
	PREFIX_DECL(Engine::Engine)								= "ENGINE ";
	PREFIX_DECL(Engine::SceneLoader)						= "SCENE_L";
	PREFIX_DECL(Engine::Scene)								= "SCENE  ";
	PREFIX_DECL(Engine::SceneManager)						= "SCENE_M";
	PREFIX_DECL(Engine::Scripting::ILuaScript)				= "SCRIPT ";
	PREFIX_DECL(Engine::Factories::FontFactory)				= "FONT_F ";
	PREFIX_DECL(Engine::Rendering::Vulkan::PipelineManager) = "VPIPE_M";
	PREFIX_DECL(Engine::Rendering::Vulkan::Instance)		= "VINSTNC";
	PREFIX_DECL(Engine::Factories::TextureFactory)			= "TXTR_F ";
	PREFIX_DECL(Engine::Object)								= "OBJECT ";
	PREFIX_DECL(Engine::Rendering::Font)					= "FONT   ";
	PREFIX_DECL(Engine::AssetManager)						= "ASSET_M";
	PREFIX_DECL(Engine::Rendering::Texture)					= "TXTR   ";
	//
	PREFIX_DECL(void)										= " ? ? ? ";

} // namespace Logging
