#ifndef INTERPRETER_HXX_INCLUDED
#define INTERPRETER_HXX_INCLUDED

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "error.hxx"
#include "runtime_error.hxx"
#include "object.hxx"
#include "expr.hxx"
#include "stmt.hxx"
#include "environment.hxx"
#include "garbage.hxx"

class Interpreter : private ExprVisitor, private StmtVisitor
{
	friend class LoxFunction;

public:
	Interpreter();
	void interpret(std::vector<StmtPtr> statements);
	void resolve(const Expr &expr, int depth) { locals[&expr] = depth; }

	void visit_assert_stmt(const Assert &stmt) override;
	void visit_print_stmt(const Print &stmt) override;
	void visit_break_stmt(const Break &stmt) override;
	void visit_expr_stmt(const Expression &stmt) override;
	void visit_block_stmt(const Block &stmt) override;
	void visit_continue_stmt(const Continue &stmt) override;
	void visit_return_stmt(const Return &stmt) override;
	void visit_if_stmt(const If &stmt) override;
	void visit_while_stmt(const While &stmt) override;
	void visit_var_stmt(const Var &stmt) override;
	void visit_function_stmt(const Function &stmt) override;
	void visit_class_stmt(const Class &stmt) override;

	Object visit_literal_expr(const Literal &expr) override;
	Object visit_grouping_expr(const Grouping &expr) override;
	Object visit_call_expr(const Call &expr) override;
	Object visit_get_expr(const Get &expr) override;
	Object visit_set_expr(const Set &expr) override;
	Object visit_this_expr(const This &expr) override;
	Object visit_super_expr(const Super &expr) override;
	Object visit_unary_expr(const Unary &expr) override;
	Object visit_binary_expr(const Binary &expr) override;
	Object visit_logical_expr(const Logical &expr) override;
	Object visit_ternary_expr(const Ternary &expr) override;
	Object visit_variable_expr(const Variable &expr) override;
	Object visit_assign_expr(const Assign &expr) override;

	// Stores the result of the last expression statement executed
	Object last_expr_result;

private:
	// Control-flow exceptions.
	// We use the already available C++ exception mechanism to
	// simplify the implementation of non-linear control flow statements
	struct ControlBreak {
	};
	struct ControlContinue {
	};
	struct ControlReturn {
		Object value;
	};

	Object look_up_variable(const Token &name, const Expr &expr)
	{
		auto result = locals.find(&expr);
		if (result != locals.end()) {
			auto distance = result->second;
			return environment->get_at(distance, name.lexeme);
		} else {
			return globals->get(name);
		}
	}

	inline void execute(Stmt &stmt) { stmt.accept(*this); }

	inline Object evaluate(Expr &expr) { return expr.accept(*this); }

	void execute_block(
		const std::vector<StmtPtr> &statements, EnvironmentPtr &&block_environ
	);

	EnvironmentPtr globals = std::make_shared<Environment>();
	EnvironmentPtr environment = globals;
	std::map<const Expr *, int> locals;
	GarbageCollector garbage_collector{globals};
};

#endif
