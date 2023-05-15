#ifndef ENVIRONMENT_HXX_INCLUDED
#define ENVIRONMENT_HXX_INCLUDED

#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "runtime_error.hxx"
#include "token.hxx"

class Environment;
using EnvironmentPtr = std::shared_ptr<Environment>;

class Environment
{
public:
	Environment(EnvironmentPtr encolsing_env = nullptr)
		: encolsing(encolsing_env)
	{
	}

	void define(std::string_view name, const Primitive &value)
	{
		values[std::string(name)] = value;
	}

	void assign(const Token &name, const Primitive &value)
	{
		auto result = values.find(std::string(name.lexeme));
		if (result != values.end()) {
			result->second = value;
			return;
		}

		if (encolsing != nullptr) {
			encolsing->assign(name, value);
			return;
		}

		throw RuntimeError(
			name, "Undefined variable '" + std::string(name.lexeme) + "'."
		);
	}

	Primitive &get(const Token &name)
	{
		auto result = values.find(std::string(name.lexeme));
		if (result != values.end())
			return result->second;

		if (encolsing != nullptr)
			return encolsing->get(name);

		throw RuntimeError(
			name, "Undefined variable '" + std::string(name.lexeme) + "'."
		);
	}

private:
	// NOT using string_view as when in REPL mode each line string
	// gets destroyed once after it is executed.
	std::map<const std::string, Primitive> values;
	EnvironmentPtr encolsing;
};

#endif