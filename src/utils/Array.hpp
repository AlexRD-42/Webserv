#pragma once
#include "core.hpp"
#include "Span.hpp"

template <typename Type>
struct ArrayView {
	Type* ptr;
	usize count;

	Type& operator[](usize index) {
		return ptr[index];
	}

	const Type& operator[](usize index) const {
		return ptr[index];
	}

};

template <typename Type, usize count>
struct Array {
	Type ptr[count];

	Type& operator[](usize index) {
		return ptr[index];
	}

	const Type& operator[](usize index) const {
		return ptr[index];
	}
};

// Span extract(const Span32& span) const {
// 	Span result;
// 	result.ptr = (char*)ptr + span.index;
// 	result.size = span.size;
// 	return result;
// }

/* Packed array
	The reason for this array's existence is to offload the memory usage consumed by the
	indices to the array itself. It has indirect access, meaning it's worse for direct
	access. But given that the array contains elements with varying lengths, there is no
	scheme that gives you direct access without wasting memory

	Compressed array format could be:
	metadata index[0] index[1] index[2] index[3]

	* Metadata would contain information on how many indices exist, the total length
	of the block, and size of each index
	* Each index contains a 16 byte alignment offset length relative to the beginning
	So a u8 could index up to 4KB of memory, a u16 1MB
	* The packed array access is essentially array + offset + indexSize
	* The length can be either:
		- the difference between index[n] - index[n-1] + find first zero byte of the last 16 bytes
			assumes the element is null terminated
		- stored together with the index
		- byte stored as the last element of the block alignment
			It would encode how many bytes behind the string is, so length = index[n] - index[n-1] - index[n-1][-1]
*/

// template <usize bufferSize>
// struct Array {
// 	u8 data[bufferSize];
// 	usize length;
// };

template <typename Type, usize rows, usize cols>
struct Matrix {
	Type array[rows][cols];

	Type (&operator[](usize index))[cols] {
		return array[index];
	}

	const Type (&operator[](usize index) const)[cols] {
		return array[index];
	}
};
