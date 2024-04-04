#ifndef STMT_HXX_INCLUDED
#define STMT_HXX_INCLUDED

#include <memory>
#include <utility>
#include <optional>
#include <vector>

#include "expr.hxx"

struct Stmt;
struct Block;
struct Expression;
struct Print;
struct Assert;
struct Break;
struct Continue;
struct Return;
struct If;
struct While;
struct Var;
struct Function;
struct Class;

using StmtPtr = std::unique_ptr<Stmt>;

struct StmtVisitor {
	virtual void visit_block_stmt(const Block &stmt) = 0;
	virtual void visit_expr_stmt(const Expression &stmt) = 0;
	virtual void visit_print_stmt(const Print &stmt) = 0;
	virtual void visit_assert_stmt(const Assert &stmt) = 0;
	virtual void visit_break_stmt(const Break &stmt) = 0;
	virtual void visit_continue_stmt(const Continue &stmt) = 0;
	virtual void visit_return_stmt(const Return &stmt) = 0;
	virtual void visit_if_stmt(const If &stmt) = 0;
	virtual void visit_while_stmt(const While &stmt) = 0;
	virtual void visit_var_stmt(const Var &stmt) = 0;
	virtual void visit_function_stmt(const Function &stmt) = 0;
	virtual void visit_class_stmt(const Class &stmt) = 0;
	virtual ~StmtVisitor() = default;
};

struct Stmt {
	virtual void accept(StmtVisitor &visitor) const = 0;
	virtual ~Stmt() = default;
};

struct Block : public Stmt {
	Block(std::vector<StmtPtr> statements_)
		: statements(std::move(statements_))
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_block_stmt(*this);
	}

	std::vector<StmtPtr> statements;
};

template <typename... Stmts>
std::unique_ptr<Block> make_block(Stmts... stmts)
{
	std::vector<StmtPtr> body;
	([&] { body.push_back(std::move(stmts)); }(), ...);

	return make_unique<Block>(std::move(body));
}

struct Expression : public Stmt {
	Expression(ExprPtr expr)
		: expression(std::move(expr))
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_expr_stmt(*this);
	}

	ExprPtr expression;
};

struct Print : public Stmt {
	Print(ExprPtr expr)
		: expression(std::move(expr))
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_print_stmt(*this);
	}

	ExprPtr expression;
};

struct Assert : public Stmt {
	Assert(const Token &token_, ExprPtr expr)
		: token(token_)
		, expression(std::move(expr))
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_assert_stmt(*this);
	}

	// Token for line information
	Token token;
	ExprPtr expression;
};

struct Break : public Stmt {
	Break(const Token &keyword_)
		: keyword(keyword_)
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_break_stmt(*this);
	}

	Token keyword;
};

struct Continue : public Stmt {
	Continue(const Token &keyword_)
		: keyword(keyword_)
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_continue_stmt(*this);
	}

	Token keyword;
};

struct Return : public Stmt {
	Return(const Token &keyword_, ExprPtr value_)
		: keyword(keyword_)
		, value(std::move(value_))
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_return_stmt(*this);
	}

	Token keyword;
	ExprPtr value;
};

struct If : public Stmt {
	If(ExprPtr condition_, StmtPtr then_branch_, StmtPtr else_branch_)
		: condition(std::move(condition_))
		, then_branch(std::move(then_branch_))
		, else_branch(std::move(else_branch_))
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_if_stmt(*this);
	}

	ExprPtr condition;
	StmtPtr then_branch;
	StmtPtr else_branch;
};

struct While : public Stmt {
	While(ExprPtr condition_, StmtPtr body_, Expression *for_update_ = nullptr)
		: condition(std::move(condition_))
		, body(std::move(body_))
		, for_update(for_update_)
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_while_stmt(*this);
	}

	ExprPtr condition;
	StmtPtr body;
	// Update clause for the 'for' loop.
	// It is also always present somewhere in the above body field.
	Expression *for_update;
};

struct Var : public Stmt {
	Var(const Token &name_, ExprPtr initializer_)
		: name(name_)
		, initializer(std::move(initializer_))
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_var_stmt(*this);
	}

	Token name;
	ExprPtr initializer;
};

struct Function : public Stmt {
	Function(
		const Token &name_, std::vector<Token> params_,
		std::shared_ptr<std::vector<StmtPtr>> body_
	)
		: name(name_)
		, params(std::move(params_))
		, body(std::move(body_))
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_function_stmt(*this);
	}

	Token name;
	std::vector<Token> params;
	std::shared_ptr<std::vector<StmtPtr>> body;
};

struct Class : public Stmt {
	Class(
		const Token &name_, std::optional<Variable> superclass_,
		std::vector<Function> methods_
	)
		: name(name_)
		, superclass(superclass_)
		, methods(std::move(methods_))
	{
	}

	void accept(StmtVisitor &visitor) const override
	{
		visitor.visit_class_stmt(*this);
	}

	Token name;
	std::optional<Variable> superclass;
	std::vector<Function> methods;
};

#endif