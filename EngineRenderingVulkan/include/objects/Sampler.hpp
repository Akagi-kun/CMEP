#include "fwd.hpp"

#include "vulkan/vulkan_raii.hpp"

namespace Engine::Rendering::Vulkan
{
	class Sampler : public vk::raii::Sampler
	{
	public:
		Sampler(
			LogicalDevice*		   with_device,
			vk::Filter			   with_filter,
			vk::SamplerAddressMode with_address_mode,
			float				   with_max_anisotropy
		);

	private:
		vk::Filter			   filter;
		vk::SamplerAddressMode address_mode;
	};
} // namespace Engine::Rendering::Vulkan
