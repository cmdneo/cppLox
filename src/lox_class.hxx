#ifndef LOX_CLASS_HXX_INCLUDED
#define LOX_CLASS_HXX_INCLUDED

#include <map>
#include <string>
#include <vector>

#include "object.hxx"
#include "stmt.hxx"
#include "lox_callable.hxx"
#include "lox_function.hxx"

#include <iostream>

class Interpreter;

using ClassMethodMap = std::map<const std::string, LoxFunctionPtr>;

// The Lox class
// Assign the shared_ptr created to the self_ptr field of this class.
class LoxClass : public LoxCallable
{
public:
	LoxClass(const std::string &name_, ClassMethodMap methods_)
		: name(name_)
		, methods(std::move(methods_))
	{
	}

	LoxFunctionPtr find_method(const std::string &method_name)
	{
		auto result = methods.find(method_name);
		if (result != methods.end())
			return result->second;
		return nullptr;
	}

	std::string to_string() const override { return "<class " + name + ">"; }

	unsigned arity() const override { return 0; }

	Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) override;

	// Instance needs to keep a reference to the Class pointer, therefore,
	// store a weak_ptr to itself which will be used by the instance
	std::weak_ptr<LoxClass> self_ptr;
	std::string name;

private:
	ClassMethodMap methods;
};

#endif