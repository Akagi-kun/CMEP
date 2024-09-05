#include "Exception.hpp"

#include "cmake_cfg.hpp"

#include <exception>
#include <filesystem>
#include <format>
#include <source_location>
#include <sstream>
#include <string>
#include <vector>

namespace Engine
{
	std::string Exception::GenerateWhat(
		const std::string&	 with_message,
		std::source_location location
	)
	{
		std::filesystem::path file = location.file_name();
		file					   = file.lexically_relative(CMAKE_CONFIGURE_SOURCE_DIR);

		return std::format(
			"({}:{}:{}):\ne.what(): '{}'",
			file.string(),
			location.line(),
			location.column(),
			with_message
		);
	}

	namespace
	{
		// prints the explanatory string of an exception. If the exception is nested,
		// recurses to print the explanatory of the exception it holds
		void PrintException(
			std::vector<std::string>& output,
			const std::exception&	  caught_exception,
			int						  level = 0
		)
		{
			output.push_back(std::format("exception {}: {}", level, caught_exception.what()));

			try
			{
				std::rethrow_if_nested(caught_exception);
			}
			catch (const std::exception& nested_exception)
			{
				PrintException(output, nested_exception, ++level);
			}
			catch (...)
			{
				throw;
			}
		}
	} // namespace

	std::string UnrollExceptions(const std::exception& caught_exception)
	{
		std::string output = "Unrolling nested exceptions...\n";

		std::vector<std::string> whats{};

		PrintException(whats, caught_exception);

		ENGINE_EXCEPTION_ON_ASSERT_NOMSG(!whats.empty())

		// Concatenate the description string for every exception
		// into a final string
		for (const auto& what : whats)
		{
			std::istringstream what_stream(what);

			// Tab out every line of output
			// except the first line
			std::string line;

			bool first_line = true;
			while (std::getline(what_stream, line))
			{
				if (first_line)
				{
					output.append("   ");
					first_line = false;
				}
				else
				{
					output.append("      ");
				}
				output.append(std::format("{}\n", line));
			}
		}

		return output;
	}
} // namespace Engine
