#include "rendering/RenderPass.hpp"

#include "backend/Instance.hpp"
#include "backend/LogicalDevice.hpp"
#include "common/InstanceOwned.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <array>
#include <cstdint>

namespace Engine::Rendering::Vulkan
{
	RenderPass::RenderPass(InstanceOwned::value_t with_instance, vk::Format with_format)
		: InstanceOwned(with_instance)
	{
		const auto& physical_device = instance->getPhysicalDevice();

		const auto msaa_samples = physical_device->getMSAASamples();

		vk::AttachmentDescription color_attachment{
			.format			= with_format,
			.samples		= msaa_samples,
			.loadOp			= vk::AttachmentLoadOp::eClear,
			.storeOp		= vk::AttachmentStoreOp::eDontCare,
			.stencilLoadOp	= vk::AttachmentLoadOp::eDontCare,
			.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			.initialLayout	= vk::ImageLayout::eUndefined,
			.finalLayout	= vk::ImageLayout::eColorAttachmentOptimal
		};

		vk::AttachmentDescription depth_attachment{
			.format			= physical_device->findSupportedDepthFormat(),
			.samples		= msaa_samples,
			.loadOp			= vk::AttachmentLoadOp::eClear,
			.storeOp		= vk::AttachmentStoreOp::eDontCare,
			.stencilLoadOp	= vk::AttachmentLoadOp::eDontCare,
			.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			.initialLayout	= vk::ImageLayout::eUndefined,
			.finalLayout	= vk::ImageLayout::eDepthStencilAttachmentOptimal
		};

		vk::AttachmentDescription color_attachment_resolve{
			.format			= with_format,
			.samples		= vk::SampleCountFlagBits::e1,
			.loadOp			= vk::AttachmentLoadOp::eDontCare,
			.storeOp		= vk::AttachmentStoreOp::eStore,
			.stencilLoadOp	= vk::AttachmentLoadOp::eDontCare,
			.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			.initialLayout	= vk::ImageLayout::eUndefined,
			.finalLayout	= vk::ImageLayout::ePresentSrcKHR
		};

		vk::AttachmentReference color_attachment_ref{
			.attachment = 0,
			.layout		= vk::ImageLayout::eColorAttachmentOptimal
		};
		vk::AttachmentReference depth_attachment_ref{
			.attachment = 1,
			.layout		= vk::ImageLayout::eDepthStencilAttachmentOptimal
		};
		vk::AttachmentReference color_resolve_attachment_ref{
			.attachment = 2,
			.layout		= vk::ImageLayout::eColorAttachmentOptimal
		};

		vk::SubpassDescription subpass{
			.flags					 = {},
			.pipelineBindPoint		 = vk::PipelineBindPoint::eGraphics,
			.colorAttachmentCount	 = 1,
			.pColorAttachments		 = &color_attachment_ref,
			.pResolveAttachments	 = &color_resolve_attachment_ref,
			.pDepthStencilAttachment = &depth_attachment_ref
		};
		std::array<vk::AttachmentDescription, 3> attachments =
			{color_attachment, depth_attachment, color_attachment_resolve};

		vk::SubpassDependency dependency{
			.srcSubpass	  = vk::SubpassExternal,
			.dstSubpass	  = {},
			.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
							vk::PipelineStageFlagBits::eEarlyFragmentTests,
			.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
							vk::PipelineStageFlagBits::eEarlyFragmentTests,
			.srcAccessMask = {},
			.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite |
							 vk::AccessFlagBits::eDepthStencilAttachmentWrite
		};

		vk::RenderPassCreateInfo create_info{
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments	 = attachments.data(),
			.subpassCount	 = 1,
			.pSubpasses		 = &subpass,
			.dependencyCount = 1,
			.pDependencies	 = &dependency
		};

		LogicalDevice* logical_device = instance->getLogicalDevice();

		native_handle = logical_device->createRenderPass(create_info);
	}

} // namespace Engine::Rendering::Vulkan
