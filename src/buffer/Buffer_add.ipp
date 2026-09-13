#pragma once
#include "Buffer.hpp"

BUFFER_INL
(char*) prepend(const Span& span) {
	readPos -= span.size;
	MEMCPY(data + readPos, span.ptr, span.size);
	return (char*)data + readPos;
}

BUFFER_INL
(char*) prepend(const char* ptr, usize length) {
	readPos -= length;
	MEMCPY(data + readPos, ptr, length);
	return (char*)data + readPos;
}

BUFFER_INL_T
(usize N, char*) prepend(const char (&string)[N]) {
	readPos -= (N - 1);
	MEMCPY_INLINE(data + readPos, string, (N - 1));
	return (char*)data + readPos;
}

BUFFER_INL_T
(usize N, char*) prepend_inline(const char* ptr, usize length) {
	readPos -= length;
	MEMCPY_INLINE(data + readPos, ptr, N);
	return (char*)data + readPos;
}

BUFFER_INL
(char*) append(const char* ptr, usize length) {
	char* optr = (char*)data + writePos;
	MEMCPY(optr, ptr, length);
	writePos += length;
	return optr;
}

BUFFER_INL
(char*) append(const Span& span) {
	char* optr = (char*)data + writePos;
	MEMCPY(optr, span.ptr, span.size);
	writePos += span.size;
	return optr;
}

BUFFER_INL_T
(usize N, char*) append(const char (&string)[N]) {
	char* optr = (char*)data + writePos;
	MEMCPY_INLINE(optr, string, N);	// Copies null terminator, advances past content only
	writePos += N - 1;
	return optr;
}

BUFFER_INL_T
(usize N, char*) append_inline(const char* ptr, usize length) {
	char* optr = (char*)data + writePos;
	MEMCPY_INLINE(optr, ptr, N);
	writePos += length;
	return optr;
}

BUFFER_INL
(char*) memset(u8 byte, usize length) {
	char* optr = (char*)data + writePos;
	MEMSET(optr, byte, length);
	writePos += length;
	return optr;
}

BUFFER_INL_T
(usize N, char*) memset_inline(u8 byte, usize length) {
	char* optr = (char*)data + writePos;
	MEMSET_INLINE(optr, byte, N);
	writePos += length;
	return optr;
}
