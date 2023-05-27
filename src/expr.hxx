#ifndef EXPR_HXX_INCLUDED
#define EXPR_HXX_INCLUDED

#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "object.hxx"
#include "token.hxx"

struct Expr;
struct Assign;
struct Ternary;
struct Logical;
struct Binary;
struct Call;
struct Get;
struct Set;
struct This;
struct Grouping;
struct Literal;
struct Unary;
struct Variable;

using ExprPtr = std::unique_ptr<Expr>;

struct ExprVisitor {
	virtual Object visit_assign_expr(const Assign &expr) = 0;
	virtual Object visit_ternary_expr(const Ternary &expr) = 0;
	virtual Object visit_logical_expr(const Logical &expr) = 0;
	virtual Object visit_binary_expr(const Binary &expr) = 0;
	virtual Object visit_call_expr(const Call &expr) = 0;
	virtual Object visit_get_expr(const Get &expr) = 0;
	virtual Object visit_set_expr(const Set &expr) = 0;
	virtual Object visit_this_expr(const This &expr) = 0;
	virtual Object visit_grouping_expr(const Grouping &expr) = 0;
	virtual Object visit_literal_expr(const Literal &expr) = 0;
	virtual Object visit_unary_expr(const Unary &expr) = 0;
	virtual Object visit_variable_expr(const Variable &expr) = 0;
	virtual ~ExprVisitor() = default;
};

struct Expr {
	virtual Object accept(ExprVisitor &visitor) = 0;
	virtual ~Expr() = default;
};

struct Assign : public Expr {
	Assign(const Token &name_, ExprPtr expression_)
		: name(name_)
		, expression(std::move(expression_))
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_assign_expr(*this);
	}

	Token name;
	ExprPtr expression;
};

struct Ternary : public Expr {
	Ternary(ExprPtr condition_, ExprPtr true_expr_, ExprPtr false_expr_)
		: condition(std::move(condition_))
		, true_expr(std::move(true_expr_))
		, false_expr(std::move(false_expr_))
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_ternary_expr(*this);
	}

	const ExprPtr condition;
	const ExprPtr true_expr;
	const ExprPtr false_expr;
};

struct Logical : public Expr {
	Logical(ExprPtr left_, const Token &operat_, ExprPtr right_)
		: left(std::move(left_))
		, operat(operat_)
		, right(std::move(right_))
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_logical_expr(*this);
	}

	ExprPtr left;
	Token operat;
	ExprPtr right;
};

struct Binary : public Expr {
	Binary(ExprPtr left_, const Token &operat_, ExprPtr right_)
		: left(std::move(left_))
		, operat(operat_)
		, right(std::move(right_))
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_binary_expr(*this);
	}

	const ExprPtr left;
	const Token operat;
	const ExprPtr right;
};

struct Call : public Expr {
	Call(ExprPtr callee_, const Token &paren_, std::vector<ExprPtr> arguments_)
		: callee(std::move(callee_))
		, paren(paren_)
		, arguments(std::move(arguments_))
	{
	}

	Object accept(ExprVisitor &visitor)
	{
		return visitor.visit_call_expr(*this);
	}

	ExprPtr callee;
	Token paren;
	std::vector<ExprPtr> arguments;
};

struct Get : public Expr {
	Get(ExprPtr object_, const Token &name_)
		: object(std::move(object_))
		, name(name_)
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_get_expr(*this);
	}

	ExprPtr object;
	Token name;
};

struct Set : public Expr {
	Set(ExprPtr object_, const Token &name_, ExprPtr value_)
		: object(std::move(object_))
		, name(name_)
		, value(std::move(value_))
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_set_expr(*this);
	}

	ExprPtr object;
	Token name;
	ExprPtr value;
};

struct This : public Expr {
	This(const Token &keyword_)
		: keyword(keyword_)
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_this_expr(*this);
	}

	Token keyword;
};

struct Grouping : public Expr {
	Grouping(ExprPtr expression_)
		: expression(std::move(expression_))
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_grouping_expr(*this);
	}

	const ExprPtr expression;
};

struct Literal : public Expr {
	Literal(Object value_)
		: value(value_)
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_literal_expr(*this);
	}

	const Object value;
};

struct Unary : public Expr {
	Unary(const Token &operat_, ExprPtr right_)
		: operat(operat_)
		, right(std::move(right_))
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_unary_expr(*this);
	}

	const Token operat;
	const ExprPtr right;
};

struct Variable : public Expr {
	Variable(const Token &name_)
		: name(name_)
	{
	}

	Object accept(ExprVisitor &visitor) override
	{
		return visitor.visit_variable_expr(*this);
	}

	Token name;
};

#endif
