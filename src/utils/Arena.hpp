#pragma once
#include <unistd.h>

#include "core.hpp"
#include "config.hpp"
#include "tables.hpp"
#include "Array.hpp"
#include "Span.hpp"

/*
	Arena does not own its storage.  It only manages a caller-provided byte
	range.  Alpha uses connection storage for parser temporaries; beta uses
	configuration storage for values that outlive parsing.
*/

struct Arena {
	u8* ptr;
	usize size, capacity;

	usize free_space()					const { return capacity - size; }
	u8* mptr(usize fileOffset)			const { return ptr + fileOffset; }
	const u8* kptr(usize fileOffset)	const { return ptr + fileOffset; }

	void clear() {
		size = 0;
	}

	u32 alloc(usize bytes, usize padding = 0, usize alignment = 64) {
		ASSERT(IS_POW2(alignment), "Alignment needs to be power of two");
		bytes = ALIGN_UP(bytes + padding, alignment);
		usize newSize = ALIGN_UP(size, alignment);
		if (bytes > capacity - newSize) {
			PRINT_LN(2, "Error: Out of memory");
			return UINT32_MAX;
		}
		u32 index = newSize;
		size = newSize + bytes;
		return index;
	}

	template <typename Type>
	ArrayView<Type> alloc_array(usize numElements, usize alignSize = 16) {
		ASSERT(numElements <= LONG_MAX / sizeof(Type), "Array size exceeds LONG_MAX");
		const usize bytes = numElements * sizeof(Type);
		alignSize = MAX(alignSize, alignof(Type));
		const u32 index = alloc(bytes, 0, alignSize);
		if (index == UINT32_MAX)
			return {};
		return {(Type*)mptr(index), numElements};
	}

	Span alloc_span(usize length) {
		const u32 offset = alloc(length, 1);
		if (offset == UINT32_MAX)
			_exit(1);
		Span result = {(char*)mptr(offset), length};
		result.ptr[result.size] = '\0';
		return result;
	}

	Span copy_span(const Span &source) {
		Span result = alloc_span(source.size);
		MEMCPY(result.ptr, source.ptr, source.size);
		return result;
	}
};

	// template <typename Type>
	// Span32 compress_span(const Array<Type> &array, usize length) {
	// 	const u32 offset = alloc(length, 1);
	// 	if (offset == UINT32_MAX)
	// 		_exit(1);
	// 	Span32 result = {(u32)((char*)mptr(offset) - (char*)array.ptr), (u32)length};
	// 	array.extract(result).ptr[length] = '\0';
	// 	return result;
	// }

	// template <typename Type>
	// Span32 compress_span(const Array<Type> &array, const Span &source) {
	// 	const Span32 result = compress_span(array, source.size);
	// 	MEMCPY(array.extract(result).ptr, source.ptr, source.size);
	// 	return result;
	// }
