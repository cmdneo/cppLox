#ifndef LOX_FUNCTION_HXX_INCLUDED
#define LOX_FUNCTION_HXX_INCLUDED

#include <string>
#include <vector>
#include <utility>
#include <memory>

#include "object.hxx"
#include "stmt.hxx"
#include "lox_callable.hxx"
#include "environment.hxx"

class Interpreter;
class LoxFunction;

using LoxFunctionPtr = std::shared_ptr<LoxFunction>;

class LoxFunction : public LoxCallable
{
public:
	LoxFunction(
		const Function &declaration_, EnvironmentPtr closure_,
		bool is_init = false
	)
		: declaration(declaration_)
		, closure(std::move(closure_))
		, is_initializer(is_init)
	{
	}

	unsigned arity() const override { return declaration.params.size(); }

	std::string to_string() const override
	{
		return "<fn " + declaration.name.lexeme + ">";
	}

	Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) override;

	LoxFunctionPtr bind(LoxInstancePtr instance);

	EnvironmentPtr closure;

private:
	Function declaration;
	bool is_initializer = false;
};

#endif