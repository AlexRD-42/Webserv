#pragma once
#include "Buffer.hpp"

BUFFER_INL
(char*) append_mime(u8 mimeIndex) {
	static const u8 mimeStrings[][32] = MIME_STRINGS;

	const u8* str = mimeStrings[mimeIndex];
	const usize length = *str;
	char* optr = (char*)data + writePos;
	MEMCPY_INLINE(optr, str + 1, 24);
	writePos += length;
	return optr;
}

BUFFER_INL
(char*) append_digit10(usize number) {
	const usize maxLength = 24;
	char buffer[maxLength * 2];
	Span digit = fn::itoa10(number, buffer, maxLength);

	char* optr = (char*)data + writePos;
	MEMCPY_INLINE(optr, digit.ptr, maxLength);
	writePos += digit.size;
	return optr;
}

BUFFER_INL
(char*) append_digit16(usize number) {
	const usize maxLength = 16;
	char buffer[maxLength * 2];
	Span digit = fn::itoa16(number, buffer, maxLength);

	char* optr = (char*)data + writePos;
	MEMCPY_INLINE(optr, digit.ptr, maxLength);
	writePos += digit.size;
	return optr;
}

BUFFER_INL
(char*) append_url_component(const char* ptr, usize length) {
	static const u8 hex[] = "0123456789ABCDEF";
	static u8 lut[2][4] = {{0, 0, 0, 1}, {'%', 0, 0, 3}};
	char* optr = (char*)data + writePos;

	for (usize index = 0; index < length; index++) {
		const u8 value = (u8)ptr[index];
		const u8 lutIndex = g_asciiLut[value] > ASCII_URL_VALID;
		lut[0][0] = value;
		lut[1][1] = hex[value >> 4];
		lut[1][2] = hex[value & 15];
		append_inline<3>((char*)lut[lutIndex], lut[lutIndex][3]);	// there has to be a better way to index the length,
															// given that it is only two possible states
	}
	return optr;
}

BUFFER_INL
(char*) append_html(char* ptr, usize length) {
	u8 lengthLut[6] = {5, 5, 6, 4, 4, 1};
	static char strLut[6][8] = {"&amp;", "&#39;", "&quot;", "&lt;", "&gt;", "\0"};
	char* optr = (char*)data + writePos;

	for (usize index = 0; index < length; index++) {
		u8 asciiLutIndex = g_asciiLut[(u8)ptr[index]] - ASCII_HTML_VALID;
		u8 strLutIndex = MIN(5, asciiLutIndex);
		strLut[5][0] = ptr[index];
		append_inline<6>(strLut[strLutIndex], lengthLut[strLutIndex]);	// Up to 8 bytes overflow is safe
	}
	return optr;
}

// <a href="filename[256]">filename[64]</a>    02-Dec-2004 18:46    241476

BUFFER_INL
(usize) append_entry(int directoryFd, char* name) {
	Span entry = {name, STRLEN(name)};

	struct stat st;
	if (LITCMP(entry.ptr, ".\0") == 0 || LITCMP(entry.ptr, "..\0") == 0)
		return 0;
	if (fstatat(directoryFd, name, &st, 0)) {
		append(HTTP_INDEX_PERMISSION);
		return sizeof(HTTP_INDEX_PERMISSION) - 1;
	}

	usize fileSize = S_ISDIR(st.st_mode) ? 0 : (usize) st.st_size;
	char buf[32];
	Clock::format_time(&st.st_mtim, buf);

	// 0 visible, 776 bytes (11 + 3 * 255)
	char* start = append("<a href=\"");
	append_url_component(entry.ptr, entry.size);
	append("\">");

	// 52 visible, 297 bytes (3 + (52 - 3) * 6 bytes) (52 = HTTP_INDEX_NAME_LENGTH)
	if (entry.size >= HTTP_INDEX_NAME_LENGTH) {
		append_html(entry.ptr, HTTP_INDEX_NAME_LENGTH - 3);
		append("...");
	}
	else {
		append_html(entry.ptr, entry.size);
		memset(' ', HTTP_INDEX_NAME_LENGTH - entry.size);
	}

	// 48 visible, 53 bytes
	append("</a>");
	memset(' ', 8);
	append_inline<17>(buf, 17);
	memset(' ', 4);
	append_digit10(fileSize);
	append("\n");
	return (usize)(wptr() - start);
}
