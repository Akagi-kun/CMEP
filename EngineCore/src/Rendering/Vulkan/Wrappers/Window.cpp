#include "Rendering/Vulkan/Wrappers/Window.hpp"

#include "Rendering/Vulkan/Wrappers/Swapchain.hpp"

#include "GLFW/glfw3.h"
#include "vulkan/vulkan_core.h"

#include <stdexcept>
#include <utility>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	Window::Window(
		InstanceOwned::value_t with_instance,
		ScreenSize with_size,
		const std::string& with_title,
		const std::vector<std::pair<int, int>>& with_hints
	)
		: InstanceOwned(with_instance), size(with_size)
	{
		// Default hint since we use Vulkan only
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		for (const auto& [hint, value] : with_hints)
		{
			glfwWindowHint(hint, value);
		}

		memset(&status, 0, sizeof(StatusBits));

		native_handle = glfwCreateWindow(
			static_cast<int>(size.x),
			static_cast<int>(size.y),
			with_title.c_str(),
			nullptr, // glfwGetPrimaryMonitor(),
			nullptr
		);

		glfwSetWindowUserPointer(native_handle, this);
		glfwSetFramebufferSizeCallback(native_handle, Window::CallbackOnFramebufferResize);
		glfwSetWindowFocusCallback(native_handle, Window::CallbackOnWindowFocus);
		glfwSetCursorPosCallback(native_handle, Window::CallbackOnCursorPosition);
		glfwSetCursorEnterCallback(native_handle, Window::CallbackOnCursorEnterLeave);
		glfwSetKeyCallback(native_handle, Window::CallbackOnKeyEvent);

		// Surface ops
		surface.created_by = with_instance;
		if (glfwCreateWindowSurface(*surface.created_by, native_handle, nullptr, &surface.native_handle) != VK_SUCCESS)
		{
			throw std::runtime_error("glfw failed to create window surface!");
		}

		// this->CreateSwapchain();
	}

	Window::~Window()
	{
		delete swapchain;

		vkDestroySurfaceKHR(*surface.created_by, surface.native_handle, nullptr);

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

	[[nodiscard]] bool Window::GetShouldClose() const
	{
		return glfwWindowShouldClose(native_handle) != 0;
	}

#pragma endregion

#pragma region Private

	Window* Window::GetWindowPtrFromGLFW(GLFWwindow* window)
	{
		return reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	}

	void Window::CallbackOnFramebufferResize(GLFWwindow* window, int width, int height)
	{
		auto* self = Window::GetWindowPtrFromGLFW(window);

		self->Resize({static_cast<ScreenSize::value_t>(width), static_cast<ScreenSize::value_t>(height)});
	}

	void Window::CallbackOnWindowFocus(GLFWwindow* window, int focused)
	{
		auto* self = Window::GetWindowPtrFromGLFW(window);

		self->status.is_focus = (focused > 0);
	}

	void Window::CallbackOnCursorEnterLeave(GLFWwindow* window, int entered)
	{
		auto* self	   = Window::GetWindowPtrFromGLFW(window);
		bool b_entered = (entered != 0);

		if (self->status.is_focus)
		{
			self->status.is_content = b_entered;
			glfwSetInputMode(window, GLFW_CURSOR, b_entered ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		}
		else
		{
			self->status.is_content = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	void Window::CallbackOnCursorPosition(GLFWwindow* window, double xpos, double ypos)
	{
		auto* self = Window::GetWindowPtrFromGLFW(window);

		if (self->status.is_focus)
		{
			self->cursor_position = {xpos, ypos};
		}
		else
		{
			self->cursor_position = {0.0, 0.0};
		}
	}

	void Window::CallbackOnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto* self = Window::GetWindowPtrFromGLFW(window);

		// Ignore scancode
		(void)(scancode);

		self->input_events.emplace(action, key, mods);
	}

	static VkExtent2D ChooseVulkanSwapExtent(
		const Window* const with_window,
		const VkSurfaceCapabilitiesKHR& capabilities
	)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		ScreenSize fb_size = with_window->GetFramebufferSize();

		VkExtent2D actual_extent = {static_cast<uint32_t>(fb_size.x), static_cast<uint32_t>(fb_size.y)};

		actual_extent.width =
			std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actual_extent.height =
			std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actual_extent;
	}

	void Window::CreateSwapchain()
	{
		// Get device and surface Swap Chain capabilities
		SwapChainSupportDetails swap_chain_support = surface.QueryVulkanSwapChainSupport(instance->GetPhysicalDevice());

		VkExtent2D extent = ChooseVulkanSwapExtent(this, swap_chain_support.capabilities);

		// Request one image more than is the required minimum
		// uint32_t swapchain_image_count = swap_chain_support.capabilities.minImageCount + 1;
		// Temporary fix for screen lag
		// uint32_t swapchain_image_count = 1;
		uint32_t swapchain_image_count = VulkanRenderingEngine::max_frames_in_flight;

		// Check if there is a defined maximum (maxImageCount > 0)
		// where 0 is a special value meaning no maximum
		//
		// And if there is a maximum, clamp swap chain length to it
		if (swap_chain_support.capabilities.maxImageCount > 0)
		{
			swapchain_image_count = std::clamp(
				swapchain_image_count,
				swap_chain_support.capabilities.minImageCount,
				swap_chain_support.capabilities.maxImageCount
			);
		}

		this->swapchain = new Swapchain(instance, &surface, extent, swapchain_image_count);
	}

	void Window::Resize(ScreenSize to_size)
	{
		status.is_resized = true;
		size			  = to_size;

		LogicalDevice* logical_device = instance->GetLogicalDevice();

		// If window is minimized, wait for it to show up again
		ScreenSize framebuffer;
		do
		{
			framebuffer = this->GetFramebufferSize();
			glfwWaitEvents();
		} while (framebuffer.x == 0 || framebuffer.y == 0);

		logical_device->WaitDeviceIdle();

		// Clean up old swap chain
		delete swapchain;

		// Create a new swap chain
		this->CreateSwapchain();
	}

#pragma endregion

} // namespace Engine::Rendering::Vulkan
