#pragma once

#include "core.hpp"
#include "Arena.hpp"
#include "Span.hpp"

namespace fn {

// Important: Assumes padding of at least 4 bytes
ATTR(static_inl, pure)
usize s_normalize_target(u8* str, usize length) {
	u8* end = str + length;
	u8* readPtr = str;
	u8* writePtr = str;

	if (*str != '/')
		return SIZE_MAX;
	while (readPtr < end) {
		u8 value = *readPtr++;
		if (value == '%') {
			if (g_asciiLut[readPtr[0]] > ASCII_HEX)
				return SIZE_MAX;
			if (g_asciiLut[readPtr[1]] > ASCII_HEX)
				return SIZE_MAX;
			value = (g_asciiLut[readPtr[0]] * 16 + g_asciiLut[readPtr[1]]);
			readPtr += 2;
		}
		if (g_asciiLut[value] > ASCII_RFC_SYMBOLS)
			return SIZE_MAX;
		*writePtr++ = value;
	}
	*writePtr = '\0';
	return (usize)(writePtr - str);
}

// /path/to/something/../this
ATTR(static_inl, pure)
usize canonicalize_target_inplace(u8* str, usize length) {
	usize newLength = s_normalize_target(str, length);
	if (newLength == SIZE_MAX)
		return SIZE_MAX;

	u8* end = str + newLength;
	while (str < end) {
		if (LITCMP(str, "/../") == 0 || LITCMP(str, "/..\0") == 0)	// Reject .. fuckery
			return SIZE_MAX;
		if (LITCMP(str, "/./") == 0 || LITCMP(str, "/.\0") == 0)
			return SIZE_MAX;
		if (LITCMP(str, "//") == 0)
			return SIZE_MAX;
		if (*str == '%')
			return SIZE_MAX;
		str++;
	}
	return newLength;
}

}
