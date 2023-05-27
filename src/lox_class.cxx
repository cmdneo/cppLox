#include <vector>
#include <memory>

#include "object.hxx"
#include "lox_class.hxx"
#include "lox_instance.hxx"
#include "interpreter.hxx"

Object LoxClass::call(Interpreter &, std::vector<Object> &)
{
	auto instance = std::make_shared<LoxInstance>(self_ptr.lock());
	instance->self_ptr = instance;
	return instance;
}