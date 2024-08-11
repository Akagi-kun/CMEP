#include "Rendering/Vulkan/Wrappers/Window.hpp"

#include "GLFW/glfw3.h"

#include <utility>

namespace Engine::Rendering::Vulkan
{

	Window* Window::GetWindowPtrFromGLFW(GLFWwindow* window)
	{
		return reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	}

	void Window::CallbackOnFramebufferResize(GLFWwindow* window, int width, int height)
	{
		auto* window_ptr = Window::GetWindowPtrFromGLFW(window);

		window_ptr->Resize({static_cast<unsigned int>(width), static_cast<unsigned int>(height)});
	}

	void Window::CallbackOnWindowFocus(GLFWwindow* window, int focused)
	{
		auto* window_ptr = Window::GetWindowPtrFromGLFW(window);

		window_ptr->is_focus = (focused > 0);
	}

	void Window::CallbackOnCursorEnterLeave(GLFWwindow* window, int entered)
	{
		auto* window_ptr = Window::GetWindowPtrFromGLFW(window);
		bool b_entered	 = (entered != 0);

		if (window_ptr->is_focus)
		{
			window_ptr->is_content = b_entered;
			glfwSetInputMode(window, GLFW_CURSOR, b_entered ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		}
		else
		{
			window_ptr->is_content = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	void Window::CallbackOnCursorPosition(GLFWwindow* window, double xpos, double ypos)
	{
		auto* window_ptr = Window::GetWindowPtrFromGLFW(window);

		if (window_ptr->is_focus)
		{
			window_ptr->cursor_position = {xpos, ypos};
		}
		else
		{
			window_ptr->cursor_position = {0.0, 0.0};
		}
	}

	void Window::CallbackOnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto* window_ptr = Window::GetWindowPtrFromGLFW(window);

		// Ignore scancode
		(void)(scancode);

		window_ptr->input_events.emplace(action, key, mods);
	}

	Window::Window(ScreenSize with_size, std::string with_title, const std::vector<std::pair<int, int>>& with_hints)
		: title(std::move(with_title)), size(with_size)
	{
		// Default hint since we use Vulkan only
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		for (const auto& [hint, value] : with_hints)
		{
			glfwWindowHint(hint, value);
		}

		native_handle = glfwCreateWindow(
			static_cast<int>(size.x),
			static_cast<int>(size.y),
			title.c_str(),
			nullptr, // glfwGetPrimaryMonitor(),
			nullptr
		);

		glfwSetWindowUserPointer(native_handle, this);
		glfwSetFramebufferSizeCallback(native_handle, Window::CallbackOnFramebufferResize);
		glfwSetWindowFocusCallback(native_handle, Window::CallbackOnWindowFocus);
		glfwSetCursorPosCallback(native_handle, Window::CallbackOnCursorPosition);
		glfwSetCursorEnterCallback(native_handle, Window::CallbackOnCursorEnterLeave);
		glfwSetKeyCallback(native_handle, Window::CallbackOnKeyEvent);
	}

	Window::~Window()
	{
		glfwDestroyWindow(native_handle);
	}

	// This function modifies external state
	// NOLINTNEXTLINE(readability-make-member-function-const)
	void Window::SetVisibility(bool visible)
	{
		if (visible)
		{
			glfwShowWindow(native_handle);
		}
		else
		{
			glfwHideWindow(native_handle);
		}
	}

	// This function modifies external state
	// NOLINTNEXTLINE(readability-make-member-function-const)
	void Window::SetShouldClose(bool should_close)
	{
		glfwSetWindowShouldClose(native_handle, static_cast<int>(should_close));
	}

	void Window::Resize(ScreenSize to_size)
	{
		is_resized = true;
		size	   = to_size;
	}

} // namespace Engine::Rendering::Vulkan
