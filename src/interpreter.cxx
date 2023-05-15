#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <variant>
#include <string>
#include <utility>

#include "runtime_error.hxx"
#include "token_type.hxx"
#include "token.hxx"
#include "expr.hxx"
#include "environment.hxx"
#include "interpreter.hxx"

using enum TokenType;
using std::get;
using std::make_shared;
using std::string;

// Helper functions and macros
//---------------------------------------------------------

#define RETURN_NUMBER_BINOP(left, right, op_token)                       \
	do {                                                                 \
		return Primitive(get<double>(left) op_token get<double>(right)); \
	} while (0)

#define RETURN_NUMBER_OR_STRING_BINOP(left, right, op_token)                 \
	do {                                                                     \
		if (match_types<double, double>(left, right)) {                      \
			return Primitive(get<double>(left) op_token get<double>(right)); \
		}                                                                    \
		if (match_types<string, string>(left, right)) {                      \
			return Primitive(get<string>(left) op_token get<string>(right)); \
		}                                                                    \
	} while (0)

// Returns true if all primitives hold the value of the corresponding given type.
// Precisely: If for every primitive
// nth primitive holds the value of the type represented
// by the nth item of MatchTypes, then return true.
template <typename... MatchTypes, int pack_barrier = 0, typename... Ps>
static inline bool match_types(const Ps &...primitives)
{
	bool all_match = true;

	(
		[&] {
			if (!std::holds_alternative<MatchTypes>(primitives))
				all_match = false;
		}(),
		...
	);

	return all_match;
}

static bool is_truthy(const Primitive &lit)
{
	if (std::holds_alternative<std::nullptr_t>(lit))
		return false;

	if (std::holds_alternative<bool>(lit))
		return std::get<bool>(lit);

	return true;
}

static void check_number_operand(const Token &op, const Primitive &right)
{
	if (match_types<double>(right))
		return;

	throw RuntimeError(op, "Operand must be a number");
}

static void check_number_operands(
	const Token &op, const Primitive &left, const Primitive &right
)
{
	if (match_types<double, double>(left, right))
		return;

	throw RuntimeError(op, "Operands must be a number");
}

// Interpreter methos
//---------------------------------------------------------

void Interpreter::interpret(std::vector<StmtPtr> statements)
{
	try {
		for (auto &stmt : statements)
			execute(*stmt);
	} catch (RuntimeError &err) {
		print_runtime_error(err);
	}
}

// Statement visitor methods
//-----------------------------------------------

void Interpreter::visit_assert_stmt(Assert &stmt)
{
	if (!is_truthy(evaluate(*stmt.expression)))
		throw RuntimeError(stmt.token, "Assertion failed.");
}

void Interpreter::visit_print_stmt(Print &stmt)
{
	auto value = evaluate(*stmt.expression);
	std::cout << to_string(value) << '\n';
}

void Interpreter::visit_expr_stmt(Expression &stmt)
{
	last_expr_result = evaluate(*stmt.expression);
}

void Interpreter::visit_block_stmt(Block &stmt)
{
	execute_block(stmt.statements, make_shared<Environment>(environment));
}

void Interpreter::visit_var_stmt(Var &stmt)
{
	auto value = evaluate(*stmt.initializer);
	environment->define(stmt.name.lexeme, value);
}

// Expression visitor methods
//-----------------------------------------------

VisResult Interpreter::visit_literal_expr(Literal &expr) { return expr.value; }

VisResult Interpreter::visit_grouping_expr(Grouping &expr)
{
	return evaluate(*expr.expression);
}

VisResult Interpreter::visit_unary_expr(Unary &expr)
{
	auto right = evaluate(*expr.right);

	switch (expr.operat.type) {
	case BANG:
		return Primitive(!is_truthy(right));
	case PLUS:
		check_number_operand(expr.operat, right);
		return Primitive(get<double>(right));
	case MINUS:
		check_number_operand(expr.operat, right);
		return Primitive(-get<double>(right));

	default:
		break;
	}

	assert(!"Unreachable code");
}

VisResult Interpreter::visit_binary_expr(Binary &expr)
{
	const auto STRING_OR_NUMBER_EXPECTED = RuntimeError(
		expr.operat, "Operands must be two strings or two numbers."
	);

	auto left = evaluate(*expr.left);
	auto right = evaluate(*expr.right);

	switch (expr.operat.type) {
	case PLUS:
		RETURN_NUMBER_OR_STRING_BINOP(left, right, +);
		throw STRING_OR_NUMBER_EXPECTED;
		break;

	case MINUS:
		check_number_operands(expr.operat, left, right);
		RETURN_NUMBER_BINOP(left, right, -);
	case STAR:
		check_number_operands(expr.operat, left, right);
		RETURN_NUMBER_BINOP(left, right, *);
	case SLASH:
		check_number_operands(expr.operat, left, right);
		RETURN_NUMBER_BINOP(left, right, /);

	case EQUAL_EQUAL:
		return left == right;
	case BANG_EQUAL:
		return left != right;

	case GREATER:
		RETURN_NUMBER_OR_STRING_BINOP(left, right, >);
		throw STRING_OR_NUMBER_EXPECTED;
	case GREATER_EQUAL:
		RETURN_NUMBER_OR_STRING_BINOP(left, right, >=);
		throw STRING_OR_NUMBER_EXPECTED;
	case LESS:
		RETURN_NUMBER_OR_STRING_BINOP(left, right, <);
		throw STRING_OR_NUMBER_EXPECTED;
	case LESS_EQUAL:
		RETURN_NUMBER_OR_STRING_BINOP(left, right, <=);
		throw STRING_OR_NUMBER_EXPECTED;

	default:
		break;
	}

	assert(!"Unreachable code");
}

VisResult Interpreter::visit_ternary_expr(Ternary &expr)
{
	bool res = is_truthy(evaluate(*expr.condition));
	return evaluate(*(res ? expr.expr1 : expr.expr2));
}

VisResult Interpreter::visit_variable_expr(Variable &expr)
{
	return environment->get(expr.name);
}

VisResult Interpreter::visit_assign_expr(Assign &expr)
{
	auto value = evaluate(*expr.expression);
	environment->assign(expr.name, value);
	return value;
}

void Interpreter::execute_block(
	std::vector<StmtPtr> &statements, EnvironmentPtr block_environ
)
{
	EnvironmentPtr previous = environment;

	try {
		environment = block_environ;

		for (const auto &stmt : statements)
			execute(*stmt);
	} catch (RuntimeError &err) {
		environment = previous;
		throw err;
	}

	environment = previous;
}