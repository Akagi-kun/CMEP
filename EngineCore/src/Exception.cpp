#include "Exception.hpp"

namespace Engine
{
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
			output.push_back(std::format("exception ({}): {}", level, caught_exception.what()));

			try
			{
				std::rethrow_if_nested(caught_exception);
			}
			catch (const std::exception& nested_exception)
			{
				PrintException(output, nested_exception, level - 1);
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

		for (const auto& what : whats)
		{
			std::istringstream what_stream(what);

			// Tab out every line of output
			std::string line;
			while (std::getline(what_stream, line))
			{
				output.append(std::format("{}\n\t", line));
			}
		}

		return output;
	}
} // namespace Engine
