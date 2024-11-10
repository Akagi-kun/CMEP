#pragma once
// IWYU pragma: private; include Exception.hpp

#include "PlatformSemantics.hpp"

#include <exception>
#include <format>
#include <memory>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>

namespace Engine::Base
{
	class CMEP_EXPORT_CLASS Exception : public std::exception
	{
	public:
		CMEP_EXPORT Exception(
			const std::string&		   with_message,
			const std::source_location location = std::source_location::current()
		)
			: message(generateWhat(with_message, location))
		{}

		[[nodiscard]] CMEP_EXPORT const char* what() const noexcept override
		{
			return message.c_str();
		}

	private:
		std::string message;

		static CMEP_EXPORT std::string generateWhat(
			const std::string&	 with_message,
			std::source_location location
		);
	};

	/**
	 * Unroll nested exceptions into a pretty-printed trace
	 *
	 * @note This function expects exceptions to be @ref Exception, but is able to handle
	 *       any exception derived from std::exception (i.e. all exceptions thrown by the std lib)
	 *
	 * @param caught_exception Initial exception, is expected to be a nested exception
	 *
	 * @return String containing the trace of all unrolled exceptions
	 */
	[[nodiscard]] CMEP_EXPORT std::string unrollExceptions(const std::exception& caught_exception);

	/**
	 * Assert @p expr to be true, otherwise throw.
	 * Serves the purpose of @code assert(expr) @endcode for release builds
	 *
	 * @sa EXCEPTION_ASSERT
	 */
	inline void exceptionAssert(
		bool				 expr,
		std::string_view	 message,
		const char* const	 expr_str,
		std::source_location forward_location = std::source_location::current()
	)
	{
		if (!expr)
		{
			throw Exception(
				std::format("Failed exceptionAssert '{}' {}", expr_str, message),
				forward_location
			);
		}
	}

	// NOLINTBEGIN(*unused-macros)
#define ENGINE_EXCEPTION(message) ::Engine::Base::Exception(message)

/**
 * Assert that @p true_expr is true, otherwise throw an exception
 */
#define EXCEPTION_ASSERT(true_expr, message)                                                       \
	::Engine::Base::exceptionAssert(static_cast<bool>(true_expr), message, #true_expr)
	// NOLINTEND(*unused-macros)

	template <typename object_t>
	object_t checkOrThrow(object_t value)
		requires(std::is_pointer_v<object_t>)
	{
		EXCEPTION_ASSERT(value != nullptr, "Check failed! value = nullptr");
		return value;
	}

	template <typename class_t>
	std::optional<class_t> checkOrThrow(std::optional<class_t> value)
	{
		EXCEPTION_ASSERT(value.has_value(), "Check failed! has_value() = false");
		return value;
	}

	// weak_ptr has to be locked when checked because expired() is unreliable
	template <typename class_t>
	std::shared_ptr<class_t> checkOrThrow(std::weak_ptr<class_t> value)
	{
		auto locked_value = value.lock();
		EXCEPTION_ASSERT(locked_value, "Check failed! could not lock()!");

		return locked_value;
	}

	// NOLINTBEGIN(*unused-macros)
#define CHECK(v) ::Engine::Base::checkOrThrow(v)

#define UNWRAP(v) ::Engine::Base::checkOrThrow(v).value()
	// NOLINTEND(*unused-macros)

} // namespace Engine::Base
