#ifndef TOKEN_HXX_INCLUDED
#define TOKEN_HXX_INCLUDED

#include <cstddef>
#include <cassert>
#include <algorithm>
#include <string_view>
#include <string>
#include <variant>

#include "token_type.hxx"

// Use std::nullptr_t to represent Nil
using Primitive = std::variant<std::nullptr_t, std::string, double, bool>;

static std::string double_to_string_trimmed(double val)
{
	auto str = std::to_string(val);
	auto is_zero = [](char c) { return c == '0'; };

	if (auto at = str.find('.');
		at != std::string::npos
		&& std::ranges::all_of(str.substr(at + 1), is_zero)) {
		return str.substr(0, at);
	}

	return str;
}

[[maybe_unused]] static std::string to_string(const Primitive &lit)
{
	switch (lit.index()) {
	case 0:
		return "nil";
	case 1:
		return std::get<1>(lit);
	case 2:
		return double_to_string_trimmed(std::get<2>(lit));
	case 3:
		return std::get<3>(lit) ? "true" : "false";
	}

	assert(!"Unreachable code");
}

struct Token {
	Token(
		TokenType type_, std::string_view lexeme_, Primitive literal_, int line_
	)
		: type(type_)
		, lexeme(lexeme_)
		, literal(literal_)
		, line(line_)
	{
	}

	const TokenType type;
	const std::string_view lexeme;
	const Primitive literal;
	const int line;
};

[[maybe_unused]] static std::string to_string(const Token &tok)
{
	return to_string(tok.type) + " " + std::string(tok.lexeme);
}

#endif