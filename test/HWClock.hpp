#pragma once
#include <cpuid.h>
#include <ctime>
#include <x86intrin.h>
#include <unistd.h>

#include "core.hpp"
#include "core_utils.hpp"

/*	Hardware Clock using RDTSC:

	Uses 64 bits for the fractional component, and computes the scale to ns
	This can overflow if tsc_freq is lower than GIGA. Very few architectures
	qualify, and by shifting it with 64 clang compiles this to a perfect and beautiful:
		mov (rdx, rsi);    mulx (rax, rax, rdi);    ret

	TODO: 
		1) Make the clock thread friendly
		2) Improve calibration (initial calibration is not very good, with 1ms calibration 
			deviates around 1000us every second. I don't really want to increase the budget
			because it's a forced calibration that delays other tasks)
		3) Separate formatting from date collection
		4) CLOCK_MONOTONIC_RAW is not UNIX time, CLOCK_REALTIME is
============================================================================= */
// If 0, it will run CPUID, and if that fails, calibration
#define TSC_FREQUENCY 0
// #define TSC_FACTOR 0
// #define TSC_FACTOR 3932232912437957575
#define TSC_FACTOR 3932232912438853884	// t=2400s More accurate
STATIC_ASSERT(TSC_FREQUENCY == 0 || TSC_FREQUENCY > 1_G);

struct HWTimer {
	static_inl u64 unixTime = 0;
	static_inl u64 calTscTick = 0, calTime = 0;
	static_inl u64 lastTscTick = 0, lastTime = 0;
	static_inl u64 tscFactor = 0;
	static inline const u8 months[12][10] = {	// TODO: USE, or maybe doesnt belong in class
		"January", "February", "March", "April", "May", "June", "July",
		"August", "September", "October", "November","December"
	};
	u64 timeNow;

	ATTR(inl) // Get absolute time cant use tsc_to_ns directly
	u64 update_clock() {
		u64 tscTick = get_tsc();
		timeNow = calTime + tsc_to_ns(tscTick - calTscTick);
		return timeNow;
	}

	ATTR(inl)
	u64 time_elapsed() {
		return timeNow - calTime;
	}

	struct Date {
		u16 year;
		u8 month, day;
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

	struct DateFull {
		u32 year;
		u8 month, week, day, hour, minute, second;
		u16 millisecond, microsecond, nanosecond;
	};

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

// ==== RDTSC Handling ========================================================
	ATTR(static_inl, flatten)
	void measure_clock(u64& curTscTick, u64& curTime, usize numSamples = 8) {
		u64 bestWidth = UINT64_MAX;
		timespec timeNow;
		clock_gettime(CLOCK_MONOTONIC_RAW, &timeNow); // Warmup

		for (usize i = 0; i < numSamples; i++) {
			u64 t1 = get_tsc_gated();
			clock_gettime(CLOCK_MONOTONIC_RAW, &timeNow);
			u64 t2 = get_tsc_gated();
			u64 width = t2 - t1;

			if (width < bestWidth) {
				bestWidth = width;
				curTscTick = t1 + width / 2;
				curTime = (u64)timeNow.tv_sec * 1_G + (u64)timeNow.tv_nsec;
			}
		}
	}

	ATTR(static_inl, flatten)
	isize measure_current_error(usize numSamples = 8) {
		u64 curTscTick, curTime;
		measure_clock(curTscTick, curTime, numSamples);
		u64 tscTimeElapsed = tsc_to_ns(curTscTick - calTscTick);
		u64 unixTimeElapsed = curTime - calTime;
		return (isize)tscTimeElapsed - (isize)unixTimeElapsed;
	}

	// REVIEW: Is it better to calibrate against first measurement or last
	ATTR(static_inl, flatten)
	void calibrate(usize nsDelay = 0, usize numSamples = 8) {
		if (nsDelay > 0) {
			timespec wait{.tv_sec = 0, .tv_nsec = (long)nsDelay};
			nanosleep(&wait, nullptr);
		}
		u64 curTscTick, curTime;
		measure_clock(curTscTick, curTime, numSamples);
		u64 deltaTsc = curTscTick - calTscTick;
		u64 deltaTime = curTime - calTime;
		lastTscTick = curTscTick;	// Updates the values for future calibrations
		lastTime = curTime;
		tscFactor = (u64)(((u128)deltaTime << 64) / deltaTsc);
	}

	ATTR(static_inl, flatten)
	u64 hard_calibrate(usize secDelay, usize numSamples) {
		measure_clock(calTscTick, calTime, numSamples);
		timespec wait{.tv_sec = (long)secDelay, .tv_nsec = 0};
		nanosleep(&wait, nullptr);
		u64 curTscTick, curTime;
		measure_clock(curTscTick, curTime, numSamples);
		u64 deltaTsc = curTscTick - calTscTick;
		u64 deltaTime = curTime - calTime;
		lastTscTick = curTscTick;	// Updates the values for future calibrations
		lastTime = curTime;
		tscFactor = (u64)(((u128)deltaTime << 64) / deltaTsc);
		return tscFactor;
	}

	ATTR(static_inl, constructor)
	void init() {
		fn::log("Init Start");
		measure_clock(calTscTick, calTime, 4096);	// 4096 iterations takes around 50 microseconds
		lastTscTick = calTscTick;					// Important for the first measurement to be accurate
		lastTime = calTime;
		fn::log("Init End");
		if (TSC_FACTOR != 0) {
			tscFactor = TSC_FACTOR;
			return;
		}
		u64 tscFreq = TSC_FREQUENCY == 0 ? identify_tsc_freq() : TSC_FREQUENCY;
		if (tscFreq != 0) {
			tscFactor = freq_to_ns_cycles(tscFreq);
			return;
		}
		calibrate(1'000'000, 4096);
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
};	// ==== HWClock End =======================================================
