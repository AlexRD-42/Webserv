#pragma once
#include "core.hpp"

// === Generic Helpers =====================================
#define SWAP(a, b) 			({__typeof__(a) _swap_tmp_ = (a); (a) = (b); (b) = _swap_tmp_; (void)0;})
#define ALIGN_UP(x, a)		(((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x, a)	((x) & ~((a) - 1))
#define IS_POW2(x)			((x) != 0 && ((x) & ((x) - 1)) == 0)
#define NEXT_POW2(x)		((__typeof__(x))((usize)1 << ((sizeof(x) * 8) - (usize)CLZ(x))))

// === Bit Helpers =========================================
// TODO: Get ranged versions, like start, end
#define BITREAD(word, index)	(((word) >> (index)) & 1)
#define BITSET(word, index)		((word) |= ((__typeof__(word))1 << (index)))
#define BITFLIP(word, index)	((word) ^= ((__typeof__(word))1 << (index)))
#define BITCLR(word, index)		((word) &= ~((__typeof__(word))1 << (index)))

// === MINMAX Helpers ========================================
#define MIN(x, y)			((x) < (y) ? (x) : (y))
#define MAX(x, y)			((x) > (y) ? (x) : (y))
#define ABS(x)				((x) > 0 ? (x) : -(x))
#define CLAMP(x, low, high)	MAX(low, MIN(x, high))

#define ABSMAX(x, y)		MAX(ABS(x), ABS(y))
#define ABSMIN(x, y)		MIN(ABS(x), ABS(y))
#define ABSDIFF(x, y)		(MAX(x, y) - MIN(x, y))

#define MIN3(x, y, z)		MIN(x, MIN(y, z))
#define MAX3(x, y, z)		MAX(x, MAX(y, z))
#define MIN4(x, y, z, w)	MIN(MIN(x, y), MIN(z, w))
#define MAX4(x, y, z, w)	MAX(MAX(x, y), MAX(z, w))

#define ABSMIN3(x, y, z)	MIN3(ABS(x), ABS(y), ABS(z))
#define ABSMAX3(x, y, z)	MAX3(ABS(x), ABS(y), ABS(z))
#define ABSMIN4(x, y, z, w)	MIN4(ABS(x), ABS(y), ABS(z), ABS(w))
#define ABSMAX4(x, y, z, w)	MAX4(ABS(x), ABS(y), ABS(z), ABS(w))

// === ASCII Helpers =======================================
#define IS_ASCII(c) ((c) >= 0 && (c) < 128)
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_UPPER(c) ((c) >= 'A' && (c) <= 'Z')
#define IS_LOWER(c) ((c) >= 'a' && (c) <= 'z')
#define IS_ALPHA(c) (IS_LOWER(c | 32))
// #define IS_SPACE(c) (((c) == ' ') || ((unsigned char)(c) - (unsigned char)'\t' < 5u))	// LUTLESS version
#define IS_SPACE(c)	(g_asciiLut[((unsigned char)(c))] == ASCII_SPACE)
#define IS_HEX(c)	(g_asciiLut[((unsigned char)(c))] <= ASCII_HEX)
#define IS_ALNUM(c) (g_asciiLut[((unsigned char)(c))] <= ASCII_LETTERS)
#define IS_IDENT(c)	(g_asciiLut[((unsigned char)(c))] <= ASCII_IDENT)
