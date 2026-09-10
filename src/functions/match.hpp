#pragma once
#include "core.hpp"
#include "webserv.hpp"
#include "Span.hpp"

/* (IMPORTANT) Field matching presumes 24 byte padding
This function performs a 24 byte load of a field delimited by : then compares
against a table of reference strings to find a match. Because MEMCMP length is
fixed, the compiler automatically vectorizes the comparison

Returns: 0 on no matches or
		index associated with the string compared

TODO:	Finding can be two operations, Setting or can be one operation
		Move table to init, Automate the creation of the enums from the table
*/

namespace fn {
// 
ATTR(static_inl, pure)
Span find_dot(Span span) {
	char* const end = span.ptr + span.size;
	Span ext = {end, 0};

	for (char* ptr = span.ptr; ptr < end; ptr++) {
		if (*ptr == '/') {
			ext.ptr = end;
			ext.size = 0;
		}
		else if (*ptr == '.') {
			ext.ptr = ptr + 1;
			ext.size = (usize)(end - ext.ptr);
		}
	}
	return ext;
}

template <usize count, usize size>
ATTR(static_inl, pure)
isize s_match(const u8* ptr, usize length, const u8 (&ltable)[count][size]) {
	u8 buffer[size * 2];

	length = length >= size ? 0 : length;
	MEMCPY_INLINE(buffer, ptr, size);
	for (usize i = 0; i < size; i++)
		buffer[i] |= 32;
	MEMSET_INLINE(buffer + length, 0, size);

	for (usize i = 0; i < count; i++) {
		if (MEMCMP(ltable[i], buffer, size) == 0)
			return (isize)i + 1;
	}
	return 0;
}

ATTR(static_inl, pure, flatten)
isize match_field(Span field) {
	static const u8 ltable[][24] = FIELD_TABLE;

	return s_match((u8*)field.ptr, field.size, ltable);
}

ATTR(static_inl, pure, flatten)
isize match_mime(Span target) {
	static const u8 ltable[][8] = MIME_TABLE;
	Span ext = find_dot(target);

	return s_match((u8*)ext.ptr, ext.size, ltable);
}
}

// static inline
// Span find_dot(Span span) {
// 	u8 tmp[2];
// 	char* dotPos = NULL;
// 	Span ext = {span.ptr, 0};
// 	char* end = span.ptr + span.size;
// 	char* padEnd = span.ptr + span.size + 2;

// 	MEMCPY_INLINE(tmp, padEnd, 2);
// 	MEMCPY_INLINE(padEnd, "/.", 2);		// TODO: needs to find app.min.js for example

// 	while (true) {
// 		while (*ext.ptr != '/')
// 			ext.ptr++;
// 		while (*ext.ptr != '.')
// 			ext.ptr++;
// 		if (ext.ptr >= padEnd)
// 			break;
// 		dotPos = ext.ptr++;
// 	}
// 	MEMCPY_INLINE(padEnd, tmp, 2);
// 	ext.size = (usize)(end - dotPos);
// 	return ext;
// }

// static inline
// Span find_dot(Span span) {
// 	char tmp = span.ptr[0];
// 	span.ptr[0] = '.';

// 	char* end = span.ptr + span.size;
// 	Span ext = {span.ptr + span.size, 0};
// 	while (*ext.ptr != '.')
// 		ext.ptr--;
// 	if (span.ptr == ext.ptr && tmp == '.')
// 		return span;
// 	span.ptr[0] = tmp;
// 	ext.size = (usize)(end - ext.ptr);
// 	ext.ptr++;
// 	return ext;
// }
