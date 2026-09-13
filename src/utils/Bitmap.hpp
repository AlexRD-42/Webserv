#pragma once
#include "core.hpp"

// TODO: Create 32, 16 and 8 bit specializations
// TODO: bitmap array & 1000 for example through overloading

struct Bitmap {
	usize value;

	ATTR(static_inl) usize mask_start(usize bitStart) {
		return SIZE_MAX << bitStart % WORD_BITS;
	}

	ATTR(static_inl) usize mask_end(usize bitEnd) {
		return SIZE_MAX >> ((usize)(0 - bitEnd) % WORD_BITS);
	}

	ATTR(static_inl) usize mask_range(usize bitStart, usize bitEnd) {
		return mask_start(bitStart) & mask_end(bitEnd);
	}

	ATTR(inl) void bitset(usize index) {
		value |= (usize)1 << index;
	}

	ATTR(inl) void bitclr(usize index) {
		value &= ~((usize)1 << index);
	}

	ATTR(inl) void bitflip(usize index) {
		value ^= (usize)1 << index;
	}

	// Inclusive start, Exclusive end
	ATTR(inl) void bitwrite(usize bitStart, usize bitEnd, bool bit) {
		const usize mask = mask_range(bitStart, bitEnd);
		const usize bitMask = (usize)-bit;

		value ^= (value ^ bitMask) & mask;
	}

	ATTR(inl) bool bitread(u8 index) const {
		return (value & ((usize)1 << index)) != 0;
	}

	ATTR(inl) usize bitread(usize bitStart, usize bitEnd) const {
		return (value & mask_range(bitStart, bitEnd)) >> bitStart;
	}

	ATTR(inl) static usize s_pop_first_set(usize &bitmap) {
		usize index = bitmap == 0 ? WORD_BITS : (usize)CTZ(bitmap);
		bitmap &= bitmap - 1;
		return index;
	}

	ATTR(inl) usize pop_first_set() {
		usize index = value == 0 ? WORD_BITS : (usize)CTZ(value);
		value &= value - 1;
		return index;
	}

	ATTR(inl) usize find_first_clear() {
		if (value == SIZE_MAX)
			return WORD_BITS;
		return (usize)CTZ(~value);
	}

	ATTR(inl) usize find_first_set() const {
		if (value == 0)
			return WORD_BITS;
		return (usize)CTZ(value);
	}

	// template <void (*Func)(usize)>
	// void for_each_active() const {
	// 	usize active = bitmap;

	// 	while (active != 0) {
	// 		usize index = (usize) FFS(active) - 1;
	// 		Func(index);
	// 		active &= active - 1;
	// 	}
	// }

/* ========== Accessors and Overloads ======================== */
	ATTR(inl) usize count() const {
		return (usize)POPCOUNT(value);
	}

	ATTR(inl) void clear() {
		value = 0;
	}

	ATTR(inl) void set() {
		value = SIZE_MAX;
	}

	operator usize() {
		return value;
	}
};
