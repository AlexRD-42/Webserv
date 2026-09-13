#pragma once
#include "Buffer.hpp"

BUFFER_INL
(char*) readdir(int fd) {
	if (readPos == writePos) {
		isize bytesRead = ::getdents64(fd, data, sizeof(data));
		while (bytesRead == -1 && errno == EINTR)
			bytesRead = ::getdents64(fd, data, sizeof(data));
		if (bytesRead <= 0) {
			errno = bytesRead == 0 ? 0 : errno;
			return NULL;
		}
		readPos = 0;
		writePos = (usize)bytesRead;
	}
	u16 recordLength;
	char* recordLengthPtr = (char*)data + readPos + offsetof(dirent64, d_reclen);
	MEMCPY_INLINE(&recordLength, recordLengthPtr, 2);
	char* name = (char*)data + readPos + offsetof(dirent64, d_name);
	readPos += recordLength;
	return name;
}

BUFFER_INL
(usize) compact() {
	const usize bytesUsed = writePos - readPos;
	const usize scanOffset = scanPos - readPos;

	MEMMOVE(data, data + readPos, bytesUsed);
	readPos = 0;
	scanPos = scanOffset;
	writePos = bytesUsed;
	return sizeof(data) - writePos;
}

BUFFER_INL
(isize) read_compact(int fd, usize bytes) {
	usize bytesFree = sizeof(data) - writePos;

	if (bytesFree < bytes) {
		bytesFree = compact();
		if (bytesFree == 0)
			return -2;
	}

	const usize bytesCapped = MIN(bytesFree, bytes);
	isize bytesRead = ::read(fd, data + writePos, bytesCapped);
	if (bytesRead < 0)
		return -1;
	writePos += (usize)bytesRead;
	return bytesRead;
}

BUFFER_INL
(isize) read(int fd, usize bytes) {
	const usize bytesFree = sizeof(data) - writePos;
	const usize bytesCapped = MIN(bytesFree, bytes);

	if (bytesCapped == 0)
		return -1;
	isize bytesRead = ::read(fd, data + writePos, bytesCapped);
	if (bytesRead > 0)
		writePos += (usize)bytesRead;
	return bytesRead;
}

BUFFER_INL
(isize) write(int fd, usize bytes) {
	usize bytesCapped = MIN3(ATOMIC_IOSIZE, bytes, writePos - readPos);
	isize bytesWritten = ::write(fd, data + readPos, bytesCapped);

	if (bytesWritten > 0) {
		readPos += (usize)bytesWritten;
		scanPos = (scanPos >= readPos) ? scanPos : readPos;
	}
	return bytesWritten;
}

BUFFER_INL
(isize) write_all(int fd, usize bytes) {
	usize numRetries = 3;
	const isize bytesTotal = MIN(bytes, writePos - readPos);
	usize bytesToWrite = bytesTotal;

	while (bytesToWrite > 0) {
		usize bytesCapped = MIN(ATOMIC_IOSIZE, bytesToWrite);
		isize bytesWritten = ::write(fd, data + readPos, bytesCapped);
		if (bytesWritten <= 0) {
			if (numRetries == 0)
				return -1;
			numRetries--;
			continue;
		}
		bytesToWrite -= (usize)bytesWritten;
		readPos += (usize)bytesWritten;
		scanPos = (scanPos >= readPos) ? scanPos : readPos;
	}
	return bytesTotal;
}
