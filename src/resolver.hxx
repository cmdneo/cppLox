#ifndef RESOLVER_HXX_INCLUDED
#define RESOLVER_HXX_INCLUDED

#include <vector>
#include <string>
#include <map>
#include <iostream>

#include "error.hxx"
#include "token.hxx"
#include "expr.hxx"
#include "stmt.hxx"

class Interpreter;

class Resolver : private StmtVisitor, private ExprVisitor
{

public:
	Resolver(Interpreter &interpreter_)
		: interpreter(interpreter_)
	{
	}

	void resolve(const std::vector<StmtPtr> &statements)
	{
		for (auto &stmt : statements) {
			resolve(*stmt);
		}
	}

	void visit_block_stmt(const Block &stmt) override
	{
		begin_scope();
		resolve(stmt.statements);
		end_scope();
	}

	void visit_var_stmt(const Var &stmt) override
	{
		declare(stmt.name);
		resolve(*stmt.initializer);
		define(stmt.name);
	}

	void visit_class_stmt(const Class &stmt) override
	{
		auto enclosing_class = current_class;
		current_class = ClassType::Class;

		declare(stmt.name);
		define(stmt.name);

		begin_scope();
		scopes.back()["this"] = true;

		for (auto &method : stmt.methods) {
			auto declaration = FunctionType::Method;
			resolve_function(method, declaration);
		}

		end_scope();
		current_class = enclosing_class;
	}

	void visit_function_stmt(const Function &stmt) override
	{
		declare(stmt.name);
		define(stmt.name);
		resolve_function(stmt, FunctionType::Function);
	}

	void visit_expr_stmt(const Expression &stmt) override
	{
		resolve(*stmt.expression);
	}

	void visit_return_stmt(const Return &stmt) override
	{
		if (current_function == FunctionType::None)
			print_error((stmt.keyword), "Return statement outside function.");

		resolve(*stmt.value);
	}

	void visit_if_stmt(const If &stmt) override
	{
		resolve(*stmt.condition);
		resolve(*stmt.then_branch);
		if (stmt.else_branch != nullptr)
			resolve(*stmt.else_branch);
	}

	void visit_while_stmt(const While &stmt) override
	{
		resolve(*stmt.condition);

		auto enclosing_loop = current_loop;
		current_loop = LoopType::While;
		resolve(*stmt.body);
		current_loop = enclosing_loop;
	}

	void visit_assert_stmt(const Assert &stmt) override
	{
		resolve(*stmt.expression);
	}

	void visit_print_stmt(const Print &stmt) override
	{
		resolve(*stmt.expression);
	}

	void visit_break_stmt(const Break &stmt) override
	{
		if (current_loop == LoopType::None)
			print_error(stmt.keyword, "break statement outside loop.");
	}

	void visit_continue_stmt(const Continue &stmt) override
	{
		if (current_loop == LoopType::None)
			print_error(stmt.keyword, "continue statement outside loop.");
	}

	Object visit_variable_expr(const Variable &expr) override
	{
		if (!scopes.empty() && scopes.back().contains(expr.name.lexeme)
			&& scopes.back()[expr.name.lexeme] == false) {
			print_error(
				expr.name, "Can't read local variable in its own initializer."
			);
		}

		resolve_local(expr, expr.name);
		return nullptr;
	}

	Object visit_assign_expr(const Assign &expr) override
	{
		resolve(*expr.expression);
		resolve_local(expr, expr.name);
		return nullptr;
	}

	Object visit_ternary_expr(const Ternary &expr) override
	{
		resolve(*expr.condition);
		resolve(*expr.true_expr);
		resolve(*expr.false_expr);
		return nullptr;
	}

	Object visit_logical_expr(const Logical &expr) override
	{
		resolve(*expr.left);
		resolve(*expr.right);
		return nullptr;
	}

	Object visit_binary_expr(const Binary &expr) override
	{
		resolve(*expr.left);
		resolve(*expr.right);
		return nullptr;
	}

	Object visit_call_expr(const Call &expr) override
	{
		resolve(*expr.callee);
		for (auto &arg : expr.arguments)
			resolve(*arg);
		return nullptr;
	}

	Object visit_get_expr(const Get &expr) override
	{
		resolve(*expr.object);
		return nullptr;
	}

	Object visit_set_expr(const Set &expr) override
	{
		resolve(*expr.value);
		resolve(*expr.object);
		return nullptr;
	}

	Object visit_this_expr(const This &expr) override
	{
		if (current_class == ClassType::None) {
			print_error(expr.keyword, "Can't use 'this' outside of a class.");
			return nullptr;
		}

		resolve_local(expr, expr.keyword);
		return nullptr;
	}

	Object visit_grouping_expr(const Grouping &expr) override
	{
		resolve(*expr.expression);
		return nullptr;
	}

	Object visit_unary_expr(const Unary &expr) override
	{
		resolve(*expr.right);
		return nullptr;
	}

	Object visit_literal_expr(const Literal &) override { return nullptr; }

private:
	enum class ClassType { None, Class };
	enum class FunctionType { None, Function, Method };
	enum class LoopType { None, While };

	// Just const_cast instead of sticking const in every accept method
	void resolve(const Stmt &stmt) { const_cast<Stmt &>(stmt).accept(*this); }

	void resolve(const Expr &expr) { const_cast<Expr &>(expr).accept(*this); }

	void begin_scope() { scopes.push_back({}); }

	void end_scope() { scopes.pop_back(); }

	void declare(const Token &name)
	{
		if (scopes.empty())
			return;
		if (scopes.back().contains(name.lexeme)) {
			print_error(
				name, "Already a variable with this name in this scope."
			);
		}

		scopes.back()[name.lexeme] = false;
	}

	void define(const Token &name)
	{
		if (scopes.empty())
			return;
		scopes.back()[name.lexeme] = true;
	}

	// Resolves a funtion, by introducing its parameters in the current scope
	void resolve_function(const Function &function, FunctionType type)
	{
		auto enclosing_function = current_function;
		current_function = type;
		begin_scope();

		for (auto &param : function.params) {
			declare(param);
			define(param);
		}
		resolve(*function.body);

		end_scope();
		current_function = enclosing_function;
	}

	void resolve_local(const Expr &expr, const Token &name);

	std::vector<std::map<const std::string, bool>> scopes;
	Interpreter &interpreter;
	// Keeps track of if we are inside a class/function/loop
	ClassType current_class = ClassType::None;
	FunctionType current_function = FunctionType::None;
	LoopType current_loop = LoopType::None;
};

#endif
