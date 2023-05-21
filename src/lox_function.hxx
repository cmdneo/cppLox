#ifndef LOX_FUNCTION_HXX_INCLUDED
#define LOX_FUNCTION_HXX_INCLUDED

#include <string>
#include <vector>

#include "object.hxx"
#include "stmt.hxx"

class Interpreter;

struct LoxFunction : public Callable {
	LoxFunction(const Function &stmt)
		: declaration(stmt)
	{
	}

	unsigned arity() override { return declaration.params.size(); }

	std::string to_string() override
	{
		return "<fn " + std::string(declaration.name.lexeme) + ">";
	}

	Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) override;

	Function declaration;
};

#endif