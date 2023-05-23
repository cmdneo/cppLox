#include <cassert>
#include <memory>
#include <utility>

#include "object.hxx"
#include "lox_function.hxx"
#include "environment.hxx"
#include "interpreter.hxx"

Object
LoxFunction::call(Interpreter &interpreter, std::vector<Object> &arguments)
{
	assert(declaration.params.size() == arguments.size());

	auto environment = std::make_shared<Environment>(closure);
	for (unsigned i = 0; i < declaration.params.size(); ++i)
		environment->define(declaration.params[i].lexeme, arguments[i]);

	try {
		interpreter.execute_block(*declaration.body, std::move(environment));
	} catch (Interpreter::ControlReturn &return_value) {
		return return_value.value;
	}

	return nullptr;
}
