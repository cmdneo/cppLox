#ifndef NATIVE_HXX_INCLUDED
#define NATIVE_HXX_INCLUDED

#include <string>
#include <vector>

#include "runtime_error.hxx"
#include "object.hxx"

// Native(built-in) functions

struct ClockFn : public Callable {
	ClockFn() = default;

	unsigned arity() override { return 0; }

	std::string to_string() override { return "<native-fn>"; }

	Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) override;
};

struct SleepFn : public Callable {
	SleepFn() = default;

	unsigned arity() override { return 1; }

	std::string to_string() override { return "<native-fn>"; }

	Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) override;
};

#endif