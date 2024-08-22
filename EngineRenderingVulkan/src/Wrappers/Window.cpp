#include "Wrappers/Window.hpp"

#include "Wrappers/Instance.hpp"
#include "Wrappers/Swapchain.hpp"

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
		for (const auto& [hint, value] : with_hints)
		{
			glfwWindowHint(hint, value);
		}

		// Default hint since we use Vulkan only
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

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

		// TODO: Make a constructor for Surface
		if (glfwCreateWindowSurface(
				*instance->GetHandle(),
				native_handle,
				nullptr,
				reinterpret_cast<VkSurfaceKHR*>(&surface.native_handle)
			) != VK_SUCCESS)
		{
			throw std::runtime_error("glfw failed to create window surface!");
		}

		// CreateSwapchain();
	}

	Window::~Window()
	{
		delete swapchain;

		(*instance->GetHandle()).destroySurfaceKHR(surface.native_handle);

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

	bool Window::GetShouldClose() const
	{
		return glfwWindowShouldClose(native_handle) != 0;
	}

	void Window::CreateSwapchain()
	{
		// Get device and surface Swap Chain capabilities
		SwapChainSupportDetails swap_chain_support = surface.QueryVulkanSwapChainSupport(*instance->GetPhysicalDevice()
		);

		VkExtent2D extent = ChooseVulkanSwapExtent(this, swap_chain_support.capabilities);

		// Request one image more than is the required minimum
		// uint32_t swapchain_image_count = swap_chain_support.capabilities.minImageCount + 1;
		// Temporary fix for screen lag
		// uint32_t swapchain_image_count = 1;
		uint32_t swapchain_image_count = max_frames_in_flight;

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

		swapchain = new Swapchain(instance, &surface, extent, swapchain_image_count);
	}

	void Window::DrawFrame()
	{
		LogicalDevice* logical_device = instance->GetLogicalDevice();

		auto& render_target = swapchain->GetRenderTarget(current_frame);

		// Wait for fence
		{
			vk::Result result =
				logical_device->GetHandle().waitForFences(*render_target.sync_objects.in_flight, vk::True, UINT64_MAX);
			if (result != vk::Result::eSuccess)
			{
				throw std::runtime_error("Failed waiting for fences in DrawFrame!");
			}
		}

		// Reset fence after wait is over
		// (fence has to be reset before being used again)
		logical_device->GetHandle().resetFences(*render_target.sync_objects.in_flight);

		// Index of framebuffer in vk_swap_chain_framebuffers
		uint32_t image_index = 0;
		// Acquire render target
		// the render target is an image in the swap chain
		{
			vk::Result result;
			std::tie(result, image_index) =
				swapchain->GetHandle().acquireNextImage(UINT64_MAX, *render_target.sync_objects.image_available, {});
			/* std::tie(result, image_index) = (**logical_device->GetHandle())
												.acquireNextImageKHR(
													*swapchain->GetHandle(),
													UINT64_MAX,
													*render_target.sync_objects->image_available,
													{}
												); */

			// Framebuffer is being resized
			if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
			{
				return;
			}

			if (result != vk::Result::eSuccess)
			{
				throw std::runtime_error("Failed to acquire swap chain image!");
			}
		}

		// Reset command buffer to initial state´
		render_target.command_buffer->ResetBuffer();

		// Records render into command buffer
		swapchain->RenderFrame(render_target.command_buffer, image_index, render_callback, user_data);

		std::array<vk::CommandBuffer, 1> command_buffers = {*render_target.command_buffer->GetHandle()};

		vk::Semaphore wait_semaphores[] = {*(render_target.sync_objects.image_available)};
		vk::PipelineStageFlags wait_stages[] =
			{vk::PipelineStageFlagBits::eColorAttachmentOutput /* VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT */};
		vk::Semaphore signal_semaphores[] = {*(render_target.sync_objects.present_ready)};

		vk::SubmitInfo submit_info(wait_semaphores, wait_stages, command_buffers, signal_semaphores, {});

		// Submit to queue
		// passed fence will be signaled when command buffer execution is finished
		logical_device->GetGraphicsQueue().submit(submit_info, *render_target.sync_objects.in_flight);

		// Increment current frame
		current_frame = (current_frame + 1) % max_frames_in_flight;

		vk::SwapchainKHR swap_chains[] = {*swapchain->GetHandle()};

		vk::PresentInfoKHR present_info(signal_semaphores, swap_chains, image_index, {}, {});

		// Present current image to the screen
		{
			vk::Result result = logical_device->GetPresentQueue().presentKHR(present_info);
			if (result != vk::Result::eSuccess)
			{
				throw std::runtime_error("Failed presenting swapchain!");
			}
		}
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

		self->Resize({width, height});
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

	VkExtent2D Window::ChooseVulkanSwapExtent(
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

	void Window::Resize(ScreenSize to_size)
	{
		status.is_resized = true;
		size			  = to_size;

		LogicalDevice* logical_device = instance->GetLogicalDevice();

		// If window is minimized, wait for it to show up again
		ScreenSize framebuffer;
		do
		{
			framebuffer = GetFramebufferSize();
			glfwWaitEvents();
		} while (framebuffer.x == 0 || framebuffer.y == 0);

		logical_device->GetHandle().waitIdle();

		// Clean up old swap chain
		delete swapchain;

		// Create a new swap chain
		CreateSwapchain();
	}

#pragma endregion

} // namespace Engine::Rendering::Vulkan
