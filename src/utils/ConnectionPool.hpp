#pragma once
#include "core.hpp"
#include "Arena.hpp"
#include "Bitmap.hpp"
#include "Connection.hpp"

/*
	Connection Pool uses a two-layer metadata bitmap to efficiently index its 4096 connections
	It is split in 64 blocks, each of which contains 64 connections (aka elements)
	- blockBitmap indexes entire blocks, so it is only set when all connections within it are used
	- elementBitmap represents the use state of a connection
	- delBitmap represents connections marked for deletion
	
	* It does not have a constructor because it is lazily paged and can be trivially constructed
	TODO: Create a bitmap array that takes a dynamic amount of connections
*/

struct ConnectionPool {
	static const usize blockSize = sizeof(Connection) * 64;
	static const usize blockCount = 64;
	static const usize elementCount = 4096;

	Connection connections[elementCount];
	struct {
		Bitmap block;
		Bitmap element[blockCount];	// Metadata for each 64 Connection Block
		Bitmap del[blockCount];		// Connections marked for closure
	}	map;

	void reset() {
		MEMSET_INLINE(&map, 0, sizeof(map));
	}

	Connection* get_ptr(usize linearIndex) {
		return connections + linearIndex;
	}

	template <void (Connection::*Func)()>
	void for_each_active() {
		for (usize blockIndex = 0; blockIndex < blockCount; blockIndex++) {
			Bitmap active = map.element[blockIndex];

			Connection *base = connections + blockIndex * 64;
			usize elementIndex;
			while ((elementIndex = active.find_first_set()) < WORD_BITS) {
				(base[elementIndex].*Func)();
				active.bitclr(elementIndex);
			}
		}
	}

	void clear() {
		for (usize blockIndex = 0; blockIndex < blockCount; blockIndex++) {
			Bitmap &elementBlock = map.element[blockIndex];
			if (elementBlock.value == 0)
				continue;

			Connection *base = connections + blockIndex * 64;
			usize elementIndex;
			while ((elementIndex = elementBlock.find_first_set()) < WORD_BITS) {
				base[elementIndex].end_connection();
				elementBlock.bitclr(elementIndex);
			}
			map.block.bitclr(blockIndex);
			map.del[blockIndex].clear();
		}
	}

	usize acquire_slot(int clientFd, VirtualServer *server) {
		usize blockIndex = map.block.find_first_clear();
		if (blockIndex >= blockCount)
			return SIZE_MAX;

		usize elementIndex = map.element[blockIndex].find_first_clear();
		map.element[blockIndex].bitset(elementIndex);
		if (map.element[blockIndex].value == SIZE_MAX)
			map.block.bitset(blockIndex);

		usize index = blockIndex * 64 + elementIndex;
		connections[index].init(clientFd, server);
		return index;
	}

	void free_slot(usize linearIndex) {
		usize elementIndex = linearIndex % 64;
		usize blockIndex = linearIndex / 64;

		map.block.bitclr(blockIndex);
		map.element[blockIndex].bitclr(elementIndex);
	}

	void mark_for_deletion(usize linearIndex) {
		usize elementIndex = linearIndex % 64;
		usize blockIndex = linearIndex / 64;

		connections[linearIndex].end_connection();
		map.del[blockIndex].bitset(elementIndex);
	}

	Connection& operator[](usize index) {
		return connections[index];
	}
};

STATIC_ASSERT(ConnectionPool::blockCount > 0);
STATIC_ASSERT(ConnectionPool::blockCount <= WORD_BITS);
