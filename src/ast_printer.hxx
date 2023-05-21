#ifndef AST_PRINTER_HXX_INCLUDED
#define AST_PRINTER_HXX_INCLUDED

#include <initializer_list>
#include <string>
#include <variant>

#include "object.hxx"
#include "expr.hxx"

struct AstPrinter : public ExprVisitor {
	inline std::string print(Expr &expr)
	{
		return std::get<std::string>(expr.accept(*this));
	}

	// Just use the std::string of the Object variant to store the result

	Object visit_ternary_expr(const Ternary &expr) override
	{
		return parenthesize({
			"?:",
			print(*expr.condition),
			print(*expr.expr1),
			print(*expr.expr2),
		});
	}

	Object visit_binary_expr(const Binary &expr) override
	{
		return parenthesize({
			std::string(expr.operat.lexeme),
			print(*expr.left),
			print(*expr.right),
		});
	}

	Object visit_call_expr(const Call &expr) override
	{
		std::string args;
		for (auto &arg : expr.arguments)
			args += print(*arg) + " ";
		// Remove trailing space
		args = args.substr(0, args.size() - 1);

		return parenthesize({
			"()",
			print(*expr.callee) + ":",
			args,
		});
	}

	Object visit_grouping_expr(const Grouping &expr) override
	{
		return parenthesize({
			"group",
			print(*expr.expression),
		});
	}

	Object visit_literal_expr(const Literal &expr) override
	{
		return to_string(expr.value);
	}

	Object visit_unary_expr(const Unary &expr) override
	{
		return parenthesize({
			std::string(expr.operat.lexeme),
			print(*expr.right),
		});
	}

	Object visit_variable_expr(const Variable &expr) override
	{
		return "var " + std::string(expr.name.lexeme);
	}

	Object visit_assign_expr(const Assign &expr) override
	{
		return parenthesize({
			"=",
			std::string(expr.name.lexeme),
			print(*expr.expression),
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