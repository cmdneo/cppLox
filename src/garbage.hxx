#ifndef GARBAGE_HXX_INCLUDED
#define GARBAGE_HXX_INCLUDED

#include <cassert>
#include <map>
#include <memory>
#include <vector>
#include <utility>

#include "environment.hxx"
#include "object/object.hxx"

class GarbageCollector
{
public:
	GarbageCollector(EnvironmentPtr initial_env)
	{
		environments.push_back(initial_env);
		directly_reachable.push_back(initial_env);
	}

	template <typename T, typename... Args>
	LoxPtr<T> create(Args &&...args)
	{
		auto ptr = new T(std::forward(args...));
		return LoxPtr<T>(ptr);
	}

	void push_environment(const EnvironmentPtr &environment)
	{
		environments.push_back(environment);
		directly_reachable.push_back(environment);
	}

	void pop_environment() { directly_reachable.pop_back(); }

	void collect()
	{
		// The timer was removed, because it caused a lot of page faults,
		// IDK why :|
		collect_impl();
	}

	//~GarbageCollector() { collect_impl(); }

private:
	void collect_impl();
	void mark_reachable(const std::weak_ptr<Environment> &environment);
	void mark_reachable_from_object(Object &object);

	// Maps environment to whether are they reachable or not
	std::vector<std::weak_ptr<Environment>> environments;
	// Environments that enclose the currently active environment,
	// they are direclty reachable because they can be found via
	// traversing the environment chain or in the interpreter.
	std::vector<std::weak_ptr<Environment>> directly_reachable;
};

#endif
