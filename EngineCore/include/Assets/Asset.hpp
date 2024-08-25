#pragma once

#include <cstdint>
#include <functional>

namespace Engine
{
	class Asset
	{
	public:
		Asset() = default;

		void AssignUID(uint_least32_t with_uid)
		{
			uid = with_uid;
		}

	protected:
		friend struct std::hash<Asset>;

		uint_least32_t uid;
	};
} // namespace Engine

namespace std
{
	template <> struct hash<Engine::Asset>
	{
		size_t operator()(const Engine::Asset& ref)
		{
			return ref.uid;
		}
	};
} // namespace std
