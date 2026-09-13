#pragma once
#include <unistd.h>
#include <dirent.h>

#include "core.hpp"
#include "tables.hpp"
#include "webserv.hpp"
#include "Span.hpp"
#include "pure_functions.hpp"
#include "Clock.hpp"
#include <errno.h>

#define BUFFER_INL(ret_type) \
	template <usize bufferSize> inline ret_type Buffer<bufferSize>::

#define BUFFER_INL_T(tmpl_param, ret_type) \
	template <usize bufferSize> template <tmpl_param> inline ret_type Buffer<bufferSize>::

template <usize bufferSize>
struct Buffer {
	u64 clobberPre;
	u8 data[bufferSize - 16 - (3 * sizeof(usize))];	// Trailing storage pads unbounded memory loads
	u64 clobberPost;
	usize writePos, readPos, scanPos;

	ATTR(inl) u8* get_end() {	// rename to mptr
		return data + sizeof(data);
	}

	ATTR(inl) Span get_span() {
		Span result = {(char*)data + readPos, writePos - readPos};
		return result;
	}

	ATTR(inl) const u8* get_end() const {
		return data + sizeof(data);
	}

	ATTR(inl) usize size() const {
		return writePos - readPos;
	}

	ATTR(inl) usize capacity() const {
		return sizeof(data);
	}

	ATTR(inl) usize bytes_free() const {
		return sizeof(data) - writePos;
	}

	ATTR(inl) usize reserve(usize bytes) {
		usize bytesFree = bytes_free();
		if (bytes >= bytesFree)
			bytesFree = compact();
		return bytesFree;
	}

	ATTR(inl) bool is_full() {
		return writePos >= sizeof(data);
	}

	void clear() {
		writePos = 0;
		readPos = 0;
		scanPos = 0;
	}

	ATTR(inl) void init(usize newWritePos, usize newReadPos, usize newScanPos) {
		writePos = newWritePos;
		readPos = newReadPos;
		scanPos = newScanPos;
	}

	// IO
	usize compact();
	isize read_compact(int fd, usize bytes);
	isize read(int fd, usize bytes);
	isize write(int fd, usize bytes);
	isize write_all(int fd, usize bytes);
	char* readdir(int fd);

	// HTTP
	Status::Code dechunk(Buffer& tmp, usize& chunkSize, usize& bodySize);

	// Search
	Span find_line_end();
	Span find_header_end();
	Span find_cgi_line_end();
	Span find_cgi_header_end();
	Span find_char(u8 c);

	bool skip_spaces();
	Span get_field_value(usize readEnd);

	// Appends and Prepends
	char* append_char(char c);
	template <usize N> char* append(const char (&string)[N]);				// Implicit
	template <usize N> char* append_inline(const char* ptr, usize length);	// Explicit
	char* append(const char* ptr, usize length);
	char* append(const Span& span);	// TODO: Is it better to have const ref or normal

	template <usize N> char* prepend(const char (&string)[N]);
	template <usize N> char* prepend_inline(const char* ptr, usize length);
	char* prepend(const char* ptr, usize length);
	char* prepend(const Span& span);

	// Append Special
	char* append_mime(Mime mimeIndex);
	char* append_digit10(usize number);
	char* append_digit16(usize number);
	char* append_url_component(const char* ptr, usize length);
	char* append_html(char* ptr, usize length);
	usize append_entry(int directoryFd, char* name);
	char* memset(u8 byte, usize length);
	template <usize N> char* memset_inline(u8 byte, usize length);

	void bufcpy(const Buffer& other) {
		const usize bytesUsed = other.writePos - other.readPos;
		writePos = bytesUsed;
		readPos = 0;
		scanPos = 0;
		MEMCPY(data, other.data + other.readPos, bytesUsed);
	}

	ATTR(inl) operator char*() { return (char*)(data + readPos); }
	ATTR(inl) char* rptr() { return (char*)(data + readPos); }
	ATTR(inl) u8& rptr(usize index) { return *(data + readPos + index); }
	ATTR(inl) char* wptr() { return (char*)(data + writePos); }
	ATTR(inl) u8& wptr(usize index) { return *(data + writePos + index); }
	ATTR(inl) char* sptr() { return (char*)(data + scanPos); }
	ATTR(inl) u8& sptr(usize index) { return *(data + scanPos + index); }
	ATTR(inl) u8& operator*() { return data[writePos]; }
};

typedef Buffer<8 * 1024> Buffer8;
typedef Buffer<16 * 1024> Buffer16;
typedef Buffer<32 * 1024> Buffer32;
typedef Buffer<64 * 1024> Buffer64;
typedef Buffer<HTTP_BUFFERSIZE> HTTP_Buffer;
typedef Buffer<2 * HTTP_BUFFERSIZE - 256> HTTP_PBuffer;

#include "Buffer_io.ipp"
#include "Buffer_add.ipp"
#include "Buffer_add_special.ipp"
#include "Buffer_search.ipp"
#include "Buffer_http.ipp"
