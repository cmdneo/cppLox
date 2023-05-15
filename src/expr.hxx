#ifndef EXPR_HXX_INCLUDED
#define EXPR_HXX_INCLUDED

#include <memory>
#include <utility>
#include <variant>

#include "token.hxx"

struct Expr;
struct Assign;
struct Ternary;
struct Binary;
struct Grouping;
struct Literal;
struct Unary;
struct Variable;

// C++ does not support virtual member templates so use this variant HACK
// to represent all the types returned by visitors
using VisResult = std::variant<std::string, Primitive>;
using ExprPtr = std::unique_ptr<Expr>;

struct ExprVisitor {
	virtual VisResult visit_assign_expr(Assign &expr) = 0;
	virtual VisResult visit_ternary_expr(Ternary &expr) = 0;
	virtual VisResult visit_binary_expr(Binary &expr) = 0;
	virtual VisResult visit_grouping_expr(Grouping &expr) = 0;
	virtual VisResult visit_literal_expr(Literal &expr) = 0;
	virtual VisResult visit_unary_expr(Unary &expr) = 0;
	virtual VisResult visit_variable_expr(Variable &expr) = 0;
};

struct Expr {
	virtual VisResult accept(ExprVisitor &visitor) = 0;
	virtual ~Expr() = default;
};

struct Assign : public Expr {
	Assign(Token name_, ExprPtr expression_)
		: name(name_)
		, expression(std::move(expression_))
	{
	}

	VisResult accept(ExprVisitor &visitor) override
	{
		return visitor.visit_assign_expr(*this);
	}

	Token name;
	ExprPtr expression;
};

struct Ternary : public Expr {
	Ternary(ExprPtr condition_, ExprPtr expr1_, ExprPtr expr2_)
		: condition(std::move(condition_))
		, expr1(std::move(expr1_))
		, expr2(std::move(expr2_))
	{
	}

	VisResult accept(ExprVisitor &visitor) override
	{
		return visitor.visit_ternary_expr(*this);
	}

	const ExprPtr condition;
	const ExprPtr expr1;
	const ExprPtr expr2;
};

struct Binary : public Expr {
	Binary(ExprPtr left_, Token operat_, ExprPtr right_)
		: left(std::move(left_))
		, operat(operat_)
		, right(std::move(right_))
	{
	}

	VisResult accept(ExprVisitor &visitor) override
	{
		return visitor.visit_binary_expr(*this);
	}

	const ExprPtr left;
	const Token operat;
	const ExprPtr right;
};

struct Grouping : public Expr {
	Grouping(ExprPtr expression_)
		: expression(std::move(expression_))
	{
	}

	VisResult accept(ExprVisitor &visitor) override
	{
		return visitor.visit_grouping_expr(*this);
	}

	const ExprPtr expression;
};

struct Literal : public Expr {
	Literal(Primitive value_)
		: value(value_)
	{
	}

	VisResult accept(ExprVisitor &visitor) override
	{
		return visitor.visit_literal_expr(*this);
	}

	const Primitive value;
};

struct Unary : public Expr {
	Unary(Token operat_, ExprPtr right_)
		: operat(operat_)
		, right(std::move(right_))
	{
	}

	VisResult accept(ExprVisitor &visitor) override
	{
		return visitor.visit_unary_expr(*this);
	}

	const Token operat;
	const ExprPtr right;
};

struct Variable : public Expr {
	Variable(Token name_)
		: name(name_)
	{
	}

	VisResult accept(ExprVisitor &visitor) override
	{
		return visitor.visit_variable_expr(*this);
	}

	Token name;
};

#endif
