#ifndef RUNTIME_ERROR_INCLUDED_HXX
#define RUNTIME_ERROR_INCLUDED_HXX

#include <stdexcept>
#include <string>

#include "token.hxx"

// Exception class for runtime error in lox script
struct RuntimeError : public std::runtime_error {
	RuntimeError(const Token &token_, const std::string &message)
		: runtime_error(message)
		, token(token_)
	{
	}

	const Token token;
};

// Exception class for runtime error in native(built-in) functions
// due to their wrong usage from the lox script
struct NativeFnError : public std::runtime_error {
	NativeFnError(const std::string &message)
		: runtime_error(message)
	{
	}
};

#endif
