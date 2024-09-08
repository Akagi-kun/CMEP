#pragma once
// IWYU pragma: private; include Rendering/Vulkan/rendering.hpp

#include "fwd.hpp"

#include "Rendering/Transform.hpp"

#include "Surface.hpp"
#include "common/HandleWrapper.hpp"
#include "common/InstanceOwned.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstddef>
#include <functional>
#include <utility>

// Include GLFW
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h" // IWYU pragma: export

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
		key_value_t	   key;
		mods_value_t   mods;

		InputEvent(int with_action, int with_key, unsigned int with_mods)
			: type(static_cast<InputEventType>(with_action)),
			  key(static_cast<key_value_t>(with_key)),
			  mods(static_cast<mods_value_t>(with_mods))
		{
		}
	};

	class Window final : public InstanceOwned, public HandleWrapper<GLFWwindow*>
	{
	public:
		glm::vec<2, double>	   cursor_position;
		std::queue<InputEvent> input_events;

		struct StatusBits
		{
			bool is_resized : 1;
			bool is_focus : 1;
			bool is_content : 1;
		} status;

		Window(
			InstanceOwned::value_t					with_instance,
			ScreenSize								with_size,
			const std::string&						with_title,
			const std::vector<std::pair<int, int>>& with_hints
		);
		~Window();

		void setVisibility(bool visible);

		void			   setShouldClose(bool should_close);
		[[nodiscard]] bool getShouldClose() const;

		[[nodiscard]] const ScreenSize& getFramebufferSize() const
		{
			return size;
		}

		[[nodiscard]] Swapchain* getSwapchain()
		{
			return swapchain;
		}

		[[nodiscard]] const Surface* getSurface() const
		{
			return &surface;
		}

		void createSwapchain();
		void drawFrame();

		void setRenderCallback(
			std::function<void(Vulkan::CommandBuffer*, uint32_t, void*)> with_callback,
			void*														 with_user_data
		)
		{
			render_callback = std::move(with_callback);
			user_data		= with_user_data;
		}

	private:
		ScreenSize size;
		uint32_t   current_frame = 0;

		Swapchain* swapchain = nullptr;
		Surface	   surface;

		// Rendering related
		std::function<void(Vulkan::CommandBuffer*, uint32_t, void*)> render_callback;
		void*														 user_data = nullptr;

		static Window* getWindowPtrFromGlfw(GLFWwindow* window);

		static void callbackOnWindowFocus(GLFWwindow* window, int focused);
		static void callbackOnFramebufferResize(GLFWwindow* window, int width, int height);
		static void callbackOnCursorEnterLeave(GLFWwindow* window, int entered);
		static void callbackOnCursorPosition(GLFWwindow* window, double xpos, double ypos);
		static void callbackOnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods);

		static vk::Extent2D chooseVulkanSwapExtent(
			const Window*					  with_window,
			const vk::SurfaceCapabilitiesKHR& capabilities
		);

		void resize(ScreenSize to_size);
	};
} // namespace Engine::Rendering::Vulkan
