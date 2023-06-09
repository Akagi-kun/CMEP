#pragma once

#include <unordered_map>
#include <functional>
#include <memory>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "GL/glcorearb.h"

#pragma region forward decls

namespace Engine
{
	class AssetManager;
	class ImageObject;
	class TextObject;
	class Generic3DObject;
	class Object;
	namespace Rendering
	{
		typedef enum class Texture_InitFiletypeEnum
		{
			FILE_RAW = 1,
			FILE_NETPBM = 2,
			FILE_PNG = 3
		} Texture_InitFiletype;

		class Renderer;
		class Texture;
		class Window;
		class Shader;
		class Font;
		class Mesh;
	}
}

#pragma endregion

namespace Engine
{
	namespace Rendering
	{

#pragma region Mesh.hpp

		class __declspec(dllimport) Mesh final
		{
		private:
		public:
			std::vector<glm::vec3> mesh_vertices;
			std::vector<glm::vec2> mesh_uvs;
			std::vector<glm::vec3> mesh_normals;

			void CreateMeshFromObj(std::string path);
		};

#pragma endregion

#pragma region Font.hpp

		struct FontChar
		{
			int x, y, width, height, xoffset, yoffset, xadvance, page, channel;
		};

		class __declspec(dllimport) Font final
		{
		private:
			AssetManager* asset_manager;
			std::string fntfile;
			unsigned int char_count = 0;

			// Data from fnt file
			std::unordered_map<std::string, std::string> info;
			std::unordered_map<int, Texture*> pages;
			std::unordered_map<int, FontChar> chars;

			void EvalBmfont(FILE* file) noexcept;
			void EvalBmfontLine(int type, char* data) noexcept;
		public:
			Font() noexcept;
			Font(const Font& other) noexcept = delete;
			Font(const Font&& other) noexcept = delete;
			Font& operator=(const Font& other) noexcept = delete;
			Font& operator=(const Font&& other) noexcept = delete;
			~Font() noexcept;

			int Init(std::string path) noexcept;

			FontChar* GetChar(char ch) noexcept;
			Texture* GetPageTexture(int page) noexcept;
		};

#pragma endregion

#pragma region Shader.hpp

		class __declspec(dllimport) Shader final
		{
		private:
			GLuint program = 0;

			static GLuint SetupShader(const char* vert, const char* frag) noexcept;

		public:
			Shader(const char* vert, const char* frag) noexcept;

			GLuint GetProgram() const noexcept;

			bool IsValid() const noexcept;
		};

#pragma endregion

#pragma region Texture.hpp

		class __declspec(dllimport) Texture final
		{
		private:
			char* data = nullptr;
			unsigned int x = 0, y = 0;
			GLenum color_fmt = GL_RGB;
			GLuint texture = 0;

		public:
			Texture() noexcept;
			Texture(const Texture& other) noexcept;
			Texture(const Texture&& other) noexcept;
			Texture& operator=(const Texture& other) noexcept;
			Texture& operator=(const Texture&& other) noexcept;
			~Texture() noexcept;

			int InitRaw(const char* const raw_data, GLenum color_format, unsigned int x, unsigned int y);
			int InitFile(Texture_InitFiletype filetype, std::string path, unsigned int sizex = 0, unsigned int sizey = 0);

			void GetSize(unsigned int& x, unsigned int& y) const noexcept;
			char* GetData() const noexcept;
			GLuint GetTexture() const noexcept;
			GLenum GetColorFormat() const noexcept;
		};

#pragma endregion

#pragma region IRenderer.hpp

		/// <summary>
		/// Interface describing GL Renderer API for UI renderables.
		/// </summary>
		class __declspec(dllimport) IRenderer
		{
		protected:
			/// <summary>
			/// Renderable's position.
			/// </summary>
			glm::vec3 _pos = glm::vec3();
			/// <summary>
			/// Renderable's size.
			/// </summary>
			glm::vec3 _size = glm::vec3();
			/// <summary>
			/// Renderable's rotation.
			/// </summary>
			glm::vec3 _rotation = glm::vec3();

			/// <summary>
			/// Screen size as reported by <seealso cref="Object"/>.
			/// </summary>
			uint_fast16_t _screenx = 0, _screeny = 0;

			/// <summary>
			/// Used by <seealso cref="UpdateMesh"/> to optimize Updates to when necessary.
			/// </summary>
			bool has_updated_mesh = false;

		public:
			IRenderer() {};

			/// <summary>
			/// Updates data for renderer.
			/// </summary>
			/// <param name="pos">Position of renderable.</param>
			/// <param name="size">Size of renderable.</param>
			/// <param name="screenx">X size of screen.</param>
			/// <param name="screeny">Y size of screen.</param>
			virtual void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept = 0;

			/// <summary>
			/// Updates mesh of renderable.
			/// </summary>
			virtual void UpdateMesh() = 0;

			/// <summary>
			/// Render the renderable represented by this <seealso cref="Renderer"/>.
			/// </summary>
			virtual void Render() = 0;
		};

#pragma endregion

#pragma region TextRenderer.hpp

		/// <summary>
		/// Implementation of <seealso cref="IRenderer"/> for text renderables.
		/// </summary>
		/// <inheritdoc cref="IRenderer"/>
		class __declspec(dllimport) TextRenderer final : public IRenderer
		{
		private:
			/// <summary>
			/// GL Vertex Array Object
			/// </summary>
			unsigned int vao = 0;
			/// <summary>
			/// GL Vertex Buffer Object
			/// </summary>
			unsigned int vbo = 0;
			/// <summary>
			/// Count of vertices in Vertex Buffer Object
			/// </summary>
			size_t vbo_vert_count = 0;
			/// <summary>
			/// Text to be rendered
			/// </summary>
			std::string text = "";

			/// <summary>
			/// Currently used shader
			/// </summary>
			std::unique_ptr<Rendering::Shader> program;
			/// <summary>
			/// Currently used font
			/// </summary>
			std::unique_ptr<Rendering::Font> font;

		public:
			TextRenderer();
			~TextRenderer();

			void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept override;

			/// <summary>
			/// Update font used by renderer. See <see cref="font"/>.
			/// </summary>
			/// <param name="font">New font.</param>
			int UpdateFont(Rendering::Font* const font) noexcept;

			/// <summary>
			/// Update rendered text.
			/// </summary>
			/// <param name="text">New text.</param>
			int UpdateText(const std::string text) noexcept;

			void UpdateMesh() override;
			void Render() override;
		};

#pragma endregion
		
#pragma region SpriteRenderer.hpp

		/// <summary>
		/// Implementation of <seealso cref="IRenderer"/> for 2D sprite renderables.
		/// </summary>
		/// <inheritdoc cref="IRenderer"/>
		class __declspec(dllimport) SpriteRenderer final : public IRenderer
		{
		private:
			/// <summary>
			/// GL Vertex Array Object
			/// </summary>
			unsigned int vao = 0;
			/// <summary>
			/// GL Vertex Buffer Object
			/// </summary>
			unsigned int vbo = 0;

			std::unique_ptr<Rendering::Shader> program;
			std::unique_ptr<const Rendering::Texture> texture;

		public:
			SpriteRenderer();
			~SpriteRenderer();

			void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept override;
			void UpdateTexture(const Rendering::Texture* texture) noexcept;
			void UpdateMesh() noexcept override;

			void Render() override;
		};

#pragma endregion

#pragma region MeshRenderer.hpp

		/// <summary>
		/// Implementation of <seealso cref="IRenderer"/> for custom mesh renderables.
		/// </summary>
		/// <inheritdoc cref="IRenderer"/>
		class __declspec(dllimport) MeshRenderer final : public IRenderer
		{
		private:
			/// <summary>
			/// GL Vertex Array Object
			/// </summary>
			unsigned int vao = 0;
			/// <summary>
			/// GL Vertex Buffer Object
			/// </summary>
			unsigned int vbo = 0;

			glm::mat4 MVP;

			/// <summary>
			/// Currently used shader
			/// </summary>
			std::unique_ptr<Shader> program;
			std::unique_ptr<const Rendering::Texture> texture;

			std::unique_ptr<Mesh> mesh;
		public:
			MeshRenderer();
			~MeshRenderer();

			void AssignMesh(Mesh& new_mesh);

			void UpdateTexture(const Rendering::Texture* texture) noexcept;
			void Update(glm::vec3 pos, glm::vec3 size, glm::vec3 rotation, uint_fast16_t screenx, uint_fast16_t screeny) noexcept override;
			void UpdateMesh() noexcept override;

			void Render() override;
		};

#pragma endregion

	}

	namespace Scripting
	{
		enum class ExecuteType {
			EventHandler,
			ObjectScript
		};

#pragma region LuaScript.hpp
		class __declspec(dllimport) LuaScript
		{
		protected:
			void* state;

		public:
			std::string path;

			LuaScript(std::string path);
			~LuaScript();

			void* GetState();
		};
#pragma endregion

#pragma region LuaScriptExecutor.hpp
		class __declspec(dllimport) LuaScriptExecutor
		{
		protected:
			static void registerCallbacks(void* state);
		public:
			LuaScriptExecutor() {};
			~LuaScriptExecutor() {};

			static void CallIntoScript(ExecuteType etype, LuaScript* script, std::string function, void* data);

			static int LoadAndCompileScript(LuaScript* script);
		};
#pragma endregion
	}

#pragma region EventHandling.hpp

	namespace EventHandling
	{
		enum class EventType
		{
			ON_INIT,
			ON_UPDATE,
			ON_KEYDOWN,
			ON_MOUSEMOVED
		};

		class __declspec(dllimport) Event final
		{
		private:
		public:
			const EventType event_type;

			double deltaTime = 0.0;
			union
			{
				unsigned char keycode = 0; // ON_KEYDOWN event
				struct {
					uint32_t x;
					uint32_t y;
				} mouse; // ON_MOUSEMOVED event
			};

			Event(const EventType eventtype) : event_type(eventtype) {};
			~Event() {};
		};
	}

#pragma endregion

#pragma region Engine.hpp

	class __declspec(dllimport) Engine final
	{
	private:
		// Keeps internal loop running
		bool run_threads = true;

		Scripting::LuaScript* config_script;

		// Window
		void* window = nullptr;
		const unsigned windowX = 0, windowY = 0;
		const char* windowTitle = nullptr;
		unsigned framerateTarget = 30;

		// Managers
		AssetManager* asset_manager = nullptr;
		Scripting::LuaScriptExecutor* script_executor = nullptr;

		std::vector<std::pair<EventHandling::EventType, std::function<void(EventHandling::Event&)>>> event_handlers;
		std::vector<std::tuple<EventHandling::EventType, Scripting::LuaScript*, std::string>> lua_event_handlers;

		static void APIENTRY debugCallbackGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) noexcept;

		static void spinSleep(double seconds);

		void handleInput(const double deltaTime) noexcept;

		void engineLoop();

		void FireEvent(EventHandling::Event& event);
	public:
		Engine(const char* windowTitle, const unsigned windowX, const unsigned windowY) noexcept;
		~Engine() noexcept;

		void SetFramerateTarget(unsigned framerate) noexcept;

		void Init();
		void Run();

		void ConfigFile(std::string path);
		void RegisterEventHandler(EventHandling::EventType event_type, std::function<void(EventHandling::Event&)> function);
		void RegisterLuaEventHandler(EventHandling::EventType event_type, Scripting::LuaScript* script, std::string function);

		void AddObject(std::string name, Object* ptr);
		Object* FindObject(std::string name);
		size_t RemoveObject(std::string name) noexcept;

		AssetManager* GetAssetManager() noexcept;
	};

	__declspec(dllimport) Engine* initializeEngine(const char* windowTitle, const unsigned windowX, const unsigned windowY);

	__declspec(dllimport) Engine* global_engine;
#pragma endregion

#pragma region Object.hpp

	class __declspec(dllimport) Object
	{
	protected:
		/// <summary>
		/// Position of object in worldspace.
		/// </summary>
		glm::vec3 _pos = glm::vec3();

		/// <summary>
		/// Size of object.
		/// </summary>
		glm::vec3 _size = glm::vec3();

		/// <summary>
		/// Rotation of object.
		/// </summary>
		glm::vec3 _rotation = glm::vec3();

		unsigned int screenx = 0, screeny = 0;

		std::function<void(Object*)> _onClick = nullptr;

	public:
		Rendering::IRenderer* renderer = nullptr;

		Object() noexcept {}

		void ScreenSizeInform(unsigned int screenx, unsigned int screeny) noexcept
		{
			this->screenx = screenx;
			this->screeny = screeny;
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny); }
		}

		virtual void UpdatePosition(const glm::vec3 pos) noexcept
		{
			this->_pos = pos;
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny); }
		}

		virtual void UpdateSize(const glm::vec3 size) noexcept
		{
			this->_size = size;
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny); }
		}

		virtual void Rotate(const glm::vec3 rotation) noexcept
		{
			this->_rotation = rotation;
			if (this->renderer != nullptr) { this->renderer->Update(this->_pos, this->_size, this->_rotation, this->screenx, this->screeny); }
		}

		virtual int Render()
		{
			if (this->renderer != nullptr) { this->renderer->Render(); }
			return 0;
		}

		void RegisterOnClick(std::function<void(Object*)> f) noexcept { this->_onClick = f; };
		void onClick()
		{
			try { this->_onClick(this); }
			// std::bad_function_call is thrown when this->_onClick has no function assigned, ignore
			catch (std::bad_function_call e) { /* exception ignored */ }
		}

		glm::vec3 pos() const noexcept { return this->_pos; }
		glm::vec3 size() const noexcept { return this->_size; }
		glm::vec3 rotation() const noexcept { return this->_rotation; }

	};

#pragma endregion

#pragma region AssetManager.hpp

	class __declspec(dllimport) AssetManager final
	{
	private:
		std::unordered_map<std::string, Rendering::Texture*> textures;
		std::unordered_map<std::string, Rendering::Shader*> shaders;
		std::unordered_map<std::string, Rendering::Font*> fonts;
	public:
		AssetManager() {};

		void AddShader(std::string name, std::string vert_source, std::string frag_source);
		void AddTexture(std::string name, std::string path, Rendering::Texture_InitFiletype filetype);
		void AddFont(std::string name, std::string path);

		Rendering::Shader* GetShader(std::string path);
		Rendering::Texture* GetTexture(std::string path);
		Rendering::Font* GetFont(std::string path);
	};

#pragma endregion

#pragma region GlobalSceneManager.hpp

	class __declspec(dllimport) GlobalSceneManager final
	{
	private:
		std::unordered_map<std::string, Object*> objects;

		glm::vec3 cameraTransform; // XYZ position
		glm::vec2 cameraHVRotation; // Horizontal and Vertical rotation
	public:
		GlobalSceneManager();
		~GlobalSceneManager();

		const std::unordered_map<std::string, Object*>* const GetAllObjects() noexcept;

		void AddObject(std::string name, Object* ptr);
		Object* FindObject(std::string name);
		size_t RemoveObject(std::string name) noexcept;

		glm::vec3 GetCameraTransform();
		glm::vec2 GetCameraHVRotation();
		glm::mat4 GetCameraViewMatrix();

		void SetCameraTransform(glm::vec3 transform);
		void SetCameraHVRotation(glm::vec2 hvrotation);
	};

	__declspec(dllimport) GlobalSceneManager* global_scene_manager;

#pragma endregion

	namespace ObjectFactory
	{
#pragma region ObjectFactory.hpp

		__declspec(dllimport) Object* CreateImageObject(double x, double y, double sizex, double sizey, ::Engine::Rendering::Texture* image);
		__declspec(dllimport) Object* CreateTextObject(double x, double y, int size, std::string text, ::Engine::Rendering::Font* font);
		__declspec(dllimport) Object* CreateGeneric3DObject(double x, double y, double z, double sizex, double sizey, double sizez, double rotx, double roty, double rotz, ::Engine::Rendering::Mesh mesh);

#pragma endregion
	}
}
