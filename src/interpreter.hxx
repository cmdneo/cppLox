#ifndef INTERPRETER_HXX_INCLUDED
#define INTERPRETER_HXX_INCLUDED

#include <exception>
#include <variant>
#include <vector>
#include <string>
#include <memory>

#include "error.hxx"
#include "runtime_error.hxx"
#include "object.hxx"
#include "expr.hxx"
#include "stmt.hxx"
#include "environment.hxx"

class Interpreter : public ExprVisitor, StmtVisitor
{
	friend struct LoxFunction;

public:
	Interpreter();
	void interpret(std::vector<StmtPtr> statements);

	void visit_assert_stmt(const Assert &stmt) override;
	void visit_print_stmt(const Print &stmt) override;
	void visit_break_stmt(const Break &stmt) override;
	void visit_expr_stmt(const Expression &stmt) override;
	void visit_block_stmt(const Block &stmt) override;
	void visit_continue_stmt(const Continue &stmt) override;
	void visit_if_stmt(const If &stmt) override;
	void visit_while_stmt(const While &stmt) override;
	void visit_var_stmt(const Var &stmt) override;
	void visit_function_stmt(const Function &stmt) override;

	Object visit_literal_expr(const Literal &expr) override;
	Object visit_grouping_expr(const Grouping &expr) override;
	Object visit_call_expr(const Call &expr) override;
	Object visit_unary_expr(const Unary &expr) override;
	Object visit_binary_expr(const Binary &expr) override;
	Object visit_logical_expr(const Logical &expr) override;
	Object visit_ternary_expr(const Ternary &expr) override;
	Object visit_variable_expr(const Variable &expr) override;
	Object visit_assign_expr(const Assign &expr) override;

	// Stores the result of the last expression statement executed
	Object last_expr_result;

private:
	// Loop control-flow (exception)marker classes
	struct ControlBreak : public std::exception {
		ControlBreak() = default;
	};
	struct ControlContinue : public std::exception {
		ControlContinue() = default;
	};

	inline void execute(Stmt &stmt) { stmt.accept(*this); }

	inline Object evaluate(Expr &expr) { return expr.accept(*this); }

	void execute_block(
		const std::vector<StmtPtr> &statements, EnvironmentPtr block_environ
	);

	EnvironmentPtr globals = std::make_shared<Environment>();
	EnvironmentPtr environment = globals;
};

#endif
