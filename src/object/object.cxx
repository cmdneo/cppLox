#include <cstddef>
#include <string>
#include <variant>

#include "object.hxx"
#include "lox_callable.hxx"
#include "lox_class.hxx"
#include "lox_instance.hxx"

using std::get;
using std::string;

static string double_to_string_trimmed(double val)
{
	auto str = std::to_string(val);
	auto is_zeros = [](const string &s) -> bool {
		for (auto c : s) {
			if (c != '0')
				return false;
		}
		return true;
	};

	// Remove the fractional part if it is zero
	if (auto at = str.find('.');
		at != string::npos && is_zeros(str.substr(at + 1))) {
		return str.substr(0, at);
	}

	return str;
}

// std::nullptr_t, std::string, LoxCallablePtr, LoxClassPtr, LoxInstancePtr, double, bool
string to_string(const Object &obj)
{
	if (auto ptr = get_if<std::nullptr_t>(&obj))
		return "nil";
	if (auto ptr = get_if<bool>(&obj))
		return *ptr ? "true" : "false";
	if (auto ptr = get_if<double>(&obj))
		return double_to_string_trimmed(*ptr);
	if (auto ptr = get_if<string>(&obj))
		return *ptr;
	if (auto ptr = get_if<LoxCallablePtr>(&obj))
		return (*ptr)->to_string();
	if (auto ptr = get_if<LoxClassPtr>(&obj))
		return (*ptr)->to_string();
	if (auto ptr = get_if<LoxInstancePtr>(&obj))
		return (*ptr)->to_string();

	assert(!"Unreachable code");
	return "";
}
