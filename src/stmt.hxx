#ifndef STMT_HXX_INCLUDED
#define STMT_HXX_INCLUDED

#include <memory>
#include <utility>
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

using StmtPtr = std::unique_ptr<Stmt>;

struct StmtVisitor {
	virtual void visit_assert_stmt(const Assert &stmt) = 0;
	virtual void visit_print_stmt(const Print &stmt) = 0;
	virtual void visit_break_stmt(const Break &stmt) = 0;
	virtual void visit_continue_stmt(const Continue &stmt) = 0;
	virtual void visit_return_stmt(const Return &stmt) = 0;
	virtual void visit_expr_stmt(const Expression &stmt) = 0;
	virtual void visit_block_stmt(const Block &stmt) = 0;
	virtual void visit_if_stmt(const If &stmt) = 0;
	virtual void visit_while_stmt(const While &stmt) = 0;
	virtual void visit_var_stmt(const Var &stmt) = 0;
	virtual void visit_function_stmt(const Function &stmt) = 0;
	virtual ~StmtVisitor() = default;
};

struct Stmt {
	virtual void accept(StmtVisitor &visitor) = 0;
	virtual ~Stmt() = default;
};

struct Block : public Stmt {
	Block(std::vector<StmtPtr> statements_)
		: statements(std::move(statements_))
	{
	}

	void accept(StmtVisitor &visitor) override
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

	void accept(StmtVisitor &visitor) override
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

	void accept(StmtVisitor &visitor) override
	{
		visitor.visit_print_stmt(*this);
	}

	ExprPtr expression;
};

struct Assert : public Stmt {
	Assert(Token token_, ExprPtr expr)
		: token(token_)
		, expression(std::move(expr))
	{
	}

	void accept(StmtVisitor &visitor) override
	{
		visitor.visit_assert_stmt(*this);
	}

	// Token for line information
	Token token;
	ExprPtr expression;
};

struct Break : public Stmt {
	Break() = default;

	void accept(StmtVisitor &visitor) override
	{
		visitor.visit_break_stmt(*this);
	}
};

struct Continue : public Stmt {
	Continue() = default;

	void accept(StmtVisitor &visitor) override
	{
		visitor.visit_continue_stmt(*this);
	}
};

struct Return : public Stmt {
	Return(Token keyword_, ExprPtr value_)
		: keyword(keyword_)
		, value(std::move(value_))
	{
	}

	void accept(StmtVisitor &visitor) override
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

	void accept(StmtVisitor &visitor) override { visitor.visit_if_stmt(*this); }

	ExprPtr condition;
	StmtPtr then_branch;
	StmtPtr else_branch;
};

struct While : public Stmt {
	While(ExprPtr condition_, StmtPtr body_)
		: condition(std::move(condition_))
		, body(std::move(body_))
	{
	}

	void accept(StmtVisitor &visitor) override
	{
		visitor.visit_while_stmt(*this);
	}

	ExprPtr condition;
	StmtPtr body;
};

struct Var : public Stmt {
	Var(Token name_, ExprPtr initializer_)
		: name(name_)
		, initializer(std::move(initializer_))
	{
	}

	void accept(StmtVisitor &visitor) override
	{
		visitor.visit_var_stmt(*this);
	}

	Token name;
	ExprPtr initializer;
};

struct Function : public Stmt {
	Function(
		Token name_, std::vector<Token> params_,
		std::shared_ptr<std::vector<StmtPtr>> body_
	)
		: name(name_)
		, params(std::move(params_))
		, body(std::move(body_))
	{
	}

	void accept(StmtVisitor &visitor) { visitor.visit_function_stmt(*this); }

	Token name;
	std::vector<Token> params;
	// LoxFunction also needs the function body AST,
	// so instead of copying the AST which will be very complicated.
	// Use shared_ptr to share the reference to the AST which was constructed.
	std::shared_ptr<std::vector<StmtPtr>> body;
};

#endif