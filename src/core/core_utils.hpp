#pragma once
#include <unistd.h>
#include <ctime>

#include "core.hpp"

namespace fn {

static u64 lastTime = 0;
static u64 firstTime = 0;

u64 get_ns() {
	timespec timeNow;
	clock_gettime(CLOCK_MONOTONIC_RAW, &timeNow);
	return (u64)timeNow.tv_sec * 1_G + (u64)timeNow.tv_nsec;		
}

ATTR(static_inl, constructor)
void log_init_run_once() {
	lastTime = get_ns();
	firstTime = lastTime;
}

void log(const char *str = nullptr) {
	const u64 curTime = get_ns();
	const u64 delta   = curTime - lastTime;
	const u64 elapsed = curTime - firstTime;
	lastTime = curTime;
	
	constexpr usize numberWidth = 12;
	char buf[256];
	char *p = buf;

	// this is so cool
	auto put_u64 = [&](u64 v) {
		char tmp[32];
		char *end = ARRAY_END(tmp);
		char *q   = end;

		const bool pad = v < 1'000'000'000'000ull;

		uint digits = 0;
		do {
			if (digits && digits % 3 == 0)
				*--q = '\'';

			*--q = '0' + v % 10;
			v /= 10;
			++digits;
		} while (v || (pad && digits < 12));

		const usize len = end - q;
		p = (char *)MEMCPY(p, q, len) + len;
	};

	if (str) {
		const usize len = STRLEN(str);
		p = (char *)MEMCPY(p, str, len) + len;
		*p++ = ' ';
	}

	p = LITAPPEND(p, "\t<t=");
	put_u64(elapsed);

	p = LITAPPEND(p, ", dt=");
	put_u64(delta);

	p = LITAPPEND(p, ">\n");

	write(STDOUT_FILENO, buf, (usize)(p - buf));
}

}
