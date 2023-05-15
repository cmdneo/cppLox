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
struct Var;

using StmtPtr = std::unique_ptr<Stmt>;

struct StmtVisitor {
	virtual void visit_assert_stmt(Assert &stmt) = 0;
	virtual void visit_print_stmt(Print &stmt) = 0;
	virtual void visit_expr_stmt(Expression &stmt) = 0;
	virtual void visit_block_stmt(Block &stmt) = 0;
	virtual void visit_var_stmt(Var &stmt) = 0;
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

#endif