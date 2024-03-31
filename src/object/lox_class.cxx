#include <vector>
#include <memory>

#include "object.hxx"
#include "lox_class.hxx"
#include "lox_instance.hxx"
#include "interpreter.hxx"

Object LoxClass::call(Interpreter &interpreter, std::vector<Object> &arguments)
{
	auto instance = std::make_shared<LoxInstance>(self_ptr.lock());
	instance->self_ptr = instance;

	auto initializer = find_method("init");
	if (initializer != nullptr)
		initializer->bind(instance)->call(interpreter, arguments);

	return instance;
}