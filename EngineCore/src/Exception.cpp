#include "Exception.hpp"

namespace Engine
{
	namespace
	{
		// prints the explanatory string of an exception. If the exception is nested,
		// recurses to print the explanatory of the exception it holds
		void PrintException(
			std::string& output_str,
			const std::exception& caught_exception,
			int level = 0
		)
		{
			output_str += "\n\texception (" + std::to_string(level) +
						  "): " + caught_exception.what();

			try
			{
				std::rethrow_if_nested(caught_exception);
			}
			catch (const std::exception& nested_exception)
			{
				PrintException(output_str, nested_exception, level - 1);
			}
			catch (...)
			{
				throw;
			}
		}
	} // namespace

	std::string UnrollExceptions(const std::exception& caught_exception)
	{
		std::string output = "Unrolling nested exceptions...";

		PrintException(output, caught_exception);

		return output;
	}
} // namespace Engine
