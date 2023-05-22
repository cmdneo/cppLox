#include <cassert>
#include <cstddef>
#include <iostream>
#include <format>
#include <variant>
#include <string>
#include <utility>

#include "runtime_error.hxx"
#include "token_type.hxx"
#include "token.hxx"
#include "object.hxx"
#include "expr.hxx"
#include "stmt.hxx"
#include "environment.hxx"
#include "interpreter.hxx"
#include "native.hxx"
#include "lox_function.hxx"

using enum TokenType;
using std::get;
using std::make_shared;
using std::make_unique;
using std::string;

// Helper functions and macros
//---------------------------------------------------------

#define RETURN_NUMBER_BINOP(left, right, op_token)                    \
	do {                                                              \
		return Object(get<double>(left) op_token get<double>(right)); \
	} while (0)

#define RETURN_NUMBER_OR_STRING_BINOP(left, right, op_token)              \
	do {                                                                  \
		if (match_types<double, double>(left, right)) {                   \
			return Object(get<double>(left) op_token get<double>(right)); \
		}                                                                 \
		if (match_types<string, string>(left, right)) {                   \
			return Object(get<string>(left) op_token get<string>(right)); \
		}                                                                 \
	} while (0)

static bool is_truthy(const Object &lit)
{
	if (match_types<std::nullptr_t>(lit))
		return false;

	if (match_types<bool>(lit))
		return std::get<bool>(lit);

	return true;
}

static void check_number_operand(const Token &op, const Object &right)
{
	if (match_types<double>(right))
		return;

	throw RuntimeError(op, "Operand must be a number");
}

static void
check_number_operands(const Token &op, const Object &left, const Object &right)
{
	if (match_types<double, double>(left, right))
		return;

	throw RuntimeError(op, "Operands must be a number");
}

// Interpreter interface methods
//---------------------------------------------------------

Interpreter::Interpreter()
{
	globals->define("clock", make_shared<ClockFn>());
	globals->define("sleep", make_shared<SleepFn>());
	globals->define("string", make_shared<StringFn>());
}

void Interpreter::interpret(std::vector<StmtPtr> statements)
{
	try {
		for (auto &stmt : statements)
			execute(*stmt);
	} catch (RuntimeError &err) {
		print_runtime_error(err);
	} catch (NativeFnError &err) {
		print_nativefn_error(err);
	}
}

// Statement visitor methods
//-----------------------------------------------

void Interpreter::visit_assert_stmt(const Assert &stmt)
{
	if (!is_truthy(evaluate(*stmt.expression)))
		throw RuntimeError(stmt.token, "Assertion failed.");
}

void Interpreter::visit_print_stmt(const Print &stmt)
{
	auto value = evaluate(*stmt.expression);
	std::cout << to_string(value) << '\n';
}

void Interpreter::visit_break_stmt(const Break &) { throw ControlBreak(); }

void Interpreter::visit_continue_stmt(const Continue &)
{
	throw ControlContinue();
}

void Interpreter::visit_return_stmt(const Return &stmt)
{
	throw ControlReturn(evaluate(*stmt.value));
}

void Interpreter::visit_expr_stmt(const Expression &stmt)
{
	last_expr_result = evaluate(*stmt.expression);
}

void Interpreter::visit_block_stmt(const Block &stmt)
{
	execute_block(stmt.statements, make_shared<Environment>(environment));
}

void Interpreter::visit_if_stmt(const If &stmt)
{
	if (is_truthy(evaluate(*stmt.condition)))
		execute(*stmt.then_branch);
	else if (stmt.else_branch != nullptr)
		execute(*stmt.else_branch);
}

void Interpreter::visit_while_stmt(const While &stmt)
{
	while (is_truthy(evaluate(*stmt.condition))) {
		try {
			execute(*stmt.body);
		} catch (ControlBreak) {
			break;
		} catch (ControlContinue) {
			continue;
		}
	}
}

void Interpreter::visit_var_stmt(const Var &stmt)
{
	auto value = evaluate(*stmt.initializer);
	environment->define(stmt.name.lexeme, value);
}

void Interpreter::visit_function_stmt(const Function &stmt)
{
	CallablePtr function = make_unique<LoxFunction>(stmt, environment);
	environment->define(stmt.name.lexeme, std::move(function));
}

// Expression visitor methods
//-----------------------------------------------

Object Interpreter::visit_literal_expr(const Literal &expr)
{
	return expr.value;
}

Object Interpreter::visit_grouping_expr(const Grouping &expr)
{
	return evaluate(*expr.expression);
}

Object Interpreter::visit_call_expr(const Call &expr)
{
	auto callee = evaluate(*expr.callee);

	std::vector<Object> arguments;
	for (auto &arg : expr.arguments)
		arguments.push_back(evaluate(*arg));

	if (!match_types<CallablePtr>(callee))
		throw RuntimeError(expr.paren, "Can only call functions and classes.");
	auto function = get<CallablePtr>(callee);

	if (arguments.size() != function->arity()) {
		auto err_msg = std::format(
			"Expected {} arguments but got {} arguments.", function->arity(),
			arguments.size()
		);
		throw RuntimeError(expr.paren, err_msg);
	}

	return function->call(*this, arguments);
}

Object Interpreter::visit_unary_expr(const Unary &expr)
{
	auto right = evaluate(*expr.right);

	switch (expr.operat.type) {
	case BANG:
		return Object(!is_truthy(right));
	case PLUS:
		check_number_operand(expr.operat, right);
		return Object(get<double>(right));
	case MINUS:
		check_number_operand(expr.operat, right);
		return Object(-get<double>(right));

	default:
		break;
	}

	assert(!"Unreachable code");
	return nullptr;
}

Object Interpreter::visit_binary_expr(const Binary &expr)
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
	return nullptr;
}

Object Interpreter::visit_logical_expr(const Logical &expr)
{
	auto left = evaluate(*expr.left);

	if (expr.operat.type == OR) {
		if (is_truthy(left))
			return left;
	} else {
		if (!is_truthy(left))
			return left;
	}

	return evaluate(*expr.right);
}

Object Interpreter::visit_ternary_expr(const Ternary &expr)
{
	bool res = is_truthy(evaluate(*expr.condition));
	return evaluate(*(res ? expr.expr1 : expr.expr2));
}

Object Interpreter::visit_variable_expr(const Variable &expr)
{
	return environment->get(expr.name);
}

Object Interpreter::visit_assign_expr(const Assign &expr)
{
	auto value = evaluate(*expr.expression);
	environment->assign(expr.name, value);
	return value;
}

void Interpreter::execute_block(
	const std::vector<StmtPtr> &statements, EnvironmentPtr block_environ
)
{
	auto previous = environment;

	// If any expected exceptions are encountered, then handle them,
	// restore the environment and then rethrow the handeled exception.
	// All this, because C++ doesn't have a finally clause like other languages.
	try {
		environment = block_environ;

		for (const auto &stmt : statements)
			execute(*stmt);
	} catch (RuntimeError &err) {
		environment = previous;
		throw err;
	}
	// Here, we are not subclassing all control-flow exceptions under
	// a single base-class and using that to catch all subclasses.
	// Because, then if we will rethrow the exception we will lose the type
	// of the subclass and things will get complicated. So, just keep it simple.
	catch (ControlBreak &err) {
		environment = previous;
		throw err;
	} catch (ControlContinue &err) {
		environment = previous;
		throw err;
	} catch (ControlReturn &err) {
		environment = previous;
		throw err;
	}

	environment = previous;
}
