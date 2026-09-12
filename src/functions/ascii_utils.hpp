#pragma once
#include "core.hpp"
#include "webserv.hpp"
#include "Span.hpp"

namespace fn {

bool strcasecmp16(const char* string, const char* ref, usize length) {
	(void) length;
	u128 buffer[2];
	u8* bufPtr = (u8*) buffer;
	const u128 tmp = (u128) 0x2020202020202020UL;
	const u128 orMask = (tmp << 64) | (tmp);

	MEMCPY_INLINE(bufPtr, string, 16);
	buffer[0] |= orMask;
	return MEMCMP(bufPtr, ref, 16) == 0;
}

}