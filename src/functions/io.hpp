#pragma once
// #include <unistd.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "core.hpp"
#include "Arena.hpp"
#include "Span.hpp"

namespace fn {

// ATTR(always_inline)
// Span alloc_whole_file(Arena& arena, const char* filePath, int& fd, usize padSize = 32, usize minSize = 0, usize maxSize = UINT32_MAX) {
// 	struct stat st;
// 	Span result = {};

// 	if (stat(filePath, &st) == -1 || (usize)st.st_size < minSize || (usize)st.st_size >= maxSize)
// 		PERR_RETURN(result, "Error: Invalid file");

// 	fd = open(filePath, O_RDONLY);
// 	if (fd == -1)
// 		PERR_RETURN(result, "Error: Failed to open file");

// 	const usize fileSize = (usize)st.st_size;
// 	const u32 fileOffset = arena.alloc(fileSize, 1 + padSize);
// 	if (fileOffset == UINT32_MAX) {
// 		close(fd);
// 		PERR_RETURN(result, "Error: Out of memory");
// 	}
// 	result.ptr = (char*)arena.mptr(fileOffset);
// 	result.size = fileSize;
// 	return result;
// }

ATTR(static_inl, flatten)
int close_noerr(int fd) {
	if (fd < 0)
		return -1;
	const int error = errno;
	close(fd);
	errno = error;
	return -1;
}

ATTR(static_inl, flatten)
int validate_file(int fd, struct stat* st, usize minSize = 0, usize maxSize = MAX_FILE_SIZE) {
	if (!S_ISREG(st->st_mode) || (usize)st->st_size < minSize || (usize)st->st_size > maxSize)
		return close_noerr(fd);
	return fd;
}

ATTR(static_inl, flatten)
int open_with_info(int dirFd, struct stat* st, const char* filePath, int flags, int accessFlags = 0) {
	int fd = openat(dirFd, filePath, flags, accessFlags);
	if (fd == -1)
		return -1;
	if (fstat(fd, st) == -1)
		return fn::close_noerr(fd);
	return fd;
}

ATTR(static_inl, flatten)
bool read_all(int fd, char* buffer, usize fileSize) {
	usize curBytes = 0;
	while (curBytes < fileSize) {
		usize bytesRemaining = fileSize - curBytes;
		isize bytesRead = read(fd, buffer + curBytes, bytesRemaining);
		if (bytesRead <= 0) {
			if (bytesRead < 0 && errno == EINTR)
				continue;
			close(fd);
			return true;
		}
		curBytes += (usize)bytesRead;
	}
	close(fd);
	return false;
}

ATTR(static_inl, flatten)
bool read_whole_file(Arena& arena, const char* filePath, Span& file, usize padSize = 32, usize minSize = 0, usize maxSize = UINT32_MAX) {
	int fd = open(filePath, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (fd == -1)
		PERR_RETURN(1, "Error: Failed to open file");
	struct stat st;
	if (fstat(fd, &st) == -1 || !S_ISREG(st.st_mode) || (usize)st.st_size < minSize || (usize)st.st_size > maxSize) {
		close(fd);
		PERR_RETURN(1, "Error: Invalid file");
	}

	const usize fileSize = (usize)st.st_size;
	const u32 fileOffset = arena.alloc(fileSize, 1 + padSize);
	if (fileOffset == UINT32_MAX) {
		close(fd);
		PERR_RETURN(1, "Error: Out of memory");
	}

	u8* ptr = arena.mptr(fileOffset);
	usize curBytes = 0;
	while (curBytes < fileSize) {
		usize bytesRemaining = fileSize - curBytes;
		isize bytesRead = read(fd, ptr + curBytes, MIN(bytesRemaining, ATOMIC_IOSIZE));
		if (bytesRead <= 0) {
			close(fd);
			PERR_RETURN(1, "Error: Read failure");
		}
		curBytes += (usize)bytesRead;
	}
	close(fd);
	ptr[fileSize] = '\0';
	file.ptr = (char*)ptr;
	file.size = fileSize;
	return 0;
}

}
