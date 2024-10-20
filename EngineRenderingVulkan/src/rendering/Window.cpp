#include "rendering/Window.hpp"

#include "Rendering/Transform.hpp"

#include "Exception.hpp"
#include "backend/Instance.hpp"
#include "rendering/Swapchain.hpp"
#include "vulkan/vulkan_enums.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <format>
#include <limits>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	Window::Window(
		InstanceOwned::value_t					with_instance,
		ScreenSize								with_size,
		const std::string&						with_title,
		const std::vector<std::pair<int, int>>& with_hints
	)
		: InstanceOwned(with_instance), size(with_size)
	{
		for (const auto& [hint, value] : with_hints)
		{
			glfwWindowHint(hint, value);
		}

		// Disable this hint since we use Vulkan only
		// and don't need glfw to load any API for us
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Reset all status bits to 0
		memset(&status, 0, sizeof(StatusBits));

		native_handle = glfwCreateWindow(
			static_cast<int>(size.x),
			static_cast<int>(size.y),
			with_title.c_str(),
			nullptr, // glfwGetPrimaryMonitor(),
			nullptr
		);

		// Set callbacks
		glfwSetWindowUserPointer(native_handle, this);
		glfwSetFramebufferSizeCallback(native_handle, Window::callbackOnFramebufferResize);
		glfwSetWindowFocusCallback(native_handle, Window::callbackOnWindowFocus);
		glfwSetCursorPosCallback(native_handle, Window::callbackOnCursorPosition);
		glfwSetCursorEnterCallback(native_handle, Window::callbackOnCursorEnterLeave);
		glfwSetKeyCallback(native_handle, Window::callbackOnKeyEvent);

		// Surface ops
		surface.created_by = with_instance;

		/**
		 * @todo Make a constructor for Surface
		 */
		if (glfwCreateWindowSurface(
				*instance->getHandle(),
				native_handle,
				nullptr,
				reinterpret_cast<VkSurfaceKHR*>(&surface.native_handle)
			) != VK_SUCCESS)
		{
			throw ENGINE_EXCEPTION("glfw failed to create window surface!");
		}
	}

	Window::~Window()
	{
		delete swapchain;

		(*instance->getHandle()).destroySurfaceKHR(surface.native_handle);

		glfwDestroyWindow(native_handle);
	}

	// NOLINTNEXTLINE(readability-make-member-function-const) This function modifies internal state
	void Window::setVisibility(bool visible)
	{
		if (visible) { glfwShowWindow(native_handle); }
		else { glfwHideWindow(native_handle); }
	}

	// NOLINTNEXTLINE(readability-make-member-function-const) This function modifies internal state
	void Window::setShouldClose(bool should_close)
	{
		assert(should_close == true);
		glfwSetWindowShouldClose(native_handle, static_cast<int>(should_close));
	}

	bool Window::getShouldClose() const
	{
		return glfwWindowShouldClose(native_handle) != 0;
	}

	void Window::createSwapchain()
	{
		// Get device and surface Swap Chain capabilities
		SwapChainSupportDetails swap_chain_support =
			surface.querySwapChainSupport(*instance->getPhysicalDevice());

		vk::Extent2D extent = chooseVulkanSwapExtent(this, swap_chain_support.capabilities);

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

#define CHECK_VKRESULT(op_result)                                                                  \
	do                                                                                             \
	{                                                                                              \
		if ((op_result) != vk::Result::eSuccess)                                                   \
		{                                                                                          \
			throw ENGINE_EXCEPTION(std::format(                                                    \
				"Vulkan call returned '{}' (expected 'eSuccess')",                                 \
				static_cast<std::underlying_type_t<vk::Result>>(op_result)                         \
			));                                                                                    \
		}                                                                                          \
	} while (0)

	void Window::drawFrame()
	{
		LogicalDevice* logical_device = instance->getLogicalDevice();

		auto& render_target = swapchain->getRenderTarget(current_frame);

		// Wait for fence
		CHECK_VKRESULT(
			logical_device->waitForFences(*render_target.sync_objects.in_flight, vk::True, UINT64_MAX)
		);

		// Reset fence after wait is over
		// (fence has to be reset before being used again)
		logical_device->resetFences(*render_target.sync_objects.in_flight);

		// Index of framebuffer in vk_swap_chain_framebuffers
		uint32_t image_index = 0;
		// Acquire render target
		// the render target is an image in the swap chain
		{
			vk::Result result;
			std::tie(result, image_index) = swapchain->getHandle().acquireNextImage(
				UINT64_MAX,
				*render_target.sync_objects.image_available,
				{}
			);

			// Framebuffer is being resized
			if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
			{
				return;
			}

			CHECK_VKRESULT(result);
		}

		// Reset command buffer to initial state
		render_target.command_buffer->reset();

		// Records render into command buffer
		swapchain->renderFrame(render_target.command_buffer, image_index, render_callback, user_data);

		vk::CommandBuffer command_buffers[] = {*render_target.command_buffer};

		vk::Semaphore		   wait_semaphores[] = {*render_target.sync_objects.image_available};
		vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
		std::array<vk::Semaphore, 1> signal_semaphores = {*render_target.sync_objects.present_ready};

		vk::SubmitInfo submit_info{
			.waitSemaphoreCount	  = 1,
			.pWaitSemaphores	  = wait_semaphores,
			.pWaitDstStageMask	  = wait_stages,
			.commandBufferCount	  = 1,
			.pCommandBuffers	  = command_buffers,
			.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size()),
			.pSignalSemaphores	  = signal_semaphores.data()
		};

		// Submit to queue
		// passed fence will be signaled when command buffer execution is finished
		logical_device->getGraphicsQueue().submit(submit_info, *render_target.sync_objects.in_flight);

		// Increment current frame
		current_frame = (current_frame + 1) % max_frames_in_flight;

		vk::SwapchainKHR swap_chains[] = {*swapchain->getHandle()};

		// Present current image to the screen

		vk::PresentInfoKHR present_info{
			.waitSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size()),
			.pWaitSemaphores	= signal_semaphores.data(),
			.swapchainCount		= 1,
			.pSwapchains		= swap_chains,
			.pImageIndices		= &image_index
		};

		CHECK_VKRESULT(logical_device->getPresentQueue().presentKHR(present_info));
	}

#pragma endregion

#pragma region Private

	namespace
	{
		Window* getWindowPtrFromGlfw(GLFWwindow* window)
		{
			return reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		}
	} // namespace

	void Window::callbackOnFramebufferResize(GLFWwindow* window, int width, int height)
	{
		auto* self = getWindowPtrFromGlfw(window);

		self->resize({width, height});
	}

	void Window::callbackOnWindowFocus(GLFWwindow* window, int focused)
	{
		auto* self = getWindowPtrFromGlfw(window);

		self->status.is_focus = (focused > 0);
	}

	void Window::callbackOnCursorEnterLeave(GLFWwindow* window, int entered)
	{
		auto* self		= getWindowPtrFromGlfw(window);
		bool  b_entered = (entered != 0);

		if (self->status.is_focus)
		{
			self->status.is_content = b_entered;
			glfwSetInputMode(
				window,
				GLFW_CURSOR,
				b_entered ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
			);
		}
		else
		{
			self->status.is_content = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	void Window::callbackOnCursorPosition(GLFWwindow* window, double xpos, double ypos)
	{
		auto* self = getWindowPtrFromGlfw(window);

		if (self->status.is_focus) { self->cursor_position = {xpos, ypos}; }
		else { self->cursor_position = {0.0, 0.0}; }
	}

	void Window::callbackOnKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto* self = getWindowPtrFromGlfw(window);

		// Ignore scancode
		(void)(scancode);

		self->keyboard_events.emplace(action, key, mods);
	}

	vk::Extent2D Window::chooseVulkanSwapExtent(
		const Window* const				  with_window,
		const vk::SurfaceCapabilitiesKHR& capabilities
	)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}

		ScreenSize fb_size = with_window->getFramebufferSize();

		vk::Extent2D actual_extent = {
			static_cast<uint32_t>(fb_size.x),
			static_cast<uint32_t>(fb_size.y)
		};

		actual_extent.width = std::clamp(
			actual_extent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actual_extent.height = std::clamp(
			actual_extent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);

		return actual_extent;
	}

	void Window::resize(ScreenSize to_size)
	{
		status.is_resized = true;
		size			  = to_size;

		LogicalDevice* logical_device = instance->getLogicalDevice();

		// If window is minimized, wait for it to show up again
		ScreenSize framebuffer;
		do
		{
			framebuffer = getFramebufferSize();
			glfwWaitEvents();
		} while (framebuffer.x == 0 || framebuffer.y == 0);

		logical_device->waitIdle();

		// Clean up old swap chain
		delete swapchain;

		// Create a new swap chain
		createSwapchain();
	}

#pragma endregion

} // namespace Engine::Rendering::Vulkan
