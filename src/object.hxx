#ifndef LOX_OBJECT_HXX_INCLUDED
#define LOX_OBJECT_HXX_INCLUDED

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>
#include <variant>
#include <memory>

// Forward declarations
struct Callable;
class Interpreter;

using CallablePtr = std::shared_ptr<Callable>;

// The Lox object type
// Represents all the in-built types supported by Lox
// Uses std::nullptr_t to represent Nil
using Object =
	std::variant<std::nullptr_t, std::string, CallablePtr, double, bool>;

// Callable object interface
struct Callable {
	virtual unsigned arity() = 0;
	virtual std::string to_string() = 0;
	virtual Object
	call(Interpreter &interpreter, std::vector<Object> &arguments) = 0;
	virtual ~Callable() = default;
};

static std::string double_to_string_trimmed(double val)
{
	auto str = std::to_string(val);
	auto is_zeros = [](const std::string &s) -> bool {
		for (auto c : s) {
			if (c != '0')
				return false;
		}
		return true;
	};

	if (auto at = str.find('.');
		at != std::string::npos && is_zeros(str.substr(at + 1))) {
		return str.substr(0, at);
	}

	return str;
}

[[maybe_unused]] static std::string to_string(const Object &lit)
{
	switch (lit.index()) {
	case 0:
		return "nil";
	case 1:
		return std::get<1>(lit);
	case 2:
		return std::get<2>(lit)->to_string();
	case 3:
		return double_to_string_trimmed(std::get<3>(lit));
	case 4:
		return std::get<4>(lit) ? "true" : "false";
	}

	assert(!"Unreachable code");
}

// Returns true if all primitives hold the value of the corresponding given type.
// Precisely: If for every object
// nth object holds the value of the type represented
// by the nth item of MatchTypes, then return true.
template <typename... MatchTypes, int pack_barrier = 0, typename... Ps>
inline bool match_types(const Ps &...objects)
{
	bool all_match = true;
	(
		[&] {
			if (!std::holds_alternative<MatchTypes>(objects))
				all_match = false;
		}(),
		...
	);
	return all_match;
}

#endif