#ifndef TOKEN_HXX_INCLUDED
#define TOKEN_HXX_INCLUDED

#include <string>

#include "object.hxx"
#include "token_type.hxx"

struct Token {
	Token(
		TokenType type_, const std::string &lexeme_, Object literal_, int line_
	)
		: type(type_)
		, lexeme(lexeme_)
		, literal(literal_)
		, line(line_)
	{
	}

	const TokenType type;
	const std::string lexeme;
	const Object literal;
	const int line;
};

[[maybe_unused]] static std::string to_string(const Token &tok)
{
	return to_string(tok.type) + " " + tok.lexeme;
}

#endif