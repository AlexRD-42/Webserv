#pragma once
#include "Buffer.hpp"

BUFFER_INL
(Span) prepend(const Span& span) {
	readPos -= span.size;
	MEMCPY(data + readPos, span.ptr, span.size);
	return {(char*)data + readPos, span.size};
}

BUFFER_INL
(Span) prepend(const char* ptr, usize length) {
	readPos -= length;
	MEMCPY(data + readPos, ptr, length);
	return {(char*)data + readPos, length};
}

BUFFER_INL_T
(usize N, Span) prepend(const char (&string)[N]) {
	readPos -= (N - 1);
	MEMCPY_INLINE(data + readPos, string, (N - 1));
	return {(char*)data + readPos, N - 1};
}

BUFFER_INL_T
(usize N, Span) prepend_inline(const char* ptr, usize length) {
	readPos -= length;
	MEMCPY_INLINE(data + readPos, ptr, N);
	return {(char*)data + readPos, length};
}

BUFFER_INL
(Span) append(const char* ptr, usize length) {
	char* optr = (char*)data + writePos;
	MEMCPY(optr, ptr, length);
	writePos += length;
	return {optr, length};
}

BUFFER_INL
(Span) append(const Span& span) {
	char* optr = (char*)data + writePos;
	MEMCPY(optr, span.ptr, span.size);
	writePos += span.size;
	return {optr, span.size};
}

BUFFER_INL_T
(usize N, Span) append(const char (&string)[N]) {
	char* optr = (char*)data + writePos;
	MEMCPY_INLINE(optr, string, N);	// Copies null terminator, advances past content only
	writePos += N - 1;
	return {optr, N - 1};
}

BUFFER_INL_T
(usize N, Span) append_inline(const char* ptr, usize length) {
	char* optr = (char*)data + writePos;
	MEMCPY_INLINE(optr, ptr, N);
	writePos += length;
	return {optr, length};
}

BUFFER_INL
(Span) memset(u8 byte, usize length) {
	char* optr = (char*)data + writePos;
	MEMSET(optr, byte, length);
	writePos += length;
	return {optr, length};
}

BUFFER_INL_T
(usize N, Span) memset_inline(u8 byte, usize length) {
	char* optr = (char*)data + writePos;
	MEMSET_INLINE(optr, byte, N);
	writePos += length;
	return {optr, length};
}
