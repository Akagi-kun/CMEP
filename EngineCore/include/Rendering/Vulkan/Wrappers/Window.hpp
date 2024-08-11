#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/ImportVulkan.hpp"

#include <bitset>
#include <cstdint>
#include <queue>
#include <string>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	struct InputEvent final
	{
		static constexpr size_t mods_highest_bit = 6; // GLFW_MOD_NUM_LOCK;

		enum InputEventType : uint_least8_t
		{
			KEY_PRESS	= GLFW_PRESS,
			KEY_RELEASE = GLFW_RELEASE,
			KEY_REPEAT	= GLFW_REPEAT
		};

		using key_value_t  = uint_least16_t;
		using mods_value_t = std::bitset<mods_highest_bit>;

		InputEventType type;
		key_value_t key;
		mods_value_t mods;

		InputEvent(int with_action, int with_key, unsigned int with_mods)
			: type(static_cast<InputEventType>(with_action)), key(static_cast<key_value_t>(with_key)),
			  mods(static_cast<mods_value_t>(with_mods))
		{
		}
	};

	struct Window final
	{
	public:
		GLFWwindow* native_handle;
		bool is_resized = false;

		bool is_focus	= false;
		bool is_content = false;

		Vector2<double> cursor_position;
		std::queue<InputEvent> input_events;

		Window(ScreenSize with_size, std::string with_title, const std::vector<std::pair<int, int>>& with_hints);
		~Window();

		void SetVisibility(bool visible);
		void SetShouldClose(bool should_close);

		[[nodiscard]] const ScreenSize& GetFramebufferSize() const
		{
			return this->size;
		}

	private:
		ScreenSize size;
		std::string title;

		static Window* GetWindowPtrFromGLFW(GLFWwindow* window);

		static void CallbackOnWindowFocus(GLFWwindow* window, int focused);
		static void CallbackOnFramebufferResize(GLFWwindow* window, int width, int height);
		static void CallbackOnCursorEnterLeave(GLFWwindow* window, int entered);
		static void CallbackOnCursorPosition(GLFWwindow* window, double xpos, double ypos);
		static void CallbackOnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);

		void Resize(ScreenSize to_size);
	};
} // namespace Engine::Rendering::Vulkan
