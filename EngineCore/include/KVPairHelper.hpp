#pragma once

#include <string>

namespace Engine::Utility
{
	inline std::tuple<std::string, std::string> SplitKVPair(const std::string& from_string, std::string_view delimiter)
	{
		// Get position of delimiter in entry
		const size_t delimiter_begin = from_string.find(delimiter);
		const size_t delimiter_end	 = delimiter_begin + delimiter.size();

		if (delimiter_begin != std::string::npos)
		{
			// Get the key and value
			std::string key	  = from_string.substr(0, delimiter_begin);
			std::string value = from_string.substr(delimiter_end, from_string.size() - delimiter_end);

			return {key, value};
		}

		return {from_string, ""};
	}
} // namespace Engine::Utility
