#ifndef INTERPRETER_HXX_INCLUDED
#define INTERPRETER_HXX_INCLUDED

#include <variant>
#include <iostream>
#include <string>

#include "error.hxx"
#include "runtime_error.hxx"
#include "expr.hxx"
#include "token.hxx"

class Interpreter : public Visitor
{
public:
	void interpret(Expr &expr)
	{
		try {
			auto value = to_string(evaluate(expr));

			// Strip off the decimal part if it is zero
			auto decimal_at = value.find('.');
			if (decimal_at != std::string::npos
				&& std::stod(value.substr(decimal_at + 1)) == 0.0)
				value = value.substr(0, decimal_at);

			std::cout << value << "\n";
		} catch (RuntimeError &err) {
			print_runtime_error(err);
		}
	}
	VisResult visit_literal_expr(Literal &expr) override;
	VisResult visit_grouping_expr(Grouping &expr) override;
	VisResult visit_unary_expr(Unary &expr) override;
	VisResult visit_binary_expr(Binary &expr) override;
	VisResult visit_ternary_expr(Ternary &expr) override;

private:
	inline Primitive evaluate(Expr &expr)
	{
		return std::get<Primitive>(expr.accept(*this));
	}
};

#endif
