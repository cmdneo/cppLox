#ifndef RUNTIME_ERROR_INCLUDED_HXX
#define RUNTIME_ERROR_INCLUDED_HXX

#include <stdexcept>
#include <string>

#include "token.hxx"

struct RuntimeError : public std::runtime_error {
	RuntimeError(const Token &token_, const std::string &message)
		: runtime_error(message)
		, token(token_)
	{
	}

	const Token token;
};

#endif
