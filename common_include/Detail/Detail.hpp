#pragma once

namespace Engine::Detail
{
	struct DisableCopy
	{
		// Needs to be explicitly defaulted
		DisableCopy() = default;

		DisableCopy(DisableCopy&)			 = delete;
		DisableCopy& operator=(DisableCopy&) = delete;
	};
} // namespace Engine::Detail
