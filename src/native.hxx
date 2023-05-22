#ifndef NATIVE_HXX_INCLUDED
#define NATIVE_HXX_INCLUDED

#include <string>
#include <vector>

#include "runtime_error.hxx"
#include "object.hxx"

// Native(built-in) functions

struct ClockFn : public Callable {
	unsigned arity() override { return 0; }
	std::string to_string() override { return "<native-fn clock>"; }

	Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) override;
};

struct SleepFn : public Callable {
	unsigned arity() override { return 1; }
	std::string to_string() override { return "<native-fn sleep>"; }

	Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) override;
};

struct StringFn : public Callable {
	unsigned arity() override { return 1; }
	std::string to_string() override { return "<native-fn string>"; }

	Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) override;
};

#endif