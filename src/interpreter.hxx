#ifndef INTERPRETER_HXX_INCLUDED
#define INTERPRETER_HXX_INCLUDED

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "error.hxx"
#include "runtime_error.hxx"
#include "expr.hxx"
#include "stmt.hxx"
#include "environment.hxx"
#include "garbage.hxx"
#include "object/object.hxx"

class Interpreter : private ExprVisitor, private StmtVisitor
{
	friend class LoxFunction; // execute_block and ControlReturn.

public:
	Interpreter();
	void interpret(std::vector<StmtPtr> statements);

	/// Puts info in name resolution side table for locals. For Resolver.
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

	inline void execute(const Stmt &stmt) { stmt.accept(*this); }

	inline Object evaluate(const Expr &expr) { return expr.accept(*this); }

	/// Execute a statement block with the provided environment.
	/// @param statements List of statements
	/// @param block_environ The environment for it
	void execute_block(
		const std::vector<StmtPtr> &statements, EnvironmentPtr block_environ
	);

	EnvironmentPtr globals = std::make_shared<Environment>();
	EnvironmentPtr environment = globals;

	// Name resolution Side Table.
	// Sotres static name resolution information for local variables to
	// prevent dynamic scope leak in case of closures.
	// For example:
	// var k2 = 42;
	// {
	//      var k1 = 10;
	//      fun clos() {
	//           print k1 + k2;
	//      }
	//      var k2 = 20;
	// }
	// Here clos should resolve k2 as the one having value 42,
	// not the one having value 20.
	//
	// The information stores is pointer representing the variable and its
	// scope distance from current use point to its closest defnition.
	std::map<const Expr *, int> locals;
	GarbageCollector garbage_collector{globals};
};

#endif
