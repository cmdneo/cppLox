#include <chrono>
#include <thread>
#include <variant>

#include "object.hxx"
#include "native.hxx"

using namespace std::chrono;

Object ClockFn::call(Interpreter &, std::vector<Object> &)
{
	duration<double> time = system_clock::now().time_since_epoch();
	return time.count();
}

Object SleepFn::call(Interpreter &, std::vector<Object> &arguments)
{
	auto &time = arguments[0];
	if (!match_types<double>(time) || std::get<double>(time) < 0)
		throw NativeFnError(
			"Argument to 'sleep' should be a non-negative number."
		);

	unsigned time_ms = 1000.0 * std::get<double>(time);
	std::this_thread::sleep_for(milliseconds(time_ms));
	return nullptr;
}

Object StringFn::call(Interpreter &, std::vector<Object> &arguments)
{
	return ::to_string(arguments[0]);
}
