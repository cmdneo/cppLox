#ifndef PARSER_HXX_INCLUDE
#define PARSER_HXX_INCLUDE

#include <stdexcept>
#include <initializer_list>
#include <memory>
#include <vector>
#include <string_view>
#include <utility>

#include "token.hxx"
#include "expr.hxx"
#include "error.hxx"

struct ParseError : public std::runtime_error {
	ParseError()
		: runtime_error("<ParseError>")
	{
	}
};

class Parser
{
public:
	Parser(const std::vector<Token> &tokens_)
		: tokens(tokens_)
	{
	}

	ExprPtr parse()
	{
		try {
			return expression();
		} catch (ParseError) {
			return nullptr;
		}
	}

private:
	ParseError make_error(Token token, std::string_view message) const
	{
		print_error(token, message);
		return ParseError();
	}

	Token peek() const { return tokens[current]; }

	Token previous() const { return tokens[current - 1]; }

	bool is_at_end() const { return peek().type == TokenType::END_OF_FILE; }

	bool check(TokenType type) const
	{
		if (is_at_end())
			return false;
		return peek().type == type;
	}

	Token advance()
	{
		if (!is_at_end())
			current++;
		return previous();
	}

	bool match(std::initializer_list<TokenType> types)
	{
		for (auto t : types) {
			if (t == tokens[current].type) {
				advance();
				return true;
			}
		}
		return false;
	}

	Token consume(TokenType type, std::string_view message)
	{
		if (check(type))
			return advance();

		// Oh, NO! We are using C++ exceptions. What have we done :()
		throw make_error(peek(), message);
	}

	void synchronize();

	ExprPtr expression();
	ExprPtr ternary();
	ExprPtr equality();
	ExprPtr comparison();
	ExprPtr term();
	ExprPtr factor();
	ExprPtr unary();
	ExprPtr primary();

	const std::vector<Token> tokens;
	std::vector<Token>::size_type current = 0;
};

#endif
