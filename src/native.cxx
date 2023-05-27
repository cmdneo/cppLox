#include <chrono>
#include <thread>
#include <variant>

#include "runtime_error.hxx"
#include "object.hxx"
#include "lox_instance.hxx"
#include "lox_class.hxx"
#include "native.hxx"
#include "interpreter.hxx"

using namespace std::chrono;
using std::get;

Object ClockFn::call(Interpreter &, std::vector<Object> &)
{
	duration<double> time = system_clock::now().time_since_epoch();
	return time.count();
}

Object SleepFn::call(Interpreter &, std::vector<Object> &arguments)
{
	auto &time = arguments[0];
	if (!match_types<double>(time) || get<double>(time) < 0) {
		throw NativeFnError(
			"Argument to 'sleep' should be a non-negative number."
		);
	}

	unsigned time_ms = 1000.0 * std::get<double>(time);
	std::this_thread::sleep_for(milliseconds(time_ms));
	return nullptr;
}

Object StringFn::call(Interpreter &, std::vector<Object> &arguments)
{
	return ::to_string(arguments[0]);
}

Object InstanceOfFn::call(Interpreter &, std::vector<Object> &arguments)
{
	auto &instance = arguments[0];
	auto &klass = arguments[1];
	if (!match_types<LoxInstancePtr, LoxClassPtr>(instance, klass)) {
		throw NativeFnError(
			"Arguments to 'instance_of' must be an instance and a class."
		);
	}

	return get<LoxInstancePtr>(instance)->instance_of(get<LoxClassPtr>(klass));
}
