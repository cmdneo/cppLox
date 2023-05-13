#include <cassert>
#include <cstddef>
#include <variant>
#include <initializer_list>
#include <string>

#include "runtime_error.hxx"
#include "token.hxx"
#include "token_type.hxx"
#include "expr.hxx"
#include "interpreter.hxx"

#define NUMBER_BINOP(left, right, op_token) \
	Primitive(std::get<double>(left) op_token std::get<double>(right))

using enum TokenType;
using std::get;
using std::holds_alternative;
using std::string;

static bool is_truthy(const Primitive &lit)
{
	if (holds_alternative<std::nullptr_t>(lit))
		return false;

	if (holds_alternative<bool>(lit))
		return std::get<bool>(lit);

	return true;
}

static void check_number_operand(const Token &op, const Primitive &right)
{
	if (holds_alternative<double>(right))
		return;

	throw RuntimeError(op, "Operand must be a number");
}

static void check_number_operands(
	const Token &op, const Primitive &left, const Primitive &right
)
{
	if (holds_alternative<double>(left) && holds_alternative<double>(right))
		return;

	throw RuntimeError(op, "Operands must be a number");
}

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
	auto left = evaluate(*expr.left);
	auto right = evaluate(*expr.right);

	switch (expr.operat.type) {
	case PLUS:
		if (holds_alternative<double>(left)
			&& holds_alternative<double>(right)) {
			return NUMBER_BINOP(left, right, +);
		}
		if (holds_alternative<string>(left)
			&& holds_alternative<string>(right)) {
			return Primitive(get<string>(left) + get<string>(right));
		}

		throw RuntimeError(
			expr.operat, "Operands must be two strings or two numbers."
		);
		break;

	case MINUS:
		check_number_operands(expr.operat, left, right);
		return NUMBER_BINOP(left, right, -);
	case STAR:
		check_number_operands(expr.operat, left, right);
		return NUMBER_BINOP(left, right, *);
	case SLASH:
		return NUMBER_BINOP(left, right, /);

	case EQUAL_EQUAL:
		return left == right;
	case BANG_EQUAL:
		return left != right;
	case GREATER:
		check_number_operands(expr.operat, left, right);
		return NUMBER_BINOP(left, right, >);
	case GREATER_EQUAL:
		check_number_operands(expr.operat, left, right);
		return NUMBER_BINOP(left, right, >=);
	case LESS:
		check_number_operands(expr.operat, left, right);
		return NUMBER_BINOP(left, right, <);
	case LESS_EQUAL:
		check_number_operands(expr.operat, left, right);
		return NUMBER_BINOP(left, right, <=);

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