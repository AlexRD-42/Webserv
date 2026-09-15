#include <cpuid.h>
#include <ctime>
#include <x86intrin.h>
#include <unistd.h>
#include <iostream>

#include "core.hpp"
#include "HWClock.hpp"

int main() {
	// std::cout << test;
	HWTimer::hard_calibrate(10, 1'000'000);
	std::cout << "Error: <" << HWTimer::measure_current_error() << ">\n";
	for (usize i = 0; i < 10; i++) {
		timespec wait{.tv_sec = 1, .tv_nsec = 0};
		nanosleep(&wait, nullptr);
		// fn::log();
		std::cout << "Error(1): <" << HWTimer::measure_current_error(4096) << ">\n";
		// HWClock::calibrate();
		// std::cout << "Error(2): <" << HWClock::measure_current_error() << ">\n";
	}
}

// int main() {
// 	struct timespec timeNow;
// 	u64 samples[NUM_SAMPLES][2];

// 	for (usize i = 0; i < NUM_SAMPLES; i++) {
// 		clock_gettime(CLOCK_MONOTONIC_RAW, &timeNow);
// 		u64 t1 = get_tsc_gated();
// 		clock_gettime(CLOCK_MONOTONIC_RAW, &timeNow);
// 		u64 t2 = get_tsc_gated();
// 		samples[i][0] = (t1 + t2) / 2;
// 		samples[i][1] = (u64)timeNow.tv_sec * 1_G + (u64)timeNow.tv_nsec;
// 		timespec wait{.tv_sec = 0, .tv_nsec = NS_WAIT};
// 		nanosleep(&wait, nullptr);
// 	}

// 	double ratioAvg = 0;
// 	for (usize i = 1; i < NUM_SAMPLES; i++) {
// 		u64 deltaTSC = samples[i][0] - samples[i - 1][0];
// 		u64 deltaTime = samples[i][1] - samples[i - 1][1];
// 		double ratio = (double)deltaTSC / (double)deltaTime;
// 		ratioAvg += ratio;
// 		std::cout << "<TSC: " << deltaTSC << "> <TIME: " << deltaTime << "> <RATIO: " << ratio << ">\n";
// 	}
// 	ratioAvg /= NUM_SAMPLES - 1;
// 	std::cout << "RATIO AVERAGE: " << ratioAvg << "\n";
// 	return 0;
// }
