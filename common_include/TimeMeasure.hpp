#pragma once

/**
 * Start measuring time using @c std::chrono::steady_clock, defines @c \<name\>_start.
 */
#define TIMEMEASURE_START(name) const auto name##_start = std::chrono::steady_clock::now()

/**
 * End a previously started time measure (references @c \<name\>_start), defines
 * @c \<name\>_total which is the duration between start and end in @b milliseconds.
 */
#define TIMEMEASURE_END_MILLI(name)                                                                \
	const std::chrono::duration<double, std::milli> name##_total =                                 \
		std::chrono::steady_clock::now() - name##_start

/**
 * End a previously started time measure (references @c \<name\>_start), defines
 * @c \<name\>_total which is the duration between start and end in @b seconds.
 */
#define TIMEMEASURE_END_SECOND(name)                                                               \
	const std::chrono::duration<double> name##_total = std::chrono::steady_clock::now() -          \
													   name##_start
