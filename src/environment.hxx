#ifndef ENVIRONMENT_HXX_INCLUDED
#define ENVIRONMENT_HXX_INCLUDED

#include <format>
#include <map>
#include <memory>
#include <string>

#include "runtime_error.hxx"
#include "object.hxx"
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

	void define(const std::string &name, const Object &value)
	{
		values[std::string(name)] = value;
	}

	void assign(const Token &name, const Object &value)
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
			name, std::format("Undefined variable '{}'.", name.lexeme)
		);
	}

	Object &get(const Token &name)
	{
		auto result = values.find(std::string(name.lexeme));
		if (result != values.end())
			return result->second;

		if (encolsing != nullptr)
			return encolsing->get(name);

		throw RuntimeError(
			name, std::format("Undefined variable '{}'.", name.lexeme)
		);
	}

private:
	std::map<const std::string, Object> values;
	EnvironmentPtr encolsing;
};

#endif