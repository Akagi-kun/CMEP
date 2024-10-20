#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>

namespace Engine
{
	class Asset
	{
	public:
		using uid_t = uint_least32_t;

		/**
		 * Subdivide the limit of @ref uid_t into equal parts
		 */
		static consteval uid_t getUIDSubdivide(unsigned int part_current, unsigned int parts_total)
		{
			return std::numeric_limits<uid_t>::max() / parts_total * part_current;
		}

		Asset() = default;

		void assignUID(uid_t with_uid)
		{
			uid = with_uid;
		}

	protected:
		// Friend hash so it can access uid
		friend struct std::hash<Asset>;

		uid_t uid = std::numeric_limits<uid_t>::max();
	};

	/**
	 * Checks whether the provided class conforms to the @ref Engine::Asset API
	 */
	template <typename child_t>
	concept IsAsset = std::is_base_of_v<Asset, child_t>;
} // namespace Engine

namespace std
{
	/**
	 * Specialization of std::hash for @ref Engine::Asset
	 */
	template <> struct hash<Engine::Asset>
	{
		size_t operator()(const Engine::Asset& ref)
		{
			return ref.uid;
		}
	};
} // namespace std
