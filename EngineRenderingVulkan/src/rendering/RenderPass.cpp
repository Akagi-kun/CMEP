#include "rendering/RenderPass.hpp"

#include "ImportVulkan.hpp"
#include "backend/Instance.hpp"
#include "backend/LogicalDevice.hpp"
#include "common/InstanceOwned.hpp"

#include <array>

namespace Engine::Rendering::Vulkan
{
	RenderPass::RenderPass(InstanceOwned::value_t with_instance, vk::Format with_format) : InstanceOwned(with_instance)
	{
		const auto& physical_device = instance->GetPhysicalDevice();

		const auto msaa_samples = instance->GetMSAASamples();

		vk::AttachmentDescription color_attachment(
			{},
			with_format,
			msaa_samples,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal
		);

		vk::AttachmentDescription depth_attachment(
			{},
			physical_device->FindSupportedDepthFormat(),
			msaa_samples,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		);

		vk::AttachmentDescription color_attachment_resolve(
			{},
			with_format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR
		);

		vk::AttachmentReference color_attachment_ref(0, vk::ImageLayout::eColorAttachmentOptimal);
		vk::AttachmentReference depth_attachment_ref(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		vk::AttachmentReference color_resolve_attachment_ref(2, vk::ImageLayout::eColorAttachmentOptimal);

		vk::SubpassDescription subpass(
			{},
			vk::PipelineBindPoint::eGraphics,
			{},
			color_attachment_ref,
			color_resolve_attachment_ref,
			&depth_attachment_ref,
			{}
		);
		std::array<vk::AttachmentDescription, 3> attachments =
			{color_attachment, depth_attachment, color_attachment_resolve};

		vk::SubpassDependency dependency(
			vk::SubpassExternal,
			{},
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
			vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
			{},
			vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
		);

		vk::RenderPassCreateInfo create_info({}, attachments, subpass, dependency, {});

		LogicalDevice* logical_device = instance->GetLogicalDevice();

		native_handle = logical_device->GetHandle().createRenderPass(create_info);
	}

} // namespace Engine::Rendering::Vulkan
