#include <cassert>
#include <string>
#include <variant>

// Nil marker class
class Nil
{
};

using Primitive = std::variant<Nil, std::string, double, bool>;

[[maybe_unused]] static std::string to_string(const Primitive &lit)
{
	switch (lit.index()) {
	case 0:
		return "nil";
	case 1:
		return "string(" + std::string(std::get<1>(lit)) + ")";
	case 2:
		return std::to_string(std::get<2>(lit));
	case 3:
		return std::get<3>(lit) ? "true" : "false";
	}

	assert(!"Unreachable code");
}

Primitive operator-(const Primitive &a, const Primitive &b) {}
Primitive operator+(const Primitive &a, const Primitive &b) {}
Primitive operator*(const Primitive &a, const Primitive &b) {}
Primitive operator/(const Primitive &a, const Primitive &b) {}
