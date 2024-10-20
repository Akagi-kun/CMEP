#pragma once

#include "Rendering/Transform.hpp"
#include "Rendering/Vulkan/exports.hpp"

#include "Asset.hpp"
#include "InternalEngineObject.hpp"

#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

namespace Engine::Rendering
{
	struct TextureData
	{
		ImageSize				   size;
		int						   color_fmt = 4;
		std::vector<unsigned char> data;
		Vulkan::ViewedImage*	   image;
		Vulkan::Sampler*		   sampler;
	};

	class Texture final : public InternalEngineObject, public Asset
	{
	public:
		Texture(Engine* with_engine, std::unique_ptr<TextureData> init_data);
		~Texture();

		[[nodiscard]] ImageSize getSize() const
		{
			return data->size;
		}

		[[nodiscard]] auto getImage()
		{
			return data->image;
		}

		[[nodiscard]] auto getSampler()
		{
			return data->sampler;
		}

		[[nodiscard]] int getColorFormat() const
		{
			return data->color_fmt;
		}

	private:
		std::unique_ptr<TextureData> data;
	};
	static_assert(!std::is_move_constructible_v<Texture> && !std::is_move_assignable_v<Texture>);

} // namespace Engine::Rendering
