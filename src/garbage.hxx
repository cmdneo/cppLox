#ifndef GARBAGE_HXX_INCLUDED
#define GARBAGE_HXX_INCLUDED

#include <cassert>
#include <initializer_list>
#include <map>
#include <memory>
#include <vector>
#include <variant>

#include "object.hxx"
#include "lox_function.hxx"
#include "lox_instance.hxx"
#include "environment.hxx"

class GarbageCollector
{
public:
	GarbageCollector(std::initializer_list<EnvironmentPtr> environs)
	{
		for (auto &env : environs) {
			environments.insert({env, false});
			directly_reachable.push_back(env);
		}
	}

	void push_environment(const EnvironmentPtr &environment)
	{
		environments.insert({environment, false});
		directly_reachable.push_back(environment);
	}

	void pop_environment() { directly_reachable.pop_back(); }

	void collect()
	{
		// Follow the chain from directly-reachable environments
		// and mark all which are reachable
		for (auto &env : directly_reachable)
			mark_reachable(env);

		// Clear out unreachable environments
		for (auto &[env, is_reachable] : environments) {
			if (env.expired()) {
				environments.erase(env);
				continue;
			}
			if (is_reachable == false) {
				env.lock()->values.clear();
				environments.erase(env);
			}
		}

		// Mark all as unreachable for the next round
		for (auto &[env, is_reachable] : environments)
			is_reachable = false;
	}

private:
	void mark_reachable(const std::weak_ptr<Environment> &environment)
	{
		// The environments containing 'this' and 'super' are not tracked here
		auto result = environments.find(environment);
		if (result != environments.end()) {

			// Indirectly reachable environments are also always valid,
			// so no need to check before locking
			auto env = result->first.lock();
			result->second = true;

			if (env->enclosing != nullptr)
				mark_reachable(env->enclosing);

			for (auto &[name, object] : env->values)
				mark_reachable_from_object(object);
		}
	}

	void mark_reachable_from_object(Object &object)
	{
		// Function objects have environments
		if (match_types<LoxCallablePtr>(object)) {
			auto &callable = *std::get<LoxCallablePtr>(object);
			if (typeid(callable) != typeid(LoxFunction))
				return;

			auto env = dynamic_cast<LoxFunction &>(callable).closure;

			// If already visited, then return
			auto result = environments.find(env);
			if (result != environments.end() && result->second == true)
				return;

			mark_reachable(env);
		}
		// An instance fields can have function objects which have environments
		else if (match_types<LoxInstancePtr>(object)) {
			for (auto &[name, obj] : std::get<LoxInstancePtr>(object)->fields)
				mark_reachable_from_object(obj);
		}
	}

	// Maps environment to whether are they reachable or not
	std::map<
		std::weak_ptr<Environment>, bool,
		std::owner_less<std::weak_ptr<Environment>>>
		environments;
	// Environments that enclose the currently active environment,
	// they are direclty reachable because they can be found via
	// traversing the environment chain upwards and they are always valid.
	std::vector<std::weak_ptr<Environment>> directly_reachable;
};

#endif