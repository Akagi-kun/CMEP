#pragma once
// IWYU pragma: private; include Rendering/Vulkan/backend.hpp

#include "Logging/Logging.hpp"

#include "vulkan/vulkan.hpp"

#include <cstdint>
#include <vector>

namespace Engine::Rendering::Vulkan
{
	class ShaderCompiler : public Logging::SupportsLogging
	{
	public:
		using glsl_code_t  = std::vector<char>;
		using spirv_code_t = std::vector<uint32_t>;

		ShaderCompiler(logger_t with_logger);
		~ShaderCompiler();

		[[nodiscard]] spirv_code_t compileShader(
			const glsl_code_t&		with_code,
			vk::ShaderStageFlagBits with_stage
		) const;

	protected:
		void logCompileEvent(const char* occured_at, const char* message_log) const;
	};
} // namespace Engine::Rendering::Vulkan
