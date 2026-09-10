#pragma once
#include "Parser.hpp"

// Guarantee that it doesnt overflow u16

static inline
Span16 s_store_location_span(Location &location, char* &wptr, const Span &source) {
	Span16 result = {(u16)(wptr - (char*)&location.uri), (u16)source.size};
	MEMCPY(wptr, source.ptr, source.size);
	wptr += source.size;
	*wptr++ = '\0';
	return result;
}

static inline
Span16 s_store_upload_span(Location &location, char* &wptr, const Span &source) {
	Span16 result = {(u16)(wptr - (char*)&location.uri), (u16)source.size};
	MEMCPY(wptr, source.ptr, source.size);
	wptr += source.size;
	if (wptr[-1] != '/') {
		*wptr++ = '/';
		result.size++;
	}
	*wptr++ = '\0';
	return result;
}

static inline
void s_store_cgi(char* &wptr, const Parser::ParsedCgi &cgiBlock, Location &location) {
	location.cgiBlock.index = (u16)(wptr - (char*)&location.uri);
	location.cgiBlock.size = (u16)cgiBlock.size;
	for (usize index = 0; index < cgiBlock.definitions.count; index += 4) {
		Parser::Token* definition = cgiBlock.definitions.ptr + index;
		const Span &extension = definition[0].value;
		const Span &interpreter = definition[2].value;
		const u16 lengths[2] = {(u16)extension.size, (u16)interpreter.size};
		MEMCPY_INLINE(wptr, lengths, sizeof(lengths));
		wptr += sizeof(lengths);
		MEMCPY(wptr, extension.ptr, extension.size);
		wptr += extension.size;
		MEMCPY(wptr, interpreter.ptr, interpreter.size);
		wptr += interpreter.size;
	}
	*wptr++ = '\0';	
}

static inline
void s_store_location(char* &wptr, const Parser::ParsedLocation &ploc, Location &loc) {
	loc.uri = s_store_location_span(loc, wptr, ploc.uri);
	loc.root = s_store_location_span(loc, wptr, ploc.root);
	loc.index = s_store_location_span(loc, wptr, ploc.index);
	loc.uploadStore = s_store_upload_span(loc, wptr, ploc.uploadStore);
	s_store_cgi(wptr, ploc.cgiBlock, loc);
	loc.redirectTarget = s_store_location_span(loc, wptr, ploc.redirectTarget);
	loc.redirectStatus = ploc.redirectStatus;
	loc.methods = ploc.methods;
	loc.autoindex = ploc.autoindex;
}

static inline
usize s_location_size(const Parser::ParsedLocation &loc) {
	usize packSize = 16 + loc.uri.size + loc.root.size + loc.index.size;
	packSize += loc.uploadStore.size + loc.cgiBlock.size + loc.redirectTarget.size;
	return packSize;
}

PARSER_INL
(ArrayView<Location>) store_locations(ArrayView<ParsedLocation> &ploc) {
	usize allocationSize = ploc.count * sizeof(Location);
	for (usize index = 0; index < ploc.count; index++)
		allocationSize += s_location_size(ploc[index]);
	const u32 allocation = beta.alloc(allocationSize, 0, __alignof__(Location));
	if (allocation == UINT32_MAX)
		_exit(1);
	ArrayView<Location> locations((Location*)beta.mptr(allocation), ploc.count);
	char* wptr = (char*)(locations.ptr + locations.count);
	for (usize locationIndex = 0; locationIndex < locations.count; locationIndex++)
		s_store_location(wptr, ploc[locationIndex], locations[locationIndex]);
	return locations;
}

PARSER_INL
(ArrayView<Location>) process_locations(ArrayView<ParsedLocation> &ploc, VirtualServer &server) {
	Span &serverRoot = server.serverRoot;

	if (server.host.size == 0)
		server.host = beta.copy_span(Span::create("localhost"));
	if (serverRoot.size == 0)
		serverRoot = beta.copy_span(Span::create(""));
	while (serverRoot.size != 0 && serverRoot.ptr[serverRoot.size - 1] == '/')
		serverRoot.size--;
	serverRoot.ptr[serverRoot.size] = '\0';
	
	Span defaultIndex = beta.copy_span(Span::create("/index.html"));
	for (usize index = 0; index < ploc.count; index++) {
		ParsedLocation &src = ploc[index];
		if (src.root.size == 0)
			src.root = serverRoot;
		while (src.root.size != 0 && src.root.ptr[src.root.size - 1] == '/')
			src.root.size--;
		if (src.uploadStore.size == 0)
			src.uploadStore = src.root;
		if (src.index.size == 0)
			src.index = defaultIndex;
		else if (*src.index.ptr != '/') {
			*(--src.index.ptr) = '/';
			src.index.size++;
		}
		if (src.methods == 0)
			src.methods = Options::GET;
	}
	return store_locations(ploc);
}

static inline
void s_build_error_page_path(char* out, const Span &root, const Span &path) {
	ASSERT(path.size != 0, "Error page path is empty");
	ASSERT(root.size == 0 || root.ptr[root.size - 1] != '/', "Root has a trailing slash");
	usize length = root.size;
	MEMCPY(out, root.ptr, length);
	if (length != 0 && path.ptr[0] != '/')
		out[length++] = '/';
	MEMCPY(out + length, path.ptr, path.size);
	length += path.size;
	out[length] = '\0';
}

PARSER_INL
(void) cache_error_pages(VirtualServer &server) {
	char pathBuffer[4 * MAX_PATH_SIZE];
	Span configuredPaths[Status::errorPageCount];

	for (usize index = 0; index < Status::errorPageCount; index++)
		configuredPaths[index] = server.errorPages[index];

	for (usize index = 0; index < Status::errorPageCount; index++) {
		Span &page = server.errorPages[index];
		const Span &path = configuredPaths[index];
		if (path.size == 0) {
			page = Status::s_error_page(index);
			continue;
		}

		usize duplicate = 0;
		for (; duplicate < index; duplicate++) {
			const Span &previousPath = configuredPaths[duplicate];
			if (path.size == previousPath.size && MEMCMP(path.ptr, previousPath.ptr, path.size) == 0)
				break;
		}
		if (duplicate != index) {
			page = server.errorPages[duplicate];
			continue;
		}
		s_build_error_page_path(pathBuffer, server.serverRoot, path);
		if (fn::read_whole_file(beta, pathBuffer, page, 0, 0, MAX_ERROR_PAGE_SIZE))
			_exit(1);
	}
}
