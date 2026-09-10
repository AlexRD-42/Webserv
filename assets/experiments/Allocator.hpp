#pragma once
#include "core.hpp"
#include "Bitmap.hpp"

// Key will have 12 bit address range, 12 bit length, 8 bit modulo range
// So 8 bits index

/*
	First we bin it in size classes, for alignment purposes. Let's call them regions:

	8 bytes, 16 bytes, 32 bytes, 64 bytes, ...

	These regions will be indexed by their size class. Then, regions inside it will be indexed by the modulo bit
	The conjunction of these two give the widening of the address range due to a higher alignment, and also the 
	compression mechanism of the address

	Modulo range is not really separate from length, it's just an expression of how much of it will be used for indexing
	So, given that, we can have two tier indexing:

	Index 1: Size Class, refers to the alignment, probably derived from the upper bits?
	Index 2: Modulo class, indexes by modulo
*/

/*
	16 byte alignment
*/

usize size_class(usize index) {
	return 64 - CLZ(index & 0x0FF0 | 1);
}

#define MODBITS 12
#define ADDRBITS 12
#define LENBITS 8
#define ALIGN 16

struct Region {
	u8 data[16 * 4096];
	Bitmap big;
	Bitmap small[64];
};

struct Allocator {
	Region region[256];
};
