#include <cpuid.h>
#include <ctime>
#include <x86intrin.h>
#include <unistd.h>
#include <iostream>

#include "core.hpp"
// #include "rdtsc.hpp"


ATTR(static_inl)
u64 get_tsc_gated() {
	_mm_lfence();
	u64 tscTicks = __rdtsc();
	_mm_lfence();
	return tscTicks;
}

#define NUM_SAMPLES 2
#define NS_WAIT 50000
int main() {
	struct timespec timeNow;
	u64 samples[NUM_SAMPLES][2];

	for (usize i = 0; i < NUM_SAMPLES; i++) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &timeNow);
		u64 t1 = get_tsc_gated();
		clock_gettime(CLOCK_MONOTONIC_RAW, &timeNow);
		u64 t2 = get_tsc_gated();
		samples[i][0] = (t1 + t2) / 2;
		samples[i][1] = (u64)timeNow.tv_sec * 1_G + (u64)timeNow.tv_nsec;
		timespec wait{.tv_sec = 0, .tv_nsec = NS_WAIT};
		nanosleep(&wait, nullptr);
	}

	double ratioAvg = 0;
	for (usize i = 1; i < NUM_SAMPLES; i++) {
		u64 deltaTSC = samples[i][0] - samples[i - 1][0];
		u64 deltaTime = samples[i][1] - samples[i - 1][1];
		double ratio = (double)deltaTSC / (double)deltaTime;
		ratioAvg += ratio;
		std::cout << "<TSC: " << deltaTSC << "> <TIME: " << deltaTime << "> <RATIO: " << ratio << ">\n";
	}
	ratioAvg /= NUM_SAMPLES - 1;
	std::cout << "RATIO AVERAGE: " << ratioAvg << "\n";
	return 0;
}
