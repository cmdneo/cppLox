#include <cassert>
#include <cstddef>
#include <format>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <variant>

#include "runtime_error.hxx"
#include "token_type.hxx"
#include "token.hxx"
#include "object.hxx"
#include "expr.hxx"
#include "stmt.hxx"
#include "environment.hxx"
#include "interpreter.hxx"
#include "native.hxx"
#include "lox_callable.hxx"
#include "lox_function.hxx"
#include "lox_class.hxx"
#include "lox_instance.hxx"

using enum TokenType;
using std::get;
using std::make_shared;
using std::make_unique;
using std::string;

// Helper functions and macros
//---------------------------------------------------------

// Calculates and returns the result, both operands should be numbers.
#define RETURN_NUMBER_BINOP(left, right, op_token)                    \
	do {                                                              \
		return Object(get<double>(left) op_token get<double>(right)); \
	} while (0)

// Operates and returns the result if both operands are numbers or
// both operands are strings, otherwise does nothing.
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
	globals->define("instance_of", make_shared<InstanceOfFn>());
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
	if (stmt.value == nullptr)
		throw ControlReturn(Object(nullptr));
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
	auto klass = make_shared<Variable>(stmt.name);
	environment->define(stmt.name.lexeme, value);
}

void Interpreter::visit_function_stmt(const Function &stmt)
{
	LoxCallablePtr function = make_shared<LoxFunction>(stmt, environment);
	environment->define(stmt.name.lexeme, std::move(function));
}

void Interpreter::visit_class_stmt(const Class &stmt)
{
	environment->define(stmt.name.lexeme, nullptr);

	// If a superclass name exists and it is an Object of type LoxClass
	LoxClassPtr superclass = nullptr;
	if (stmt.superclass != nullptr) {
		auto maybe_class = evaluate(*stmt.superclass);
		if (match_types<LoxClassPtr>(maybe_class)) {
			superclass = get<LoxClassPtr>(maybe_class);
		} else {
			throw RuntimeError(
				stmt.superclass->name, "Superclass must be a class."
			);
		}
	}

	// The enclosing environment in which 'super' is defined always remains
	// the same because it only used to access methods and methods remain the
	// same for every instance of a class, unlike instance's data-fields.
	if (stmt.superclass != nullptr) {
		environment = make_shared<Environment>(environment);
		environment->define("super", superclass);
	}

	// A new enclosing environment is created and 'this' defined in that
	// when we access the method of an instance, and not here.
	// Since every instances do not share data-fields.
	ClassMethodMap methods;
	for (auto &method : stmt.methods) {
		bool is_init = method.name.lexeme == "init";
		methods.insert({
			method.name.lexeme,
			make_unique<LoxFunction>(method, environment, is_init),
		});
	}

	auto klass = make_shared<LoxClass>(
		stmt.name.lexeme, std::move(superclass), std::move(methods)
	);
	klass->self_ptr = klass;

	// Pop the environment in which 'super' was defined.
	if (stmt.superclass != nullptr)
		environment = environment->enclosing;

	environment->assign(stmt.name, std::move(klass));
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

	LoxCallablePtr function = nullptr;
	if (match_types<LoxCallablePtr>(callee))
		function = get<LoxCallablePtr>(callee);
	else if (match_types<LoxClassPtr>(callee))
		function = get<LoxClassPtr>(callee);
	else
		throw RuntimeError(expr.paren, "Can only call functions and classes.");

	if (arguments.size() != function->arity()) {
		auto err_msg = std::format(
			"Expected {} arguments but got {} arguments.", function->arity(),
			arguments.size()
		);
		throw RuntimeError(expr.paren, err_msg);
	}

	return function->call(*this, arguments);
}

Object Interpreter::visit_get_expr(const Get &expr)
{
	auto object = evaluate(*expr.object);
	if (!match_types<LoxInstancePtr>(object))
		throw RuntimeError(expr.name, "Only instances have properties.");

	return get<LoxInstancePtr>(object)->get(expr.name);
}

Object Interpreter::visit_set_expr(const Set &expr)
{
	auto object = evaluate(*expr.object);
	if (!match_types<LoxInstancePtr>(object))
		throw RuntimeError(expr.name, "Only instances have fields.");

	auto value = evaluate(*expr.value);
	get<LoxInstancePtr>(object)->set(expr.name, value);
	return value;
}

Object Interpreter::visit_super_expr(const Super &expr)
{
	auto distance = locals.at(&expr);
	auto superclass = get<LoxClassPtr>(environment->get_at(distance, "super"));
	// 'this' resides inside the scope which is nested inside the scope
	// in which 'super' resides.
	auto object =
		get<LoxInstancePtr>(environment->get_at(distance - 1, "this"));

	auto method = superclass->find_method(expr.method.lexeme);
	if (method == nullptr) {
		throw RuntimeError(
			expr.method,
			std::format("Undefined property '{}'", expr.method.lexeme)
		);
	}

	return method->bind(std::move(object));
}

Object Interpreter::visit_this_expr(const This &expr)
{
	return look_up_variable(expr.keyword, expr);
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
	return evaluate(*(res ? expr.true_expr : expr.false_expr));
}

Object Interpreter::visit_variable_expr(const Variable &expr)
{
	return look_up_variable(expr.name, expr);
}

Object Interpreter::visit_assign_expr(const Assign &expr)
{
	auto value = evaluate(*expr.expression);
	auto result = locals.find(&expr);
	if (result != locals.end()) {
		auto distance = result->second;
		environment->assign_at(distance, expr.name, value);
	} else {
		globals->assign(expr.name, value);
	}

	return value;
}

void Interpreter::execute_block(
	const std::vector<StmtPtr> &statements, EnvironmentPtr &&block_environ
)
{
	auto previous = std::move(environment);

	// We do not pop the environment before running the GC, because
	// the environment still may have Objects(that is LoxFunctions)
	// which refer to an enclosing environment.
	// If we pop the environment then the environments referred-to by the objects
	// in this environment may not get marked, which will result in clearing
	// those environments wrongly by the GC, but when returned those objects
	// will refer to a cleared environment and they(that is closures) may not
	// find the variables they expect to find in their enclosing scope.
	auto restore_environment = [&] {
		garbage_collector.collect();
		garbage_collector.pop_environment();
		environment = std::move(previous);
	};

	// Tell the garbage collector that the current environment is directly reachable
	garbage_collector.push_environment(block_environ);

	// If any expected exceptions are encountered, then handle them,
	// restore the environment and then rethrow the handeled exception.
	// All this, because C++ doesn't have a finally clause like other languages.
	try {
		environment = std::move(block_environ);

		for (const auto &stmt : statements)
			execute(*stmt);
	} catch (RuntimeError &err) {
		environment = previous;
		throw err;
	} catch (ControlBreak &err) {
		restore_environment();
		throw err;
	} catch (ControlContinue &err) {
		restore_environment();
		throw err;
	} catch (ControlReturn &err) {
		restore_environment();
		throw err;
	}

	restore_environment();
}
