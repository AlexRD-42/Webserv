#pragma once

#include "core.hpp"
#include "Arena.hpp"
#include "Span.hpp"

namespace fn {

ATTR(static_inl)
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
		if (g_asciiLut[value] > ASCII_RFC_SYMBOLS || value == '%')	// Reject % fuckery
			return SIZE_MAX;
		*writePtr++ = value;
	}
	*writePtr = '\0';
	return (usize)(writePtr - str);
}

ATTR(static_inl)
usize decode_nop_inplace(u8* str, usize length) {
	u8* end = str + length;
	u8* readPtr = str;
	u8* writePtr = str;

	while (readPtr < end) {
		if (LITCMP(readPtr, "/./") == 0)
			readPtr += 2;
		else if (LITCMP(readPtr, "//") == 0)
			readPtr++;
		*writePtr++ = *readPtr++;
	}
	*writePtr = '\0';
	return (usize)(writePtr - str);
}

ATTR(static_inl)
usize decode_escdot_inplace(u8* str, usize length) {
	u8* end = str + length;
	u8* readPtr = str;
	u8* writePtr = str;

	while (readPtr < end) {
		if (LITCMP(readPtr, "/./") == 0)
			readPtr += 2;
		else if (LITCMP(readPtr, "//") == 0)
			readPtr++;
		*writePtr++ = *readPtr++;
	}
	*writePtr = '\0';
	return (usize)(writePtr - str);
}

// This needs to search and validate for query as well
// Ideally, should be /path/to/something\0QUERY=this
// TODO: This needs to guarantee that there is no \r\n in the query
// /path/to/something/../this
ATTR(static_inl)
usize canonicalize_target_inplace(u8* str, usize length) {
	usize newLength = decode_percent_inplace(str, length);
	if (newLength == SIZE_MAX || *str != '/')
		return SIZE_MAX;
	u8* end = str + newLength;
	while (str < end) {
		if (LITCMP(str, "/.") != 0) {	// Reject .. fuckery
			str++;
			continue;
		}
		str += 2 + (str[2] == '.');
		if (*str == '/' | *str == '\0')
			return SIZE_MAX;
		str++;
	}
	return newLength;
}
}
