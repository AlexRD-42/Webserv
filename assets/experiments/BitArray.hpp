#pragma once

#include "core.hpp"

template <usize bitCount>
class BitArray {
private:	// Ghetto static asserts for C++ 98
	typedef char bit_count_must_be_nonzero[(bitCount != 0) ? 1 : -1];

public:
	static const usize metaSize = bitCount / WORD_BITS + (bitCount % WORD_BITS != 0);
	static const usize lengthMask = SIZE_MAX >> ((usize)(0 - bitCount) % WORD_BITS);
	usize bitmap[metaSize];

	void bitset() {
		MEMSET_INLINE(bitmap, 0xFF, sizeof(bitmap));
	}

	void bitset(usize index) {
		bitmap[index / WORD_BITS] |= ((usize)1 << (index % WORD_BITS));
	}

	void bitset(usize bitStart, usize bitEnd) {
		const usize wordStart = bitStart / WORD_BITS;
		const usize wordEnd = (bitEnd - 1) / WORD_BITS;
		const usize startMask = SIZE_MAX << (bitStart % WORD_BITS);
		const usize endMask = SIZE_MAX >> ((usize)(0 - bitEnd) % WORD_BITS);
		const usize diffMask = (size_t)-(wordStart != wordEnd);

		bitmap[wordStart] |= startMask & (endMask | diffMask);
		for (usize word = wordStart + 1; word < wordEnd; word++)
			bitmap[word] = SIZE_MAX;
		bitmap[wordEnd] |= endMask & diffMask;
	}

	void bitclr() {
		MEMSET_INLINE(bitmap, 0, sizeof(bitmap));
		bitmap[metaSize - 1] = ~lengthMask;
	}

	void bitclr(usize index) {
		bitmap[index / WORD_BITS] &= ~((usize)1 << (index % WORD_BITS));
	}

	void bitclr(usize bitStart, usize bitEnd)	{
		const usize wordStart = bitStart / WORD_BITS;
		const usize wordEnd = (bitEnd - 1) / WORD_BITS;
		const usize startMask = SIZE_MAX << (bitStart % WORD_BITS);
		const usize endMask = SIZE_MAX >> ((usize)(0 - bitEnd) % WORD_BITS);
		const usize diffMask = (size_t)-(wordStart != wordEnd);

		bitmap[wordStart] &= ~(startMask & (endMask | diffMask));
		for (usize word = wordStart + 1; word < wordEnd; word++)
			bitmap[word] = 0;
		bitmap[wordEnd] &= ~(endMask & diffMask);
	}

	void bitflip() {
		for (usize i = 0; i < metaSize; i++)
			bitmap[i] = ~bitmap[i];
		bitmap[metaSize - 1] |= ~lengthMask;
	}

	void bitflip(usize index) {
		bitmap[index / WORD_BITS] ^= ((usize)1 << (index % WORD_BITS));
	}

	void bitflip(usize bitStart, usize bitEnd) {
		const usize wordStart = bitStart / WORD_BITS;
		const usize wordEnd = (bitEnd - 1) / WORD_BITS;
		const usize startMask = SIZE_MAX << (bitStart % WORD_BITS);
		const usize endMask = SIZE_MAX >> ((usize)(0 - bitEnd) % WORD_BITS);
		const usize diffMask = (size_t)-(wordStart != wordEnd);

		bitmap[wordStart] ^= startMask & (endMask | diffMask);
		for (usize i = wordStart + 1; i < wordEnd; i++)
			bitmap[i] = ~bitmap[i];
		bitmap[wordEnd] ^= endMask & diffMask;
	}

	void bitwrite(usize index, bool bit) {
		usize& word = bitmap[index / WORD_BITS];
		const usize mask = ((usize)1 << (index % WORD_BITS));
		word = (word & ~mask) | ((usize)-(usize)bit & mask);
	}

	bool bitread(usize index) const {
		return (bitmap[index / WORD_BITS] & ((usize)1 << (index % WORD_BITS))) != 0;
	}

	usize bitfind(usize bitStart, usize bitEnd, bool bit) const {
		const usize invert = (usize)-(usize)!bit;
		const usize last = (bitEnd - 1) / WORD_BITS;
		const usize endMask = SIZE_MAX >> ((usize)(0 - bitEnd) % WORD_BITS);
		usize wordIndex = bitStart / WORD_BITS;
		usize candidate = (bitmap[wordIndex] ^ invert) & (SIZE_MAX << (bitStart % WORD_BITS));
	
		if (wordIndex == last)
			candidate &= endMask;
		while (true) {
			if (candidate != 0)
				return wordIndex * WORD_BITS + (usize)CTZ(candidate);
			if (wordIndex == last)
				return SIZE_MAX;
			candidate = bitmap[++wordIndex] ^ invert;
			if (wordIndex == last)
				candidate &= endMask;
		}
	}

	usize find_first_clear() const {
		for (usize word = 0; word < metaSize; word++) {
			const usize candidate = ~bitmap[word];
			if (candidate != 0)
				return word * WORD_BITS + (usize)CTZ(candidate);
		}
		return SIZE_MAX;
	}

	usize find_first_set() const {
		for (usize word = 0; word + 1 < metaSize; word++) {
			if (bitmap[word] != 0)
				return word * WORD_BITS + (usize)CTZ(bitmap[word]);
		}
		const usize candidate = bitmap[metaSize - 1] & lengthMask;
		return candidate != 0 ? (metaSize - 1) * WORD_BITS + (usize)CTZ(candidate) : SIZE_MAX;
	}

	usize count() const {
		usize result = 0;
		for (usize i = 0; i + 1 < metaSize; i++)
			result += POPCOUNT(bitmap[i]);
		result += POPCOUNT(bitmap[metaSize - 1] & lengthMask);
		return result;
	}

	usize size() const {
		return bitCount;
	}

	bool operator[](usize index) const {
		return bitread(index);
	}

	BitArray() {
		bitclr();
	}
};
