#include <cassert>
#include <memory>
#include <vector>
#include <utility>
#include <typeinfo>
#include <initializer_list>

#include "token.hxx"
#include "token_type.hxx"
#include "expr.hxx"
#include "parser.hxx"

using enum TokenType;
using std::make_unique;

// CodeGen macro for parsing binary expressions
#define BINARY_EXPR(next_rule_method, ...)                 \
	{                                                      \
		using TypeList = std::initializer_list<TokenType>; \
		auto expr = next_rule_method();                    \
                                                           \
		while (match(TypeList{__VA_ARGS__})) {             \
			auto op = previous();                          \
			auto right = next_rule_method();               \
			expr = std::make_unique<Binary>(               \
				std::move(expr), op, std::move(right)      \
			);                                             \
		}                                                  \
                                                           \
		return expr;                                       \
	}

// Statement parsing
//---------------------------------------------------------

StmtPtr Parser::declaration()
{
	try {
		if (match({VAR}))
			return var_declaration();
		return statement();
	} catch (ParseError &) {
		synchronize();
		return nullptr;
	}

	assert(!"Unreachable code");
}

StmtPtr Parser::var_declaration()
{
	auto name = consume(IDENTIFIER, "Expect a variable name.");

	// Nil is the default value represented by type nullptr_t
	ExprPtr init = make_unique<Literal>(nullptr);

	if (match({EQUAL}))
		init = expression();

	consume(SEMICOLON, "Expect ';' after variable declaration.");
	return make_unique<Var>(name, std::move(init));
}

StmtPtr Parser::statement()
{
	if (match({ASSERT}))
		return assert_statement();
	if (match({PRINT}))
		return print_statement();
	if (match({LEFT_BRACE}))
		return block();

	return expression_statement();
}

StmtPtr Parser::assert_statement()
{
	auto expr = expression();
	consume(SEMICOLON, "Expect ';' after expression.");
	return make_unique<Assert>(previous(), std::move(expr));
}

StmtPtr Parser::print_statement()
{
	auto expr = expression();
	consume(SEMICOLON, "Expect ';' after expression.");
	return make_unique<Print>(std::move(expr));
}

StmtPtr Parser::block() { return make_unique<Block>(bare_block()); }

StmtPtr Parser::expression_statement()
{
	auto expr = expression();
	consume(SEMICOLON, "Expect ';' after expression.");
	return make_unique<Expression>(std::move(expr));
}

std::vector<StmtPtr> Parser::bare_block()
{
	std::vector<StmtPtr> statements;

	while (!check(RIGHT_BRACE) && !is_at_end())
		statements.push_back(declaration());

	consume(RIGHT_BRACE, "Expect '}' after block.");
	return statements;
}

// Expression parsing
//---------------------------------------------------------

ExprPtr Parser::expression() { return assignment(); }

ExprPtr Parser::assignment()
{
	auto expr = ternary();

	if (match({EQUAL})) {
		auto equals = previous();
		auto value = assignment();

		try {
			auto name = dynamic_cast<Variable &>(*expr).name;
			return make_unique<Assign>(name, std::move(value));
		} catch (std::bad_cast &) {
			print_error(equals, "Invalid assignment target.");
		}
	}

	return expr;
}

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

	if (match({IDENTIFIER}))
		return make_unique<Variable>(previous());

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