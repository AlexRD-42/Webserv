#pragma once
#include <emmintrin.h>

#include "core.hpp"
#include "webserv.hpp"
#include "Span.hpp"
namespace fn {

ATTR(static_inl, pure)
bool q16strcasecmp(const char* str, const char* ref, usize length) {
	ENFORCE(length <= 16, "q16strcasecmp length higher than 16");

	const __m128i caseMask = _mm_set1_epi8(0x20);
	__m128i lowerStr = _mm_or_si128(_mm_loadu_si128((const __m128i*)str) , caseMask);
	__m128i lowerRef = _mm_loadu_si128((const __m128i*)ref);
	__m128i eq = _mm_cmpeq_epi8(lowerRef, lowerStr);
	usize mask = ~((usize)_mm_movemask_epi8(eq));
	usize matchIndex = (usize)CTZ(mask);
	return matchIndex >= length;
}

}
