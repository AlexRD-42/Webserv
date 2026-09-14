#pragma once
#include <cpuid.h>
#include <ctime>
#include <x86intrin.h>
#include <unistd.h>

#include "core.hpp"

/*	Hardware Clock using RDTSC:

	Uses 64 bits for the fractional component, and computes the scale to ns
	This can overflow if tsc_freq is lower than GIGA. Very few architectures
	qualify, and by shifting it with 64 clang compiles this to a perfect and beautiful:
		mov (rdx, rsi);    mulx (rax, rax, rdi);    ret

	TODO: 
		1) Make the clock thread friendly
		2) Improve calibration
		3) Separate formatting from date collection
============================================================================= */
// If 0, it will run CPUID, and if that fails, calibration
#define TSC_FREQUENCY 0	
STATIC_ASSERT(TSC_FREQUENCY == 0 || TSC_FREQUENCY > 1_G);

struct HWClock {
	static u64 firstTscTick, firstTime;
	static u64 lastTscTick, lastTime;
	static u64 tscFactor;
	static inline const u8 months[12][10] = {
		"January", "February", "March", "April", "May", "June", "July",
		"August", "September", "October", "November","December"
	};
	u64 timeNow;

	// Get absolute time cant use tsc_to_ns directly
	ATTR(inl)
	u64 update_clock() {
		u64 tscTicks = get_tsc();
		timeNow = tsc_to_ns(tscTicks);
		return timeNow;
	}

	ATTR(inl)
	u64 time_elapsed() {
		return timeNow - firstTime;
	}

	struct Date {
		u16 year;
		u8 month, day;
	};

	struct PackedDate {
		static const u16 dayMask =   0xF800;
		static const u16 monthMask = 0x0780;
		static const u16 weekMask =  0x007F;
		u16 data, year;

		u8 day() {
			return dayMask & data;
		}

		void store_day(u16 newDay) {
			data |= newDay & ~dayMask;
		}
	};

	struct DateFull {
		u32 year;
		u8 month, week, day, hour, minute, second;
		u16 millisecond, microsecond, nanosecond;
	};

	// Ben Joffe's FastDate algorithm adapted https://www.benjoffe.com/fast-date-64
	ATTR(static_inl, const)
	Date get_calendar_time(u64 nanoseconds) {
		Date output = {};
		const u64 totalSeconds = nanoseconds / 1000000000UL;
		const u64 days = totalSeconds / 86400UL;
		const u32 revDays = 303210U - (u32)days;
		const u32 revCenturies = (u32)(((u64)revDays * 470369U) >> 34U);
		const u32 revJulianDays = revDays + revCenturies - (revCenturies >> 2U);
		const u64 yearProduct = (u64)revJulianDays * 11758980U;
		const u32 marchYear = 2799U - (u32)(yearProduct >> 32U);
		const u32 revYearPos = (u32)(((u64)(u32)yearProduct * 782432U) >> 32U);
		const u32 isJanOrFeb = revYearPos < 126464U;
		const u32 monthDay = (marchYear & 3U) * 512U + (isJanOrFeb ? 191360U : 977792U) - revYearPos;
		output.year = marchYear + isJanOrFeb;
		output.month = (u8)(monthDay >> 16U);
		output.day = (u8)((((u64)(u16)monthDay * 2006994U) >> 32U) + 1U);
		return output;
	}

	ATTR(static_inl, const)
	DateFull get_calendar_time_full(u64 nanoseconds) {
		DateFull output = {};
		const Date date = get_calendar_time(nanoseconds);
		const u64 totalSeconds = nanoseconds / 1000000000ULL;
		const u32 secondsInDay = (u32)(totalSeconds % 86400ULL);
		const u32 nanosecondsInSecond = (u32)(nanoseconds % 1000000000ULL);
		const u32 hoursInDay = (u32)(((u64)secondsInDay * 1193047U) >> 32U);
		const u32 minutesInDay = (u32)(((u64)secondsInDay * 71582789U) >> 32U);
		const u32 milliseconds = nanosecondsInSecond / 1000000U;
		const u32 microsecondsInSecond = nanosecondsInSecond / 1000U;
		output.year = date.year;
		output.month = date.month;
		output.day = date.day;
		output.hour = (u8)hoursInDay;
		output.minute = (u8)((minutesInDay + hoursInDay * 4U) & 63U);
		output.second = (u8)((secondsInDay + minutesInDay * 4U) & 63U);
		output.millisecond = (u16)milliseconds;
		output.microsecond = (u16)(microsecondsInSecond - milliseconds * 1000U);
		output.nanosecond = (u16)(nanosecondsInSecond - microsecondsInSecond * 1000U);
		return output;
	}
	// ATTR(static_inl, const)
	// TimeOfDay get_calendar_time_full(u64 nanoseconds) {
	// 	TimeOfDay output = {};
	// 	const u64 totalSeconds = nanoseconds / 1000000000ULL;
	// 	const u32 secondsInDay = (u32)(totalSeconds % 86400ULL);
	// 	const u32 nanosecondsInSecond = (u32)(nanoseconds % 1000000000ULL);
	// 	const u32 hoursInDay = (u32)(((u64)secondsInDay * 1193047U) >> 32U);
	// 	const u32 minutesInDay = (u32)(((u64)secondsInDay * 71582789U) >> 32U);
	// 	const u32 milliseconds = nanosecondsInSecond / 1000000U;
	// 	const u32 microsecondsInSecond = nanosecondsInSecond / 1000U;
	// 	output.hours = (u8)hoursInDay;
	// 	output.minutes = (u8)((minutesInDay + hoursInDay * 4U) & 63U);
	// 	output.seconds = (u8)((secondsInDay + minutesInDay * 4U) & 63U);
	// 	output.milliseconds = (u16)milliseconds;
	// 	output.microseconds = (u16)(microsecondsInSecond - milliseconds * 1000U);
	// 	output.nanoseconds = (u16)(nanosecondsInSecond - microsecondsInSecond * 1000U);
	// 	return output;
	// }

// ==== RDTSC Handling ========================================================
	ATTR(static_inl, flatten)
	void measure_clock(u64& curTscTick, u64& curTime) {
		struct timespec timeNow;
		clock_gettime(CLOCK_MONOTONIC_RAW, &timeNow);
		u64 t1 = get_tsc_gated();
		clock_gettime(CLOCK_MONOTONIC_RAW, &timeNow);
		u64 t2 = get_tsc_gated();
		curTscTick = t1 + (t2 - t1) / 2;
		curTime = (u64)timeNow.tv_sec * 1_G + (u64)timeNow.tv_nsec;
	}

	// REVIEW: Is it better to calibrate against first measurement or last
	ATTR(static_inl, flatten)
	void calibrate() {
		u64 curTscTick, curTime;
		measure_clock(curTscTick, curTime);
		u64 deltaTsc = curTscTick - lastTscTick;
		u64 deltaTime = curTime - lastTime;
		lastTscTick = curTscTick;	// Updates the values for future calibrations
		lastTime = curTime;
		tscFactor = (u64)(((u128)deltaTime << 64) / deltaTsc);
	}

	ATTR(static_inl)
	u64 identify_tsc_freq() {
		uint eax, ebx;		// EAX, EBX define the ratio between TSC and crystal
		uint ecx, edx;		// TSC = EBX / EAX * ECX, where ECX is the crystal frequency
		uint maxLeaf = __get_cpuid_max(0, NULL);
		u64 tscFreq = 0;	// 10^9 hz order of magnitude, so GHz

		// leaf 0x15: TSC/core-crystal clock ratio
		if (maxLeaf >= 0x15 && __get_cpuid_count(0x15, 0, &eax, &ebx, &ecx, &edx)) {
			if (eax != 0 && ebx != 0 && ecx != 0)
				tscFreq = ((u64)ecx * ebx) / eax;
		}

		return tscFreq;
	}

	ATTR(static_inl, constructor)
	void init() {
		measure_clock(firstTscTick, firstTime);
		lastTscTick = firstTscTick;
		lastTime = firstTime;
		u64 tscFreq = TSC_FREQUENCY == 0 ? identify_tsc_freq() : TSC_FREQUENCY;
		if (tscFreq != 0) {
			tscFactor = freq_to_ns_cycles(tscFreq);
			return;
		}
		timespec wait{.tv_sec = 0, .tv_nsec = 50000};
		nanosleep(&wait, nullptr);
		calibrate();
	}

	ATTR(static_inl) constexpr
	u64 freq_to_ns_cycles(u64 tscFreq) {
		return (u64)(((u128)1_G << 64) / tscFreq);
	}

	ATTR(static_inl)
	u64 tsc_to_ns(u64 tscTicks) {
		return (u64)((u128)tscTicks * tscFactor >> 64);
	}

	ATTR(static_inl)
	u64 tsc_delta_to_ns(u64 tscTick1, u64 tscTick2) {
		u64 tscDelta = tscTick2 - tscTick1;
		return (u64)((u128)tscDelta * tscFactor >> 64);
	}

	ATTR(static_inl)
	u64 get_tsc() {
		return __rdtsc();
	}

	ATTR(static_inl)
	u64 get_tsc_gated() {
		_mm_lfence();
		u64 tscTicks = __rdtsc();
		_mm_lfence();
		return tscTicks;
	}
};
