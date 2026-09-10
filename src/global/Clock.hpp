#pragma once
#include <ctime>

#include "core.hpp"

// static const u8 months[12][10] = {
// 	"January", "February", "March", "April", "May", "June", "July",
// 	"August", "September", "October", "November","December"
// };

struct Clock {
	Clock() = delete;

	stinl std::time_t timeBegin = 0;
	stinl std::time_t timeNow = 0;
	stinl u32 timeElapsed = 0;

	static void init() {
		timeBegin = std::time(NULL);
		timeNow = timeBegin;
		timeElapsed = 0;
	}

	static u32 update() {
		timeNow = std::time(NULL);
		timeElapsed = (u32)(timeNow - timeBegin);
		return timeElapsed;
	}

	// Does not call update time
	inl static u32 time_elapsed() {
		return (u32)timeElapsed;
	}

	struct TimeBuffer {
		u32 years;
		u8 months, days, hours, minutes, seconds;
		u16 milliseconds, microseconds, nanoseconds;
	};

	static void format_time(struct timespec* tm, char* buffer) {
		u64 nanoseconds = (u64)tm->tv_nsec + (u64)(tm->tv_sec) * 1000000000ul;
		return s_format_time(nanoseconds, (u8*)buffer);	
	}

	// Howard Hinnant’s civil_from_days algorithm
	static void s_format_time(u64 nanoseconds, u8 buffer[32]) {
		static const u8 months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
	
		const u64 totalMinutes = nanoseconds / 60000000000UL;
		const u64 days = totalMinutes / 1440UL;
		const u32 minutesInDay = (u32)(totalMinutes % 1440UL);
		const u32 hour = minutesInDay / 60U;
		const u32 minute = minutesInDay % 60U;

		const u64 adjustedDays = days + 719468UL;
		const u64 era = adjustedDays / 146097UL;
		const u32 dayOfEra = (u32)(adjustedDays - era * 146097UL);
		const u32 yearOfEra = (dayOfEra - dayOfEra / 1460U + dayOfEra / 36524U - dayOfEra / 146096U) / 365U;
		const u32 dayOfYear = dayOfEra - (365U * yearOfEra + yearOfEra / 4U - yearOfEra / 100U);
		const u32 monthPrime = (5U * dayOfYear + 2U) / 153U;
		const u32 day = dayOfYear - (153U * monthPrime + 2U) / 5U + 1U;
		const u32 month = monthPrime < 10U ? monthPrime + 3U : monthPrime - 9U;
		const u32 monthOffset = (month - 1U) * 3U;
	
		u32 year = yearOfEra + (u32)(era * 400UL);
		year += month <= 2U ? 1U : 0U;
		MEMCPY_INLINE(buffer, "00-\0\0\0-0000 00:00\0\0\0\0\0\0", 24);
		buffer[0] += (u8)(day / 10U);
		buffer[1] += (u8)(day % 10U);
		MEMCPY_INLINE(buffer + 3, months + monthOffset, 3);
		buffer[7] += (u8)(year / 1000U);
		buffer[8] += (u8)(year / 100U % 10U);
		buffer[9] += (u8)(year / 10U % 10U);
		buffer[10] += (u8)(year % 10U);
		buffer[12] += (u8)(hour / 10U);
		buffer[13] += (u8)(hour % 10U);
		buffer[15] += (u8)(minute / 10U);
		buffer[16] += (u8)(minute % 10U);
	}
};
