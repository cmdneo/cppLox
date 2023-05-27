#ifndef LOX_INSTANCE_HXX_INCLUDED
#define LOX_INSTANCE_HXX_INCLUDED

#include <map>
#include <string>
#include <utility>
#include <format>

#include "runtime_error.hxx"
#include "token.hxx"
#include "object.hxx"
#include "lox_class.hxx"

// The Lox class instance
// Assign the shared_ptr created to the self_ptr field of this class.
class LoxInstance
{
public:
	LoxInstance(LoxClassPtr klass_)
		: klass(std::move(klass_))
	{
	}

	std::string to_string() const
	{
		return "<instance of " + klass->name + ">";
	}

	Object get(const Token &name)
	{
		auto result = fields.find(name.lexeme);
		if (result != fields.end())
			return result->second;

		auto method = klass->find_method(name.lexeme);
		if (method != nullptr)
			return method->bind(self_ptr.lock());

		throw RuntimeError(
			name, std::format("Undefined property '{}'.", name.lexeme)
		);
	}

	void set(const Token &name, const Object &value)
	{
		fields[name.lexeme] = value;
	}

	bool instance_of(const LoxClassPtr &klass_type)
	{
		return klass_type == klass;
	}

	// Methods needs to keep a reference to the instance pointer, therefore,
	// store a weak_ptr to itself which will be used by the methods
	std::weak_ptr<LoxInstance> self_ptr;

private:
	LoxClassPtr klass;
	std::map<const std::string, Object> fields;
};

#endif