#include "backend/ShaderCompiler.hpp"

#include "Logging/Logging.hpp"

#include "Exception.hpp"
#include "TimeMeasure.hpp"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/SPIRV/Logger.h"

#include <chrono>
#include <cstring>
#include <string>
#include <utility>

namespace Engine::Rendering::Vulkan
{
#pragma region Public

	ShaderCompiler::ShaderCompiler(logger_t with_logger)
		: Logging::SupportsLogging(std::move(with_logger))
	{
		// Should only call this once
		// it's assumed that there won't be more than 1 ShaderCompiler currently
		glslang::InitializeProcess();

		const auto version = glslang::GetVersion();

		std::string spirv_version;
		glslang::GetSpirvVersion(spirv_version);

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::Info,
			"Initialized (Using glslang v%u.%u.%u for SPIR-V '%s')",
			version.major,
			version.minor,
			version.patch,
			spirv_version.c_str()
		);
	}
	ShaderCompiler::~ShaderCompiler()
	{
		// Should only call this once
		// it's assumed that there won't be more than 1 ShaderCompiler currently
		glslang::FinalizeProcess();
	}

	ShaderCompiler::spirv_code_t ShaderCompiler::compileShader(
		const ShaderCompiler::glsl_code_t& with_code,
		vk::ShaderStageFlagBits			   with_stage
	) const
	{
		EShLanguage stage_lang;
		switch (with_stage)
		{
			case vk::ShaderStageFlagBits::eFragment: stage_lang = EShLangFragment; break;
			case vk::ShaderStageFlagBits::eVertex:	 stage_lang = EShLangVertex; break;
			default:								 throw ENGINE_EXCEPTION("Passed invalid stage to ShaderModule constructor!");
		}

		TIMEMEASURE_START(compile);

		glslang::TShader shader = glslang::TShader(stage_lang);

		// Input code
		const char* code_strings[] = {with_code.data()};
		const int	code_lengths[] = {static_cast<int>(with_code.size())};
		shader.setStringsWithLengths(code_strings, code_lengths, 1);

		// Vulkan dialect (v. 100) GLSL compile to SPIR-V 1.0 targeting Vulkan 1.2
		constexpr int dialect_version = 100;
		shader.setEnvInput(
			glslang::EShSourceGlsl,
			stage_lang,
			glslang::EShClientVulkan,
			dialect_version
		);
		shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
		shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

		shader.parse(GetDefaultResources(), dialect_version, false, EShMsgDefault);

		const char* current_log = shader.getInfoLog();
		if (strlen(current_log) > 0) { logCompileEvent("Shader parse", current_log); }

		// Link shader into a program
		glslang::TProgram program;
		program.addShader(&shader);
		program.link(EShMsgDefault);

		current_log = program.getInfoLog();
		if (strlen(current_log) > 0) { logCompileEvent("Program link", current_log); }

		// Get intermediate form from linked program
		glslang::TIntermediate* intermediate = program.getIntermediate(stage_lang);

		// Generator options
		glslang::SpvOptions spirv_options = {};
		spirv_options.validate			  = true;

		// Compile intermediate form to SPIR-V
		spv::SpvBuildLogger build_logger;
		spirv_code_t		spirv_code;
		glslang::GlslangToSpv(*intermediate, spirv_code, &build_logger, &spirv_options);

		current_log = build_logger.getAllMessages().c_str();
		if (strlen(current_log) > 0) { logCompileEvent("SPIR-V build", current_log); }

		TIMEMEASURE_END_MILLI(compile);

		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::VerboseDebug,
			"Compiled shader in %.3lfms",
			compile_total.count()
		);

		return spirv_code;
	}

#pragma endregion

#pragma region Protected

	void ShaderCompiler::logCompileEvent(const char* occured_at, const char* message_log) const
	{
		this->logger->simpleLog<decltype(this)>(
			Logging::LogLevel::Warning,
			"Potentially important compilation event occured at '%s', log: '%s'",
			occured_at,
			message_log
		);
	}

#pragma endregion
} // namespace Engine::Rendering::Vulkan
