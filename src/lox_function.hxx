#ifndef LOX_FUNCTION_HXX_INCLUDED
#define LOX_FUNCTION_HXX_INCLUDED

#include <string>
#include <vector>
#include <utility>

#include "object.hxx"
#include "stmt.hxx"
#include "environment.hxx"

class Interpreter;

struct LoxFunction : public Callable {
	LoxFunction(const Function &declaration_, EnvironmentPtr closure_)
		: declaration(declaration_)
		, closure(std::move(closure_))
	{
	}

	unsigned arity() override { return declaration.params.size(); }

	std::string to_string() override
	{
		return "<fn " + declaration.name.lexeme + ">";
	}

	Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) override;

	Function declaration;
	EnvironmentPtr closure;
};

#endif