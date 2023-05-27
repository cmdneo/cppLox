#include <cassert>
#include <format>
#include <memory>
#include <vector>
#include <utility>
#include <string_view>
#include <typeinfo>
#include <initializer_list>

#include "token.hxx"
#include "token_type.hxx"
#include "expr.hxx"
#include "parser.hxx"

using enum TokenType;
using std::format;
using std::make_unique;
using std::vector;

constexpr unsigned MAX_PARAMS = 255;

// CodeGen macro for parsing binary expressions
#define RETURN_BINARY_EXPR(expr_type, next_rule_method, ...) \
	do {                                                     \
		using TypeList = std::initializer_list<TokenType>;   \
		auto expr = next_rule_method();                      \
                                                             \
		while (match(TypeList{__VA_ARGS__})) {               \
			auto operat = previous();                        \
			auto right = next_rule_method();                 \
			expr = std::make_unique<expr_type>(              \
				std::move(expr), operat, std::move(right)    \
			);                                               \
		}                                                    \
                                                             \
		return expr;                                         \
	} while (0)

// Parser interface method
//---------------------------------------------------------

std::vector<StmtPtr> Parser::parse()
{
	vector<StmtPtr> statements;
	while (!is_at_end()) {
		try {
			statements.push_back(declaration());
		} catch (ParseError &e) {
			return {};
		}
	}

	return statements;
}

// Statement parsing
//---------------------------------------------------------

StmtPtr Parser::declaration()
{
	try {
		if (match({CLASS}))
			return class_declaration();
		if (match({FUN}))
			return make_unique<Function>(function("function"));
		if (match({VAR}))
			return var_declaration();

		return statement();
	} catch (ParseError &) {
		synchronize();
		return nullptr;
	}

	assert(!"Unreachable code");
	return nullptr;
}

StmtPtr Parser::class_declaration()
{
	auto name = consume(IDENTIFIER, "Expect class name.");
	consume(LEFT_BRACE, "Expect '{' before class body.");

	std::vector<Function> methods;
	while (!check(RIGHT_BRACE) && !is_at_end())
		methods.push_back(function("method"));

	consume(RIGHT_BRACE, "Expect '}' after class body.");
	return make_unique<Class>(name, std::move(methods));
}

Function Parser::function(std::string_view kind)
{
	auto name = consume(IDENTIFIER, format("Expect {} name.", kind));
	consume(LEFT_PAREN, format("Expect '(' after {} name.", kind));

	vector<Token> parameters;
	if (!check(RIGHT_PAREN)) {
		do {
			if (parameters.size() >= MAX_PARAMS) {
				print_error(
					peek(),
					format("Can't have more than {} parameters.", MAX_PARAMS)
				);
			}
			auto param = consume(IDENTIFIER, "Expect parameter name.");
			parameters.push_back(param);
		} while (match({COMMA}));
	}
	consume(RIGHT_PAREN, "Expect ')' after parameters.");

	consume(LEFT_BRACE, format("Expect '{{' before {} body.", kind));
	auto body = std::make_shared<std::vector<StmtPtr>>(bare_block());

	return Function(name, std::move(parameters), std::move(body));
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
	if (match({BREAK}))
		return break_statement();
	if (match({CONTINUE}))
		return continue_statement();
	if (match({RETURN}))
		return return_statement();
	if (match({IF}))
		return if_statement();
	if (match({WHILE}))
		return while_statement();
	if (match({FOR}))
		return for_statement();
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

StmtPtr Parser::break_statement()
{
	auto keyword = previous();
	consume(SEMICOLON, "Expect ';' after 'break'.");
	return make_unique<Break>(keyword);
}

StmtPtr Parser::continue_statement()
{
	auto keyword = previous();
	consume(SEMICOLON, "Expect ';' after 'continue'.");
	return make_unique<Continue>(keyword);
}

StmtPtr Parser::return_statement()
{
	auto keyword = previous();
	ExprPtr value = make_unique<Literal>(nullptr);
	if (!check(SEMICOLON))
		value = expression();

	consume(SEMICOLON, "Expect ';' after return value.");
	return make_unique<Return>(keyword, std::move(value));
}

StmtPtr Parser::if_statement()
{
	consume(LEFT_PAREN, "Expect '(' after if.");
	auto condition = expression();
	consume(RIGHT_PAREN, "Expect ')' after condition.");

	auto then_branch = statement();
	auto else_branch = match({ELSE}) ? statement() : nullptr;

	return make_unique<If>(
		std::move(condition), std::move(then_branch), std::move(else_branch)
	);
}

StmtPtr Parser::while_statement()
{
	consume(LEFT_PAREN, "Expect '(' after 'while'.");
	auto condition = expression();
	consume(RIGHT_PAREN, "Expect ')' after condition.");

	auto body = statement();
	return make_unique<While>(std::move(condition), std::move(body));
}

StmtPtr Parser::for_statement()
{
	consume(LEFT_PAREN, "Expect '(' after 'for'.");
	StmtPtr initializer;
	if (match({SEMICOLON}))
		initializer = nullptr;
	else if (match({VAR}))
		initializer = var_declaration();
	else
		initializer = expression_statement();

	ExprPtr condition = nullptr;
	if (!check(SEMICOLON))
		condition = expression();
	consume(SEMICOLON, "Expect ';' after loop condition.");

	ExprPtr increment = nullptr;
	if (!check(RIGHT_PAREN))
		increment = expression();
	consume(RIGHT_PAREN, "Expect ')' after for clauses.");
	auto body = statement();

	// A 'for' loop is just a syntactic sugar for the while loop.
	// The below two are equaivalent:
	//	for (initializer; condition; increment) { body }
	//	{ initializer; while (condition) { { body } increment; } }
	// So, convert the for loop to a while loop as per the above equivalence.
	if (increment != nullptr)
		body = make_block(
			std::move(body), make_unique<Expression>(std::move(increment))
		);

	if (condition == nullptr)
		condition = make_unique<Literal>(true);
	body = make_unique<While>(std::move(condition), std::move(body));

	if (initializer != nullptr)
		body = make_block(std::move(initializer), std::move(body));

	return body;
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
	vector<StmtPtr> statements;

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
	// Since the '=' can be any-number of tokens ahead,
	// parse the left hand side and then check for equal sign.
	// Then, check if the assingment target is valid.
	auto expr = ternary();

	if (match({EQUAL})) {
		auto equals = previous();
		auto value = assignment();

		// If Variable then just assign.
		if (typeid(expr.get()) == typeid(Variable &)) {
			auto name = dynamic_cast<Variable &>(*expr).name;
			return make_unique<Assign>(name, std::move(value));
		}
		// If Set(like: object.name) then transform it into a Get,
		// where the rightmost part(name) is the property to be set.
		else if (typeid(expr.get()) == typeid(Get &)) {
			auto &get = dynamic_cast<Get &>(*expr);
			return make_unique<Set>(
				std::move(get.object), get.name, std::move(value)
			);
		} else {
			print_error(equals, "Invalid assignment target.");
		}
	}

	return expr;
}

ExprPtr Parser::ternary()
{
	auto expr = logic_or();

	while (match({QUESTION})) {
		auto true_expr = expression();
		consume(COLON, "Expect colon in ternary expression.");
		auto false_expr = ternary();
		return make_unique<Ternary>(
			std::move(expr), std::move(true_expr), std::move(false_expr)
		);
	}

	return expr;
}

ExprPtr Parser::logic_or() { RETURN_BINARY_EXPR(Logical, logic_and, OR); }

ExprPtr Parser::logic_and() { RETURN_BINARY_EXPR(Logical, equality, AND); }

ExprPtr Parser::equality()
{
	RETURN_BINARY_EXPR(Binary, comparison, EQUAL_EQUAL, BANG_EQUAL);
}

ExprPtr Parser::comparison()
{
	RETURN_BINARY_EXPR(Binary, term, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL);
}

ExprPtr Parser::term() { RETURN_BINARY_EXPR(Binary, factor, PLUS, MINUS); }

ExprPtr Parser::factor() { RETURN_BINARY_EXPR(Binary, unary, SLASH, STAR); }

ExprPtr Parser::unary()
{
	if (match({BANG, PLUS, MINUS})) {
		Token op = previous();
		auto right = unary();
		return make_unique<Unary>(op, std::move(right));
	}

	return call();
}

ExprPtr Parser::call()
{
	auto expr = primary();

	while (true) {
		if (match({DOT})) {
			auto name = consume(IDENTIFIER, "Expect property name after '.'.");
			expr = make_unique<Get>(std::move(expr), name);
		} else if (match({LEFT_PAREN})) {
			expr = finish_call(std::move(expr));
		} else {
			break;
		}
	}

	return expr;
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

	if (match({THIS}))
		return make_unique<This>(previous());

	if (match({IDENTIFIER}))
		return make_unique<Variable>(previous());

	if (match({LEFT_PAREN})) {
		auto expr = expression();
		consume(RIGHT_PAREN, "Expect ')' after expression.");
		return make_unique<Grouping>(std::move(expr));
	}

	throw make_error(peek(), "Expect expression.");
}

ExprPtr Parser::finish_call(ExprPtr callee)
{
	vector<ExprPtr> arguments;

	if (!check(RIGHT_PAREN)) {
		do {
			if (arguments.size() >= MAX_PARAMS)
				make_error(
					peek(),
					format("Can't have more than {} arguments.", MAX_PARAMS)
				);
			// Continue even after the above error,
			// because the parser is still in a valid known state
			arguments.push_back(expression());
		} while (match({COMMA}));
	}

	auto paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");
	return make_unique<Call>(std::move(callee), paren, std::move(arguments));
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