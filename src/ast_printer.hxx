#ifndef AST_PRINTER_HXX_INCLUDED
#define AST_PRINTER_HXX_INCLUDED

#include <initializer_list>
#include <string>
#include <variant>

#include "expr.hxx"

struct AstPrinter : public Visitor {
	inline std::string print(Expr &expr)
	{
		return std::get<std::string>(expr.accept(*this));
	}

	VisResult visit_ternary_expr(Ternary &expr) override
	{
		return parenthesize({
			"?:",
			print(*expr.condition),
			print(*expr.expr1),
			print(*expr.expr2),
		});
	}
 
	VisResult visit_binary_expr(Binary &expr) override
	{
		return parenthesize({
			std::string(expr.operat.lexeme),
			print(*expr.left),
			print(*expr.right),
		});
	}

	VisResult visit_grouping_expr(Grouping &expr) override
	{
		return parenthesize({
			"group",
			print(*expr.expression),
		});
	}

	VisResult visit_literal_expr(Literal &expr) override
	{
		return to_string(expr.value);
	}

	VisResult visit_unary_expr(Unary &expr) override
	{
		return parenthesize({
			std::string(expr.operat.lexeme),
			print(*expr.right),
		});
	}

private:
	static std::string parenthesize(std::initializer_list<std::string> li)
	{
		std::string ret;

		ret += "(";
		for (const auto &item : li) {
			ret += item;
			ret += " ";
		}
		ret.back() = ')';

		return ret;
	}
};

#endif