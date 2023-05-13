#ifndef ERROR_HXX_INCLUDED
#define ERROR_HXX_INCLUDED

#include <string>
#include <string_view>
#include <iostream>

#include "token.hxx"
#include "token_type.hxx"
#include "runtime_error.hxx"

// Use this global, works fine for our simple use case
extern bool lox_had_error;
extern bool lox_had_runtime_error;

[[maybe_unused]] static void
print_error(int line, std::string_view message, std::string_view where = "")
{
	lox_had_error = true;

	std::cout << "[line " << line << "] Error " << where << ": " << message
			  << '\n';
}

[[maybe_unused]] static void print_error(Token token, std::string_view message)
{
	lox_had_error = true;

	if (token.type == TokenType::END_OF_FILE) {
		print_error(token.line, message, "at end");
	} else {
		print_error(
			token.line, message,
			std::string("at '") + std::string(token.lexeme) + "'"
		);
	}
}

[[maybe_unused]] static void print_runtime_error(const RuntimeError &err)
{
	std::cout << err.what() << "\n[line " << err.token.line << "]\n";
	lox_had_runtime_error = true;
}

#endif
