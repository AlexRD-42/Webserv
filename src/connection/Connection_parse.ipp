#pragma once
#include "Connection.hpp"

CONNECTION_INL
(Status::Code) parse_first_line(Span line) {
	if (line.size < 14 || line.size >= 8000)	// ERROR: Bad request "GET / HTTP/1.1" shortest possible
		return line.size < 14 ? Status::i400 : Status::i431;

	char* targetEnd = line.ptr + line.size - 9;
	char* targetStart = parseBuffer.rptr();

	if (LITCMP(line.ptr, "GET ") == 0) {
		targetStart += 4;
		options |= Options::GET;
	}
	else if (LITCMP(line.ptr, "POST ") == 0) {
		targetStart += 5;
		options |= Options::POST;
	}
	else if (LITCMP(line.ptr, "DELETE ") == 0) {
		targetStart += 7;
		options |= Options::DELETE;
	}
	else
		return Status::i501;
	if (targetStart >= targetEnd)
		return Status::i400;		// Review
	if (LITCMP(targetEnd, " HTTP/1.1\r\n") != 0)
		return Status::i505;
	parseBuffer.readPos = parseBuffer.scanPos;	//
	return parse_validate(targetStart, targetEnd);
}

CONNECTION_INL
(Status::Code) parse_line(Span line) {
	if (line.size < 2 || line.size >= 8000)
		return line.size < 2 ? Status::i400 : Status::i431;

	const usize readEnd = parseBuffer.readPos + line.size;
	Span field = parseBuffer.find_char(':');
	if (field.ptr == NULL || field.size == 0)
		return Status::i400;

	Field fieldIndex = fn::match_field(field);
	Span value = parseBuffer.get_field_value(readEnd);
	if (value.ptr == NULL)
		return Status::i400;	// Rejects empty values
	switch (fieldIndex) {
		default: break;
// ============================================================================
		case Field::TRANSFER_ENCODING:
			if (options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH))
				return Status::i400; // ERROR: transfer method already set
			if (value.strcasecmp("chunked") == false)
				return Status::i400; // ERROR: transfer encoding isnt chunked
			options |= Options::CHUNKED_LENGTH;
			break;
// ============================================================================
		case Field::CONTENT_LENGTH:
			if (options & (Options::CHUNKED_LENGTH | Options::FIXED_LENGTH))
				return Status::i400; // ERROR: bad request, transfer method had already been set
			bodySize = fn::strtol10(value.ptr, value.size);
			if (bodySize == SIZE_MAX)
				return Status::i400;
			if (bodySize > cfg->maxBodySize)
				return Status::i413;
			options |= Options::FIXED_LENGTH;
			break;
// ============================================================================
		case Field::CONTENT_TYPE:
			req.contentTypeHeader = value;
			break;
// ============================================================================
		case Field::HOST:
			if (options & Options::HOST)
				return Status::i400;	// ERROR: Multiple hosts
			req.host = value;
			options |= Options::HOST;
			break;
// ============================================================================
		case Field::CONNECTION:
			if (value.strcasecmp("keep-alive"))	// not adding another bit just to check
				options |= Options::KEEP_ALIVE;			// if had been set already
			else if (value.strcasecmp("close"))	// last setting counts
				options &= ~(u16)Options::KEEP_ALIVE;
			else
				return Status::i400;
		break;
// ============================================================================
		case Field::COOKIES:
			req.cookies = value;
			break;
	}
	return Status::unset;
}
