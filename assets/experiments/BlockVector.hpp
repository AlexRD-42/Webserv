#pragma once

#include "BitArray.hpp"
#include "Arena.hpp"
#include "core.hpp"
#include <unistd.h>

template <typename Type, usize blockSize, usize maxGrowth>
class BlockVector {
public:
	static const usize maxElements = blockSize * maxGrowth;

	u32 blocks[maxGrowth];
	BitArray<maxElements> metadata;
	usize numBlocks;
	usize numAllocatedBlocks;
	usize numElements;

	BlockVector() : numBlocks(0), numAllocatedBlocks(0), numElements(0) {
		for (usize index = 0; index < maxGrowth; index++)
			blocks[index] = UINT32_MAX;
		if (!grow())
			_exit(1);
	}

	usize find_free_slot() {
		usize freeIndex = metadata.find_first_clear();
		if (freeIndex >= capacity()) {
			if (freeIndex >= maxElements || !grow())
				return SIZE_MAX;
		}
		return freeIndex;
	}

	usize acquire_slot() {
		usize index = find_free_slot();
		if (index == SIZE_MAX)
			return SIZE_MAX;
		metadata.bitset(index);
		numElements++;
		return index;
	}

	void init(usize index) {
		(*this)[index].init();
		metadata.bitset(index);
		numElements++;
	}

	void clear(usize index) {
		(*this)[index].clear();
		numElements--;
		metadata.bitclr(index);
	}

	bool grow() {
		if (numBlocks >= maxGrowth)
			return false;
		if (numBlocks == numAllocatedBlocks) {
			u32 offset = Arena::alloc_index(sizeof(Type) * blockSize);
			if (offset == UINT32_MAX)
				return false;
			blocks[numBlocks] = offset;
			numAllocatedBlocks++;
		}
		numBlocks++;
		return true;
	}

	bool shrink() {
		if (numBlocks <= 1)
			return false;
		numBlocks--;
		return true;
	}

	usize capacity() const {
		return blockSize * numBlocks;
	}

	usize size() const {
		return numElements;
	}

	Type& operator[](usize index) {
		u32 blockOffset = blocks[index / blockSize];
		return ((Type*)(Arena::data + blockOffset))[index % blockSize];
	}

	const Type& operator[](usize index) const {
		u32 blockOffset = blocks[index / blockSize];
		return ((const Type*)(Arena::data + blockOffset))[index % blockSize];
	}
};
