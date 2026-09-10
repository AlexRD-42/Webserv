#pragma once

#include "core.hpp"
#include <cstdlib>
#include <unistd.h>

class Arena {
private:
	Arena();

public:
	static u8* data;
	static usize size;
	static usize capacity;

	static void clear() {
		if (data != NULL)
			std::free(data);
		data = NULL;
		size = 0;
		capacity = 0;
	}

	static bool error() {
		PRINT_LN(2, "Error: Memory allocation failed");
		clear();
		return true;
	}

	static bool init(u16 initialCapacity) {
		size = 0;
		capacity = NEXT_POW2((usize)initialCapacity << 10);
		data = (u8*) std::malloc(capacity);
		if (data == NULL)
			return error();
		return false;
	}

	static bool realloc(usize bytes) {
		usize newCapacity = NEXT_POW2(bytes + size);
		if (newCapacity >= UINT32_MAX)
			return error();
		u8* newAllocPtr = (u8*) std::realloc(data, newCapacity);
		if (newAllocPtr == NULL)
			return error();
		capacity = newCapacity;
		data = newAllocPtr;
		return false;
	}

	static void* alloc(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (size + bytes >= capacity && realloc(bytes) == true)
			return NULL;
		u8* ptr = data + size;
		size += bytes;
		return ptr;
	}

	static u32 alloc_index(usize bytes) {
		bytes = ALIGN_UP(bytes, 64);
		if (size + bytes >= capacity && realloc(bytes) == true)
			return UINT32_MAX;
		u32 index = size;
		size += bytes;
		return index;
	}
};

#ifdef MAIN_FILE
	u8* Arena::data = NULL;
	usize Arena::size = 0;
	usize Arena::capacity = 0;
#endif
