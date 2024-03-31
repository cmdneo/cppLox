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
	LoxClass(
		const std::string &name_, LoxClassPtr superclass_,
		ClassMethodMap methods_
	)
		: name(name_)
		, superclass(std::move(superclass_))
		, methods(std::move(methods_))
	{
	}

	LoxFunctionPtr find_method(const std::string &method_name) const
	{
		auto result = methods.find(method_name);
		if (result != methods.end())
			return result->second;

		if (superclass != nullptr)
			return superclass->find_method(method_name);

		return nullptr;
	}

	std::string to_string() const override { return "<class " + name + ">"; }

	unsigned arity() const override
	{
		auto initializer = find_method("init");
		if (initializer == nullptr)
			return 0;
		return initializer->arity();
	}

	Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) override;

	// Instance needs to keep a reference to the Class, therefore,
	// store a weak_ptr to itself which will be used by the instance
	// It is set after creating the class ptr.
	std::weak_ptr<LoxClass> self_ptr;
	std::string name;

private:
	LoxClassPtr superclass;
	ClassMethodMap methods;
};

#endif
