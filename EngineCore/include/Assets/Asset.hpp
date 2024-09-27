#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace Engine
{
	class Asset
	{
	public:
		Asset() = default;

		using uid_t = uint_least32_t;

		void assignUID(uid_t with_uid)
		{
			uid = with_uid;
		}

	protected:
		// Friend hash so it can access uid
		friend struct std::hash<Asset>;

		uid_t uid;
	};

	/**
	 * @brief Checks whether the provided class conforms to the @ref Engine::Asset API
	 */
	template <typename child_t>
	concept IsAsset = std::is_base_of_v<Asset, child_t>;
} // namespace Engine

namespace std
{
	/**
	 * @brief Specialization of std::hash for @ref Engine::Asset
	 */
	template <> struct hash<Engine::Asset>
	{
		size_t operator()(const Engine::Asset& ref)
		{
			return ref.uid;
		}
	};
} // namespace std
