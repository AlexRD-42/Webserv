#pragma once
#include "Parser.hpp"

static inline
void s_directive_error_page(Parser::Directive &dir, VirtualServer &server) {
	if (dir.args.count < 2)
		PERR_EXIT(1, "Error: Invalid error page");
	Span path = dir.args[dir.args.count - 1];
	if (path.size >= MAX_PATH_SIZE)
		PERR_EXIT(1, "Error: Invalid error page");
	for (usize index = 0; index + 1 < dir.args.count; index++) {
		Status status = {(u16)Status::s_str_to_code(dir.args[index].ptr)};
		if (dir.args[index].size != 3 || !status.is_error())
			PERR_EXIT(1, "Error: Invalid error number");
		if (server.errorPages[status.get_page_index()].size != 0)
			PERR_EXIT(1, "Error: Duplicate error page");
		server.errorPages[status.get_page_index()] = path;
	}
}

static inline
void s_directive_listen(Arena &arena, const Span &value, VirtualServer &server) {
	if (server.port != SIZE_MAX)
		PERR_EXIT(1, "Error: Invalid port definition");
	char* port = value.ptr;
	usize portLength = value.size;
	char* separator = (char*)MEMCHR(port, ':', portLength);
	if (separator != NULL) {
		usize hostLength = (usize)(separator - port);
		if (hostLength == 0 || hostLength == value.size - 1 || server.host.size != 0)
			PERR_EXIT(1, "Error: Invalid listen address");
		Span host = {port, hostLength};
		server.host = arena.copy_span(host);
		port = separator + 1;
		portLength -= hostLength + 1;
	}
	server.port = fn::strtol10(port, portLength);
	if (server.port < 1 || server.port > 65535)
		PERR_EXIT(1, "Error: Invalid port");
}

static inline
void s_directive_body_size(const Span &value, usize &bodySize) {
	if (bodySize != SIZE_MAX)
		PERR_EXIT(1, "Error: Invalid max body size");

	u8 factor = 0;
	const char* str = value.ptr;
	usize digitLength = value.size;
	if (str[digitLength - 1] == 'G') {
		factor = 30;
		digitLength--;
	}
	else if (str[digitLength - 1] == 'M') {
		factor = 20;
		digitLength--;
	}
	else if (str[digitLength - 1] == 'K') {
		factor = 10;
		digitLength--;
	}
	if (digitLength == 0)
		PERR_EXIT(1, "Error: Invalid max body size");
	const usize bytes = fn::strtol10(str, digitLength);
	if (bytes > ((usize)LONG_MAX >> factor))
		PERR_EXIT(1, "Error: Invalid max body size");
	bodySize = bytes << factor;
}

PARSER_INL
(void) parse_server_directive(VirtualServer &server, Directive &dir) {
	const Span &name = dir.name;

	if (name == "error_page")
		return s_directive_error_page(dir, server);
	
	if (dir.args.count != 1 || dir.args[0].size >= MAX_PATH_SIZE)
		PERR_EXIT(1, "Error: Invalid server directive");
	const Span &value = dir.args[0];
	if (name == "listen")
		return s_directive_listen(beta, value, server);
	else if (name == "host") {
		if (server.host.size != 0)
			PERR_EXIT(1, "Error: Duplicate host definition");
		server.host = beta.copy_span(value);
	}
	else if (name == "client_max_body_size")
		s_directive_body_size(value, server.maxBodySize);
	else if (name == "root") {
		if (server.serverRoot.size != 0)
			PERR_EXIT(1, "Error: Duplicate root definition");
		server.serverRoot = beta.copy_span(value);
	}
	else
		PERR_EXIT(1, "Error: Invalid server directive");
}

static inline
usize s_count_locations(ArrayView<Parser::Token> tokArray) {
	usize locationCount = 0;
	while (tokArray[0].type != Parser::Token::CLOSE_BRACKET) {
		if (tokArray[0].value == "location") {
			Parser::Token* locationStart = tokArray.ptr;
			tokArray.ptr++;
			if (tokArray[0].type != Parser::Token::WORD || tokArray[1].type != Parser::Token::OPEN_BRACKET)
				PERR_EXIT(1, "Error: Invalid location block");
			tokArray.ptr += 2;
			usize braces = 1;
			while (braces != 0) {
				braces += tokArray[0].type == Parser::Token::OPEN_BRACKET;
				braces -= tokArray[0].type == Parser::Token::CLOSE_BRACKET;
				tokArray.ptr++;
			}
			const usize locationSize = (usize)(tokArray.ptr[-1].value.ptr - locationStart->value.ptr) + 1;
			if (locationSize > MAX_LOCATION_BLOCK_SIZE)
				PERR_EXIT(1, "Error: Invalid location block");
			locationCount++;
		}
		else {
			while (tokArray[0].type == Parser::Token::WORD)
				tokArray.ptr++;
			if (tokArray[0].type != Parser::Token::SEMICOLON)
				PERR_EXIT(1, "Error: Unexpected token");
			tokArray.ptr++;
		}
	}
	if (locationCount > MAX_LOCATION_COUNT)
		PERR_EXIT(1, "Error: Location count exceeds the maximum");
	return locationCount;
}

PARSER_INL
(void) parse_server(ArrayView<Token> &tokArray, VirtualServer &server) {
	tokArray.ptr++;
	usize locationCount = s_count_locations(tokArray);
	ArrayView<ParsedLocation> parsedLocations = alpha.alloc_array<ParsedLocation>(locationCount);
	if (parsedLocations.ptr == NULL)
		std::exit(1);

	usize locationIndex = 0;
	while (tokArray[0].type != Token::CLOSE_BRACKET) {
		if (tokArray[0].value == "location") {
			tokArray.ptr++;
			ParsedLocation loc = parse_location(tokArray);
			for (usize index = 0; index < locationIndex; index++) {
				const Span &path = parsedLocations[index].uri;
				if (path.size == loc.uri.size && MEMCMP(path.ptr, loc.uri.ptr, path.size) == 0)
					PERR_EXIT(1, "Error: Duplicate location");
			}
			parsedLocations[locationIndex] = loc;
			locationIndex++;
		}
		else {
			Directive dir = s_build_directive(alpha, tokArray);
			parse_server_directive(server, dir);
		}
	}
	tokArray.ptr++;
	if (server.port == SIZE_MAX)
		PERR_EXIT(1, "Error: Missing listen directive");
	if (server.maxBodySize == SIZE_MAX)
		server.maxBodySize = LONG_MAX;
	server.locations = process_locations(parsedLocations, server);
}
