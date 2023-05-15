#ifndef INTERPRETER_HXX_INCLUDED
#define INTERPRETER_HXX_INCLUDED

#include <variant>
#include <vector>
#include <string>
#include <memory>

#include "error.hxx"
#include "runtime_error.hxx"
#include "token.hxx"
#include "expr.hxx"
#include "stmt.hxx"
#include "environment.hxx"

class Interpreter : public ExprVisitor, StmtVisitor
{
public:
	void interpret(std::vector<StmtPtr> statements);

	void visit_assert_stmt(Assert &stmt) override;
	void visit_print_stmt(Print &stmt) override;
	void visit_expr_stmt(Expression &stmt) override;
	void visit_block_stmt(Block &stmt) override;
	void visit_var_stmt(Var &stmt) override;

	VisResult visit_literal_expr(Literal &expr) override;
	VisResult visit_grouping_expr(Grouping &expr) override;
	VisResult visit_unary_expr(Unary &expr) override;
	VisResult visit_binary_expr(Binary &expr) override;
	VisResult visit_ternary_expr(Ternary &expr) override;
	VisResult visit_variable_expr(Variable &expr) override;
	VisResult visit_assign_expr(Assign &expr) override;

	// Stores the result of the last expression statement executed
	Primitive last_expr_result;

private:
	inline void execute(Stmt &stmt) { stmt.accept(*this); }

	inline Primitive evaluate(Expr &expr)
	{
		return std::get<Primitive>(expr.accept(*this));
	}

	void execute_block(
		std::vector<StmtPtr> &statements, EnvironmentPtr block_environ
	);

	EnvironmentPtr environment = std::make_shared<Environment>();
};

#endif
