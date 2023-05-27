#ifndef PARSER_HXX_INCLUDE
#define PARSER_HXX_INCLUDE

#include <stdexcept>
#include <initializer_list>
#include <memory>
#include <vector>
#include <string_view>
#include <utility>

#include "error.hxx"
#include "token.hxx"
#include "stmt.hxx"

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

	std::vector<StmtPtr> parse();

private:
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

	ParseError make_error(Token token, std::string_view message) const
	{
		print_error(token, message);
		return ParseError();
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
		throw make_error(peek(), message);
	}

	void synchronize();

	StmtPtr declaration();
	StmtPtr class_declaration();
	Function function(std::string_view kind);
	StmtPtr var_declaration();
	StmtPtr statement();
	StmtPtr assert_statement();
	StmtPtr print_statement();
	StmtPtr break_statement();
	StmtPtr continue_statement();
	StmtPtr return_statement();
	StmtPtr if_statement();
	StmtPtr while_statement();
	StmtPtr for_statement();
	StmtPtr block();
	StmtPtr expression_statement();

	ExprPtr expression();
	ExprPtr assignment();
	ExprPtr ternary();
	ExprPtr logic_or();
	ExprPtr logic_and();
	ExprPtr equality();
	ExprPtr comparison();
	ExprPtr term();
	ExprPtr factor();
	ExprPtr unary();
	ExprPtr call();
	ExprPtr primary();

	// Parsing helpers (common facilities)
	// Parses a block. Like: { ... }
	std::vector<StmtPtr> bare_block();
	// Parses function call arguments and makes a Call object
	// Like: arguments?)
	ExprPtr finish_call(ExprPtr callee);

	const std::vector<Token> tokens;
	std::vector<Token>::size_type current = 0;
};

#endif
