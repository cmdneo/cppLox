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
		if (is_initializer)
			return closure->get_at(0, "this");
		return return_value.value;
	}

	if (is_initializer)
		return closure->get_at(0, "this");
	return nullptr;
}

LoxFunctionPtr LoxFunction::bind(LoxInstancePtr instance)
{
	// Create a new environment whithin the method closure
	auto environment = std::make_shared<Environment>(closure);
	// and bind 'this' to the instance passed
	environment->define("this", std::move(instance));

	return std::make_unique<LoxFunction>(
		declaration, std::move(environment), is_initializer
	);
}
