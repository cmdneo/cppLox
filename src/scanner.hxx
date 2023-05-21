#ifndef SCANNER_HXX_INCLUDED
#define SCANNER_HXX_INCLUDED

#include <cstddef>
#include <string_view>
#include <vector>
#include <map>
#include <charconv>

#include "object.hxx"
#include "token.hxx"
#include "token_type.hxx"
#include "error.hxx"

class Scanner
{
public:
	Scanner(std::string_view source_)
		: source(source_)

	{
	}

	std::vector<Token> scan_tokens();

private:
	bool is_at_end() const { return current >= source.size(); }

	char peek() const
	{
		return current >= source.size() ? '\0' : source[current];
	}

	char peek_next() const
	{
		return current + 1 >= source.size() ? '\0' : source[current + 1];
	}

	char advance()
	{
		if (source[current] == '\n')
			line++;
		return source[current++];
	}

	void add_token(TokenType type, Object literal = Object())
	{
		auto lexeme = source.substr(start, current - start);
		tokens.emplace_back(type, lexeme, literal, line);
	}

	bool match(char expected)
	{
		if (is_at_end())
			return false;
		if (source.at(current) != expected)
			return false;

		current++;
		return true;
	}

	void do_identifier();
	void do_string();
	void do_number();
	void scan_token();

	using size_type = std::string_view::size_type;
	const std::string_view source;
	std::vector<Token> tokens;
	size_type start = 0;
	size_type current = 0;
	int line = 1;
};

#endif