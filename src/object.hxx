#ifndef LOX_OBJECT_HXX_INCLUDED
#define LOX_OBJECT_HXX_INCLUDED

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>
#include <variant>
#include <memory>

// Forward declarations
class LoxCallable;
class LoxClass;
class LoxInstance;
class Interpreter;

using LoxCallablePtr = std::shared_ptr<LoxCallable>;
using LoxClassPtr = std::shared_ptr<LoxClass>;
using LoxInstancePtr = std::shared_ptr<LoxInstance>;

// The Lox object type
// Represents all the in-built types supported by Lox
// Uses std::nullptr_t to represent Nil
// Even though LoxClass is a subclass of LoxCallable we store it as a seperate
// type and not behind a polymorphic pointer to LoxCallable.
using Object = std::variant<
	std::nullptr_t, bool, double, std::string, LoxCallablePtr, LoxClassPtr,
	LoxInstancePtr>;

std::string to_string(const Object &obj);

// Returns true if all primitives hold the value of the corresponding given type.
// Precisely: If for every object
// nth object holds the value of the type represented
// by the nth item of MatchTypes, then return true.
template <typename... MatchTypes, int pack_barrier = 0, typename... Objects>
inline bool match_types(const Objects &...objects)
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