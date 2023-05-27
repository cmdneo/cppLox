#ifndef CALLABLE_HXX_INCLUDED
#define CALLABLE_HXX_INCLUDED

#include <string>
#include <vector>

#include "object.hxx"

class Interpreter;

// LoxCallable object interface
class LoxCallable
{
public:
	virtual unsigned arity() const = 0;
	virtual std::string to_string() const = 0;
	virtual Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) = 0;
	virtual ~LoxCallable() = default;
};

#endif