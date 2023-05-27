#ifndef NATIVE_HXX_INCLUDED
#define NATIVE_HXX_INCLUDED

#include <string>
#include <vector>

#include "object.hxx"
#include "lox_callable.hxx"

class Interpreter;

#define GENERATE_NATIVE_FUNCTION(class_name, arity_expr, name_str)  \
	struct class_name : public LoxCallable {                        \
		unsigned arity() const override { return arity_expr; }      \
		std::string to_string() const override { return name_str; } \
		Object call(Interpreter &, std::vector<Object> &) override; \
	}

// Native(built-in) functions
GENERATE_NATIVE_FUNCTION(ClockFn, 0, "<native-fn clock>");
GENERATE_NATIVE_FUNCTION(SleepFn, 1, "<native-fn sleep>");
GENERATE_NATIVE_FUNCTION(StringFn, 1, "<native-fn string>");
GENERATE_NATIVE_FUNCTION(InstanceOfFn, 2, "<native-fn instance_of>");

#undef GENERATE_NATIVE_FUNCTION



#endif