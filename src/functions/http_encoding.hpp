#pragma once

#include "core.hpp"
#include "Arena.hpp"
#include "Span.hpp"

namespace fn {

ATTR(static_inl, pure)
usize html_encoded_size(const char* src, usize length) {
	static const u8 growthLut[6] = {4, 4, 5, 3, 3, 0};
	usize result = length;

	for (usize index = 0; index < length; index++) {
		u8 lutIndex = g_asciiLut[(u8)src[index]] - ASCII_HTML_ESCAPE_START;
		lutIndex = MIN(5, lutIndex);
		result += growthLut[lutIndex];
	}
	return result;
}

// Important: Assumes padding of at least 4 bytes
ATTR(static_inl, pure)
usize decode_percent_inplace(u8* str, usize length) {
	u8* end = str + length;
	u8* readPtr = str;
	u8* writePtr = str;

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
	usize newLength = decode_percent_inplace(str, length);
	if (newLength == SIZE_MAX || *str != '/')
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
