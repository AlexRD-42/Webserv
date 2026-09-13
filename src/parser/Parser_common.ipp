#pragma once
#include "Parser.hpp"

PARSER_INL
(Parser::Directive) s_build_directive(Arena& arena, ArrayView<Token>& tokArray) {
	Directive dir;
	if (tokArray[0].type != Token::WORD)
		PERR_EXIT(1, "Error: Unexpected token");
	dir.name = tokArray[0].value;
	tokArray.ptr++;
	Token* argumentStart = tokArray.ptr;
	while (tokArray[0].type == Token::WORD)
		tokArray.ptr++;
	if (tokArray[0].type != Token::SEMICOLON)
		PERR_EXIT(1, "Error: Unexpected token");

	const usize argumentCount = (usize)(tokArray.ptr - argumentStart);
	dir.args = arena.alloc_array<Span>(argumentCount);
	if (dir.args.ptr == NULL)
		_exit(1);
	for (usize index = 0; index < dir.args.count; index++)
		dir.args[index] = argumentStart[index].value;
	tokArray.ptr++;
	return dir;
}
