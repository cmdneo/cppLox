#include <cassert>
#include <memory>

#include "lox_function.hxx"
#include "environment.hxx"
#include "interpreter.hxx"

Object
LoxFunction::call(Interpreter &interpreter, std::vector<Object> &arguments)
{
	assert(declaration.params.size() == arguments.size());

	auto environment = std::make_shared<Environment>(interpreter.globals);
	for (unsigned i = 0; i < declaration.params.size(); ++i)
		environment->define(declaration.params[i].lexeme, arguments[i]);

	interpreter.execute_block(*declaration.body, environment);

	return nullptr;
}
