#include <cassert>
#include <memory>
#include <utility>
#include <initializer_list>

#include "token.hxx"
#include "token_type.hxx"
#include "expr.hxx"
#include "parser.hxx"

using enum TokenType;
using std::make_unique;

// CodeGen macro (tried templates :( ).
#define BINARY_EXPR(mem_func_name, ...)                    \
	{                                                      \
		using TypeList = std::initializer_list<TokenType>; \
		auto expr = mem_func_name();                       \
                                                           \
		while (match(TypeList{__VA_ARGS__})) {             \
			auto op = previous();                          \
			auto right = mem_func_name();                  \
			expr = std::make_unique<Binary>(               \
				std::move(expr), op, std::move(right)      \
			);                                             \
		}                                                  \
                                                           \
		return expr;                                       \
	}

ExprPtr Parser::expression() { return ternary(); }

ExprPtr Parser::ternary()
{
	auto expr = equality();

	while (match({QUESTION})) {
		auto expr1 = expression();
		consume(COLON, "Expect colon in ternary expression.");
		auto expr2 = ternary();
		return make_unique<Ternary>(
			std::move(expr), std::move(expr1), std::move(expr2)
		);
	}

	return expr;
}

ExprPtr Parser::equality() { BINARY_EXPR(comparison, EQUAL_EQUAL, BANG_EQUAL); }

ExprPtr Parser::comparison()
{
	BINARY_EXPR(term, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL);
}

ExprPtr Parser::term() { BINARY_EXPR(factor, PLUS, MINUS); }

ExprPtr Parser::factor() { BINARY_EXPR(unary, SLASH, STAR); }

ExprPtr Parser::unary()
{
	if (match({BANG, PLUS, MINUS})) {
		Token op = previous();
		auto right = unary();
		return make_unique<Unary>(op, std::move(right));
	}

	return primary();
}

ExprPtr Parser::primary()
{
	if (match({FALSE}))
		return make_unique<Literal>(false);
	if (match({TRUE}))
		return make_unique<Literal>(true);
	if (match({NIL}))
		return make_unique<Literal>(nullptr);

	if (match({NUMBER, STRING}))
		return make_unique<Literal>(previous().literal);

	if (match({LEFT_PAREN})) {
		auto expr = expression();
		consume(RIGHT_PAREN, "Expect ')' after expression.");
		return make_unique<Grouping>(std::move(expr));
	}

	throw make_error(peek(), "Expect expression.");
}

void Parser::synchronize()
{
	advance();

	while (!is_at_end()) {
		if (previous().type == SEMICOLON)
			return;

		switch (peek().type) {
		case CLASS:
		case FUN:
		case VAR:
		case FOR:
		case IF:
		case WHILE:
		case PRINT:
		case RETURN:
			return;

		default:
			break;
		}

		advance();
	}
}